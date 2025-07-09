package com.crfzit.crfzit.data.repository

import com.crfzit.ipc.ActiveAppsStateList
import com.crfzit.ipc.GlobalStats
import kotlinx.coroutines.flow.Flow

/**
 * 仪表盘数据仓库接口
 * 定义了获取仪表盘所需数据的方法
 */
interface DashboardRepository {
    /**
     * 获取全局系统状态的持续数据流
     */
    fun getGlobalStatsStream(): Flow<GlobalStats>

    /**
     * 获取活动应用状态列表的持续数据流
     */
    fun getActiveAppsStateStream(): Flow<ActiveAppsStateList>
}