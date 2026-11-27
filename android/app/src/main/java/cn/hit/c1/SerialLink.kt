package cn.hit.c1

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.os.Handler
import android.os.Looper
import java.io.IOException
import java.util.UUID
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit

/** One writer serializes RFCOMM frames. Input changes replace pending motion. */
@SuppressLint("MissingPermission")
internal class SerialLink(private val changed: (String, Boolean) -> Unit) {
    private val main = Handler(Looper.getMainLooper())
    private val connector = Executors.newSingleThreadExecutor()
    private val writer = Executors.newSingleThreadScheduledExecutor()
    @Volatile private var socket: BluetoothSocket? = null
    @Volatile private var ready = false
    private var command = "S\n"
    private var stopPending = true
    private var wakePending = false

    init {
        writer.scheduleWithFixedDelay({ transmit() }, 0, 50, TimeUnit.MILLISECONDS)
    }

    fun connect(device: BluetoothDevice) {
        disconnect()
        val target = device.createRfcommSocketToServiceRecord(
            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"))
        val label = device.name ?: device.address
        socket = target
        changed("正在连接 $label", false)
        connector.execute {
            try {
                target.connect()
                main.post {
                    if (socket === target) {
                        ready = true
                        changed("已连接 $label", true)
                    }
                }
            } catch (e: Exception) {
                failed(target, "连接失败，请检查模块配对和供电")
            }
        }
    }

    @Synchronized fun update(frame: String) {
        if (!ready) return
        command = frame
        if (frame == "S\n") stopPending = true
        if (!wakePending) {
            wakePending = true
            writer.execute {
                synchronized(this) { wakePending = false }
                transmit()
            }
        }
    }

    private fun transmit() {
        val target: BluetoothSocket
        val frame: String
        synchronized(this) {
            if (!ready) return
            target = socket ?: return
            frame = if (stopPending) "S\n" else command
            stopPending = false
        }
        try {
            target.outputStream.write(frame.toByteArray(Charsets.US_ASCII))
        } catch (e: Exception) {
            failed(target, "连接已断开")
        }
    }

    private fun failed(target: BluetoothSocket, message: String) {
        main.post {
            if (socket === target) {
                disconnect()
                changed(message, false)
            }
        }
    }

    @Synchronized fun disconnect() {
        ready = false
        val old = socket
        socket = null
        command = "S\n"
        stopPending = true
        try { old?.close() } catch (_: IOException) {}
    }

    fun close() {
        disconnect()
        connector.shutdownNow()
        writer.shutdownNow()
    }
}
