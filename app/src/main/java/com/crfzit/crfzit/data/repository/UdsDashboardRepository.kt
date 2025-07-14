package com.crfzit.crfzit.data.repository

import android.util.Log
import com.crfzit.crfzit.data.model.AppRuntimeState
import com.crfzit.crfzit.data.model.DashboardUpdate
import com.crfzit.crfzit.data.model.GlobalStats
import com.crfzit.crfzit.data.uds.UdsClient
import com.google.gson.Gson
import com.google.gson.JsonSyntaxException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.*

class UdsDashboardRepository(
    scope: CoroutineScope
) : DashboardRepository {

    private val udsClient = UdsClient(scope)
    private val gson = Gson()

    // 一个共享的数据流，用于解析所有来自daemon的消息
    private val dashboardUpdateFlow: SharedFlow<DashboardUpdate> = flow {
        udsClient.start() // 启动UDS客户端连接
        udsClient.incomingMessages.collect { jsonLine ->
            try {
                val update = gson.fromJson(jsonLine, DashboardUpdate::class.java)
                if (update.command == "dashboard_update") {
                    emit(update)
                }
            } catch (e: JsonSyntaxException) {
                Log.e("UdsDashboardRepo", "JSON parse error: ${e.message} \n\t for line: $jsonLine")
            }
        }
    }.flowOn(Dispatchers.IO)
     .shareIn(scope, SharingStarted.WhileSubscribed(5000), replay = 1)


    override fun getGlobalStatsStream(): Flow<GlobalStats> {
        return dashboardUpdateFlow
            .map { it.payload.globalStats }
            .distinctUntilChanged() // 只有在数据变化时才发出
    }

    override fun getAppRuntimeStateStream(): Flow<List<AppRuntimeState>> {
        return dashboardUpdateFlow
            .map { it.payload.appsRuntimeState }
            .distinctUntilChanged()
    }
}