package cn.hit.c1

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.awaitEachGesture
import androidx.compose.foundation.gestures.awaitFirstDown
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.PointerEventPass
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlin.math.hypot
import kotlin.math.roundToInt

private val Ink = Color(0xFF11273A)
private val Blue = Color(0xFF176A87)
private val Pale = Color(0xFFEAF4F5)

@Composable
fun Controls(status: String, connected: Boolean, connect: () -> Unit, send: (Int, Int, Int, Char) -> Unit, stop: () -> Unit) {
    var armMode by remember { mutableStateOf(false) }
    var vx by remember { mutableIntStateOf(0) }
    var vy by remember { mutableIntStateOf(0) }
    var yaw by remember { mutableIntStateOf(0) }
    var key by remember { mutableStateOf('-') }
    var activePointers by remember { mutableIntStateOf(0) }
    var blocked by remember { mutableStateOf(false) }
    var generation by remember { mutableIntStateOf(0) }
    val currentSend by rememberUpdatedState(send)
    val currentStop by rememberUpdatedState(stop)
    fun reset() {
        generation++
        blocked = activePointers > 0
        vx = 0; vy = 0; yaw = 0; key = '-'
        currentStop()
    }
    fun transmit() {
        if (connected && !blocked) currentSend(vx, vy, yaw, key)
    }
    LaunchedEffect(connected) {
        if (!connected) {
            generation++
            blocked = activePointers > 0
            vx = 0; vy = 0; yaw = 0; key = '-'
        }
    }

    Column(Modifier.fillMaxSize().background(Color(0xFFF3F7F7)).pointerInput(Unit) {
        awaitPointerEventScope {
            while (true) {
                val event = awaitPointerEvent(PointerEventPass.Initial)
                activePointers = event.changes.count { it.pressed }
                if (activePointers == 0) blocked = false
            }
        }
    }.systemBarsPadding().padding(horizontal = 24.dp, vertical = 14.dp)) {
        Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.SpaceBetween) {
            Column {
                Text("移动与机械臂控制台", color = Ink, fontSize = 23.sp, fontWeight = FontWeight.Bold)
            }
            Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                Text(status, color = if (connected) Blue else Color(0xFF7C8991), fontSize = 14.sp)
                OutlinedButton(onClick = connect) { Text(if (connected) "连接管理" else "连接设备") }
                Button(onClick = { reset() }, colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFB83F42))) { Text("停止") }
            }
        }
        Spacer(Modifier.height(12.dp))
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            listOf(false to "底盘驾驶", true to "机械臂点动").forEach { (mode, label) ->
                FilledTonalButton(onClick = { if (armMode != mode) { reset(); armMode = mode } },
                    colors = ButtonDefaults.filledTonalButtonColors(containerColor = if (armMode == mode) Blue else Color.White, contentColor = if (armMode == mode) Color.White else Ink)) { Text(label) }
            }
            Spacer(Modifier.weight(1f))
            Text(if (armMode) "按住动作，松手保持" else "双手操作 · 松手归中", color = Color(0xFF67808B), modifier = Modifier.align(Alignment.CenterVertically))
        }
        Spacer(Modifier.height(10.dp))
        if (!armMode) {
            val stickGeneration = generation
            Row(Modifier.weight(1f).fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                StickCard("平移", "上下前后 · 左右横移", connected && !blocked, generation, Modifier.weight(1f).fillMaxHeight()) { x, y ->
                    if (generation == stickGeneration) { vx = -y; vy = -x; transmit() }
                }
                StickCard("旋转", "左右转向", connected && !blocked, generation, Modifier.weight(1f).fillMaxHeight(), horizontalOnly = true) { x, _ ->
                    if (generation == stickGeneration) { yaw = (-x * 0.45f).roundToInt(); transmit() }
                }
            }
        } else {
            Row(Modifier.weight(1f).fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                val columns = listOf(
                    listOf(Triple("升降", '5' to '6', "上升" to "下降"), Triple("夹爪", '7' to '8', "闭合" to "张开")),
                    listOf(Triple("底座", '1' to '2', "正转" to "反转"), Triple("腕部", '3' to '4', "正转" to "反转"))
                )
                columns.forEach { groups ->
                    Column(Modifier.weight(1f).fillMaxHeight(), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                        groups.forEach { (title, keys, labels) ->
                            Row(Modifier.weight(1f).fillMaxWidth().background(Color.White, RoundedCornerShape(16.dp)).padding(horizontal = 14.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                                Text(title, Modifier.weight(1f), color = Ink, fontWeight = FontWeight.SemiBold)
                                listOf(keys.first to labels.first, keys.second to labels.second).forEach { (action, label) ->
                                    val epoch = generation
                                    Box(Modifier.weight(1f).fillMaxHeight().padding(vertical = 7.dp).background(if (key == action) Blue else Pale, RoundedCornerShape(12.dp)).pointerInput(connected, generation) {
                                        detectTapGestures(onPress = {
                                            if (connected && !blocked && key == '-') {
                                                key = action; transmit()
                                                try { awaitRelease() } finally {
                                                    if (generation == epoch && key == action) { key = '-'; transmit() }
                                                }
                                            }
                                        })
                                    }, contentAlignment = Alignment.Center) { Text(label, color = if (key == action) Color.White else Blue, fontWeight = FontWeight.SemiBold) }
                                }
                            }
                        }
                    }
                }
            }
        }
        Spacer(Modifier.height(8.dp))
        Text("停止：底盘停车，机械臂保持当前位置", color = Color(0xFF7D919A), fontSize = 12.sp)
    }
}

