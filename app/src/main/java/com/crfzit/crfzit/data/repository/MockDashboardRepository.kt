package com.crfzit.crfzit.data.repository

import com.crfzit.ipc.ActiveAppsStateList
import com.crfzit.ipc.AppDisplayStatus
import com.crfzit.ipc.AppRuntimeState
import com.crfzit.ipc.FreezeMode
import com.crfzit.ipc.GlobalStats
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlin.random.Random

/**
 * DashboardRepository的模拟实现。
 * 用于在没有真实后端的情况下，为UI开发提供数据。
 */
class MockDashboardRepository : DashboardRepository {

    private var appStates = createInitialMockApps()

    /**
     * 模拟全局状态流，每秒更新一次数据。
     */
    override fun getGlobalStatsStream(): Flow<GlobalStats> = flow {
        while (true) {
            val stats = GlobalStats.newBuilder()
                .setTotalCpuUsagePercent(Random.nextFloat() * 50 + 10) // CPU在10-60%之间波动
                .setTotalMemKb(8 * 1024 * 1024) // 8GB 总内存
                .setAvailMemKb((4 * 1024 * 1024 + Random.nextLong(1024 * 1024)).toLong()) // 4-5GB可用
                .setNetworkSpeedDownBps(Random.nextLong(2 * 1024 * 1024 * 8)) // 0-2MB/s下载
                .setNetworkSpeedUpBps(Random.nextLong(512 * 1024 * 8)) // 0-512KB/s上传
                .setActiveProfileName("🎮 游戏模式")
                .build()
            emit(stats)
            delay(1000) // 每秒发送一次
        }
    }

    /**
     * 模拟活动应用列表流，每2秒更新一次应用状态。
     */
    override fun getActiveAppsStateStream(): Flow<ActiveAppsStateList> = flow {
        while (true) {
            // 随机更新一两个应用的状态，模拟动态变化
            updateRandomApps()
            
            val list = ActiveAppsStateList.newBuilder()
                .addAllApps(appStates)
                .build()
            emit(list)
            delay(2000) // 每2秒发送一次
        }
    }

    private fun updateRandomApps() {
        if (appStates.isEmpty()) return
        val indexToUpdate = Random.nextInt(appStates.size)
        val app = appStates[indexToUpdate]
        val builder = app.toBuilder()

        // 简单的状态机转换模拟
        when (app.displayStatus) {
            AppDisplayStatus.PENDING_FREEZE -> {
                if (app.pendingFreezeSec > 0) {
                    builder.pendingFreezeSec = app.pendingFreezeSec - 2
                } else {
                    builder.displayStatus = AppDisplayStatus.FROZEN
                    builder.cpuUsagePercent = 0.0f
                }
            }
            AppDisplayStatus.FROZEN -> {
                 // 模拟随机解冻
                if(Random.nextFloat() > 0.9) {
                    builder.displayStatus = AppDisplayStatus.BACKGROUND_ACTIVE
                }
            }
            else -> {
                // 模拟CPU和内存的小幅波动
                val currentCpu = app.cpuUsagePercent
                builder.cpuUsagePercent = (currentCpu + (Random.nextFloat() - 0.5f) * 0.5f).coerceIn(0f, 15f)
                val currentMem = app.memoryUsageKb
                builder.memoryUsageKb = (currentMem + (Random.nextFloat() - 0.5f) * 1000).toLong().coerceAtLeast(10000)
            }
        }
        
        appStates = appStates.toMutableList().also { it[indexToUpdate] = builder.build() }
    }

    private fun createInitialMockApps(): List<AppRuntimeState> {
        return listOf(
            AppRuntimeState.newBuilder()
                .setPackageName("com.tencent.mm") // 微信
                .setDisplayStatus(AppDisplayStatus.FOREGROUND)
                .setIsForeground(true)
                .setIsWhitelisted(true)
                .setCpuUsagePercent(0.5f)
                .setMemoryUsageKb(256 * 1024)
                .build(),
            AppRuntimeState.newBuilder()
                .setPackageName("com.tencent.qqmusic") // QQ音乐
                .setDisplayStatus(AppDisplayStatus.BACKGROUND_ACTIVE)
                .setHasPlayback(true)
                .setCpuUsagePercent(2.1f)
                .setMemoryUsageKb(128 * 1024)
                .build(),
            AppRuntimeState.newBuilder()
                .setPackageName("com.baidu.netdisk") // 百度网盘
                .setDisplayStatus(AppDisplayStatus.BACKGROUND_ACTIVE)
                .setHasNetworkActivity(true)
                .setCpuUsagePercent(8.5f)
                .setMemoryUsageKb(310 * 1024)
                .build(),
            AppRuntimeState.newBuilder()
                .setPackageName("com.coolapk.market") // 酷安
                .setDisplayStatus(AppDisplayStatus.BACKGROUND_ACTIVE)
                .setHasNotification(true)
                .setCpuUsagePercent(0.0f)
                .setMemoryUsageKb(64 * 1024)
                .build(),
            AppRuntimeState.newBuilder()
                .setPackageName("com.taobao.taobao") // 淘宝
                .setDisplayStatus(AppDisplayStatus.FROZEN)
                .setActiveFreezeMode(FreezeMode.V2_FREEZE)
                .setCpuUsagePercent(0.0f)
                .setMemoryUsageKb(180 * 1024)
                .build(),
            AppRuntimeState.newBuilder()
                .setPackageName("com.xunmeng.pinduoduo") // 拼多多
                .setDisplayStatus(AppDisplayStatus.KILLED)
                .setActiveFreezeMode(FreezeMode.KILL)
                .setCpuUsagePercent(0.0f)
                .setMemoryUsageKb(0)
                .build(),
            AppRuntimeState.newBuilder()
                .setPackageName("com.zhihu.android") // 知乎
                .setDisplayStatus(AppDisplayStatus.PENDING_FREEZE)
                .setPendingFreezeSec(135) // 2m 15s
                .setCpuUsagePercent(0.1f)
                .setMemoryUsageKb(150 * 1024)
                .build()
        )
    }
}