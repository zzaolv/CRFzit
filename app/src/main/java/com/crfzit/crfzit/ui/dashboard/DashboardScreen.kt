package com.crfzit.crfzit.ui.dashboard

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
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.crfzit.crfzit.data.model.AppRuntimeState
import com.crfzit.crfzit.data.model.DisplayStatus
import com.crfzit.crfzit.data.model.FreezeMode
import com.crfzit.crfzit.data.model.GlobalStats
import java.util.Locale

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen(
    viewModel: DashboardViewModel = viewModel()
) {
    val uiState by viewModel.uiState.collectAsState()

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("CRFzit Dashboard") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer
                )
            )
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .padding(paddingValues)
                .fillMaxSize()
        ) {
            if (uiState.isLoading) {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        CircularProgressIndicator()
                        Spacer(modifier = Modifier.height(8.dp))
                        Text("等待连接守护进程...")
                    }
                }
            } else if (!uiState.isConnected) {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                     Text("连接守护进程失败！\n请检查模块是否正常运行。", textAlign = TextAlign.Center)
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
    val netDownMbps = stats.netDownSpeedBps / (1024.0 * 1024.0 * 8) // 注意是 bps
    val netUpKbps = stats.netUpSpeedBps / (1024.0 * 8)

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
                Box(
                    modifier = Modifier
                        .size(40.dp)
                        .background(Color.LightGray, shape = MaterialTheme.shapes.medium),
                    contentAlignment = Alignment.Center
                ) {
                    Text(text = app.appName.take(2).uppercase(Locale.ROOT), fontWeight = FontWeight.Bold)
                }
                Spacer(modifier = Modifier.width(12.dp))
                Text(
                    text = app.appName,
                    style = MaterialTheme.typography.bodyLarge,
                    modifier = Modifier.weight(1f)
                )
                AppStatusIndicators(app = app)
            }
            Spacer(modifier = Modifier.height(8.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = "MEM: ${app.memUsageKb / 1024} MB   CPU: ${"%.1f".format(app.cpuUsagePercent)}%",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
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
        if (app.displayStatus == DisplayStatus.FOREGROUND_GAME) Text("🎮", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.isWhitelisted) Text("🛡️", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.hasPlayback) Text("🎵", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.hasNotification) Text("🔔", modifier = Modifier.padding(horizontal = 2.dp))
        if (app.hasNetworkActivity) Text("📡", modifier = Modifier.padding(horizontal = 2.dp))

        when(app.displayStatus) {
            DisplayStatus.FROZEN -> {
                val freezeMethod = when(app.activeFreezeMode) {
                    FreezeMode.CGROUP -> "(CG)"
                    FreezeMode.SIGSTOP -> "(ST)"
                    else -> ""
                }
                Text("❄️ $freezeMethod", modifier = Modifier.padding(horizontal = 2.dp))
            }
            DisplayStatus.KILLED -> {
                Text("🧊", modifier = Modifier.padding(horizontal = 2.dp))
            }
            DisplayStatus.PENDING_FREEZE -> Text("⏳", modifier = Modifier.padding(horizontal = 2.dp))
            else -> {}
        }
    }
}

fun getStatusText(app: AppRuntimeState): String {
    return when(app.displayStatus) {
        DisplayStatus.FOREGROUND_GAME -> "前台 / 游戏中"
        DisplayStatus.FOREGROUND -> "前台 / 活动中"
        DisplayStatus.BACKGROUND_ACTIVE -> "后台 / 活动中"
        DisplayStatus.FROZEN -> "已冻结 (${app.activeFreezeMode?.name ?: "N/A"})"
        DisplayStatus.KILLED -> "已杀死"
        DisplayStatus.PENDING_FREEZE -> "等待冻结..."
        DisplayStatus.UNFROZEN_BY_PROFILE -> "已解冻 (场景模式)"
        else -> "未知状态"
    }
}