@Composable
private fun StickCard(title: String, subtitle: String, enabled: Boolean, generation: Int, modifier: Modifier = Modifier, horizontalOnly: Boolean = false, onMove: (Int, Int) -> Unit) {
    var position by remember { mutableStateOf(Offset.Zero) }
    val currentGeneration by rememberUpdatedState(generation)
    LaunchedEffect(generation, enabled) { position = Offset.Zero }
    Column(modifier.background(Color.White, RoundedCornerShape(20.dp)).padding(14.dp), horizontalAlignment = Alignment.CenterHorizontally) {
        Text(title, color = Ink, fontSize = 18.sp, fontWeight = FontWeight.Bold)
        Text(subtitle, color = Color(0xFF78909A), fontSize = 12.sp)
        BoxWithConstraints(Modifier.weight(1f).fillMaxWidth(), contentAlignment = Alignment.Center) {
            val size = minOf(maxWidth, maxHeight) * 0.85f
            Canvas(Modifier.size(size).pointerInput(enabled, generation) {
                if (!enabled) return@pointerInput
                awaitEachGesture {
                    val down = awaitFirstDown(requireUnconsumed = false)
                    val radius = minOf(this@pointerInput.size.width, this@pointerInput.size.height) / 2f
                    fun update(point: Offset) {
                        val delta = point - Offset(radius, radius)
                        val length = hypot(delta.x, delta.y)
                        val limited = if (length > radius) delta * (radius / length) else delta
                        position = Offset(limited.x / radius, if (horizontalOnly) 0f else limited.y / radius)
                        fun axis(value: Float) = if (kotlin.math.abs(value) < 0.12f) 0 else (value * 100).roundToInt().coerceIn(-100, 100)
                        onMove(axis(position.x), axis(position.y))
                    }
                    try {
                        update(down.position)
                        do {
                            val event = awaitPointerEvent()
                            val change = event.changes.firstOrNull { it.id == down.id } ?: break
                            if (change.pressed) { update(change.position); change.consume() }
                        } while (change.pressed)
                    } finally {
                        position = Offset.Zero
                        onMove(0, 0)
                    }
                }
            }) {
                val r = this.size.minDimension / 2f
                drawCircle(Pale, r)
                drawCircle(Color(0xFFD0E5E7), r * 0.72f, style = androidx.compose.ui.graphics.drawscope.Stroke(width = 2.dp.toPx()))
                drawCircle(Blue.copy(alpha = if (enabled) 0.9f else 0.35f), r * 0.25f, center = center + position * (r * 0.68f))
            }
        }
    }
}


