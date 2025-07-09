package com.crfzit.crfzit.ui.dashboard

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.crfzit.crfzit.data.repository.DashboardRepository
import com.crfzit.crfzit.data.repository.MockDashboardRepository
import com.crfzit.ipc.ActiveAppsStateList
import com.crfzit.ipc.AppRuntimeState
import com.crfzit.ipc.GlobalStats
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.launch

// UI状态数据类
data class DashboardUiState(
    val globalStats: GlobalStats = GlobalStats.getDefaultInstance(),
    val activeApps: List<AppRuntimeState> = emptyList(),
    val isLoading: Boolean = true
)

class DashboardViewModel(
    // 以后这里会通过Hilt等依赖注入框架来提供真实的Repository
    private val repository: DashboardRepository = MockDashboardRepository()
) : ViewModel() {

    private val _uiState = MutableStateFlow(DashboardUiState())
    val uiState: StateFlow<DashboardUiState> = _uiState.asStateFlow()

    init {
        observeDashboardData()
    }

    private fun observeDashboardData() {
        viewModelScope.launch {
            // 使用 combine 操作符将两个数据流合并
            // 当任何一个流发出新数据时，都会重新计算并更新UI状态
            repository.getGlobalStatsStream()
                .combine(repository.getActiveAppsStateStream()) { stats, appsList ->
                    // 返回一个新的UI状态对象
                    DashboardUiState(
                        globalStats = stats,
                        activeApps = appsList.appsList, // 从 ActiveAppsStateList 中取出列表
                        isLoading = false
                    )
                }
                .collect { newState ->
                    // 将合并后的新状态赋值给 StateFlow
                    _uiState.value = newState
                }
        }
    }
}