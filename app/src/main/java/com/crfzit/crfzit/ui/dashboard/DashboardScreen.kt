package com.crfzit.crfzit.ui.dashboard

import androidx.compose.material3.ExperimentalMaterial3Api // 确保导入
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.crfzit.ipc.AppDisplayStatus
import com.crfzit.ipc.AppRuntimeState
import com.crfzit.ipc.FreezeMode
import com.crfzit.ipc.GlobalStats
import java.util.Locale


@OptIn(ExperimentalMaterial3Api::class) // <--- 添加这一行
@Composable
fun DashboardScreen(
    viewModel: DashboardViewModel = viewModel()
) {
    val uiState by viewModel.uiState.collectAsState()

    Scaffold(
        topBar = {
            TopAppBar(title = { Text("CRFzit Dashboard") })
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .padding(paddingValues)
                .fillMaxSize()
        ) {
            if (uiState.isLoading) {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    CircularProgressIndicator()
                }
            } else {
                GlobalStatsHeader(stats = uiState.globalStats)
                Divider()
                ActiveAppsList(apps = uiState.activeApps)
            }
        }
    }
}

@Composable
fun GlobalStatsHeader(stats: GlobalStats) {
    val memUsedGb = (stats.totalMemKb - stats.availMemKb) / (1024.0 * 1024.0)
    val memTotalGb = stats.totalMemKb / (1024.0 * 1024.0)
    val netDownMbps = stats.networkSpeedDownBps / (1024.0 * 1024.0)
    val netUpKbps = stats.networkSpeedUpBps / 1024.0
    
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(text = "CPU: ${"%.1f".format(stats.totalCpuUsagePercent)}%", fontSize = 12.sp)
        Text(text = "MEM: ${"%.1f".format(memUsedGb)}/${"%.1f".format(memTotalGb)} GB", fontSize = 12.sp)
        Text(text = "NET: ↓${"%.1f".format(netDownMbps)}MB/s ↑${"%.0f".format(netUpKbps)}KB/s", fontSize = 12.sp)
        Text(text = "MODE: ${stats.activeProfileName}", fontSize = 12.sp, fontWeight = FontWeight.Bold)
    }
}

@Composable
fun ActiveAppsList(apps: List<AppRuntimeState>) {
    LazyColumn(
        modifier = Modifier.fillMaxSize(),
        contentPadding = PaddingValues(horizontal = 16.dp, vertical = 8.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        // 使用 packageName 作为 key，可以获得更好的性能和动画效果
        items(items = apps, key = { it.packageName }) { app ->
            AppStatusCard(app = app)
        }
    }
}

@Composable
fun AppStatusCard(app: AppRuntimeState) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                // 模拟应用图标
                Box(
                    modifier = Modifier
                        .size(40.dp)
                        .background(Color.LightGray, shape = MaterialTheme.shapes.medium),
                    contentAlignment = Alignment.Center
                ) {
                    Text(text = getAppName(app.packageName).take(2).uppercase(Locale.ROOT), fontWeight = FontWeight.Bold)
                }
                
                Spacer(modifier = Modifier.width(12.dp))
                
                // 应用名
                Text(
                    text = getAppName(app.packageName),
                    style = MaterialTheme.typography.bodyLarge,
                    modifier = Modifier.weight(1f)
                )

                // 状态指示器图标
                AppStatusIndicators(app = app)
            }
            Spacer(modifier = Modifier.height(8.dp))
            
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                // 资源占用
                Text(
                    text = "MEM: ${app.memoryUsageKb / 1024} MB   CPU: ${"%.1f".format(app.cpuUsagePercent)}%",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
                // 详细状态文本
                Text(
                    text = getStatusText(app),
                    style = MaterialTheme.typography.bodySmall,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

@Composable
fun AppStatusIndicators(app: AppRuntimeState) {
    Row {
        if (app.isForeground) Text("▶️", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.isWhitelisted) Text("🛡️", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.hasPlayback) Text("🎵", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.hasNotification) Text("🔔", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.hasNetworkActivity) Text("📡", modifier = Modifier.padding(horizontal = 2.dp))
        
        when(app.displayStatus) {
            AppDisplayStatus.FROZEN -> {
                val freezeMethod = when(app.activeFreezeMode) {
                    FreezeMode.V2_FREEZE -> "(V2)"
                    FreezeMode.V1_FREEZE -> "(V1)"
                    FreezeMode.SIGSTOP -> "(ST)"
                    else -> ""
                }
                Text("❄️ $freezeMethod", modifier = Modifier.padding(horizontal = 2.dp))
            }
            AppDisplayStatus.KILLED -> {
                 val killMethod = when(app.activeFreezeMode) {
                    FreezeMode.KILL -> "(KILL)"
                    else -> ""
                }
                Text("🧊 $killMethod", modifier = Modifier.padding(horizontal = 2.dp))
            }
            AppDisplayStatus.PENDING_FREEZE -> Text("⏳", modifier = Modifier.padding(horizontal = 2.dp))
            else -> {} // 其他状态不显示特殊图标
        }
    }
}

fun getStatusText(app: AppRuntimeState): String {
    return when(app.displayStatus) {
        AppDisplayStatus.FOREGROUND -> "前台 / " + if (app.isWhitelisted) "白名单" else "活动中"
        AppDisplayStatus.BACKGROUND_ACTIVE -> {
            when {
                app.hasPlayback -> "后台播放 / 活动中"
                app.hasNetworkActivity -> "网速豁免 / 活动中"
                app.hasNotification -> "通知豁免 / 活动中"
                else -> "后台 / 活动中"
            }
        }
        AppDisplayStatus.FROZEN -> "已冻结 (${app.activeFreezeMode.name.replace("_FREEZE","")})"
        AppDisplayStatus.KILLED -> "已杀死"
        AppDisplayStatus.PENDING_FREEZE -> "等待冻结 (剩 ${app.pendingFreezeSec}s)"
        else -> "未知状态"
    }
}

// 模拟从包名获取应用名
fun getAppName(packageName: String): String {
    return packageName.substringAfterLast('.').replaceFirstChar { it.titlecase(Locale.getDefault()) }
}