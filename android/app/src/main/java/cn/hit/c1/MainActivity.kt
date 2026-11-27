package cn.hit.c1

import android.Manifest
import android.annotation.SuppressLint
import android.app.AlertDialog
import android.bluetooth.BluetoothManager
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.*

class MainActivity : ComponentActivity() {
    private var status by mutableStateOf("未连接")
    private var connected by mutableStateOf(false)
    private var epoch by mutableIntStateOf(0)
    private lateinit var link: SerialLink
    private val permission = registerForActivityResult(ActivityResultContracts.RequestPermission()) {
        if (it) chooseDevice() else status = "请允许连接附近设备"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        link = SerialLink { message, live ->
            status = message
            connected = live
            epoch++
        }
        setContent {
            key(epoch) {
                Controls(status, connected, ::chooseDevice, { vx, vy, yaw, arm ->
                    link.update("M,$vx,$vy,$yaw,$arm\n")
                }, { link.update("S\n") })
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun chooseDevice() {
        if (Build.VERSION.SDK_INT >= 31 &&
            checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            permission.launch(Manifest.permission.BLUETOOTH_CONNECT)
            return
        }
        link.update("S\n")
        epoch++
        val adapter = getSystemService(BluetoothManager::class.java)?.adapter
        if (adapter?.isEnabled != true) {
            status = "请在系统设置开启蓝牙"
            return
        }
        val paired = adapter.bondedDevices.sortedBy { it.name ?: it.address }
        if (paired.isEmpty()) {
            status = "请先在系统蓝牙设置中配对车载模块"
            return
        }
        AlertDialog.Builder(this).setTitle("选择已配对的车载模块")
            .setItems(paired.map { "${it.name ?: "蓝牙设备"}  ${it.address}" }.toTypedArray()) { _, i ->
                try { link.connect(paired[i]) }
                catch (_: Exception) { connected = false; status = "无法打开蓝牙串口"; epoch++ }
            }.setNegativeButton("取消", null).show()
    }

    override fun onPause() {
        link.update("S\n")
        epoch++
        super.onPause()
    }

    override fun onStop() {
        link.disconnect()
        connected = false
        status = "未连接"
        super.onStop()
    }

    override fun onDestroy() {
        link.close()
        super.onDestroy()
    }
}
