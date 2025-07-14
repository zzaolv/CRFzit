package com.crfzit.crfzit.ui.dashboard

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.crfzit.crfzit.data.model.AppRuntimeState
import com.crfzit.crfzit.data.model.GlobalStats
import com.crfzit.crfzit.data.repository.DashboardRepository
import com.crfzit.crfzit.data.repository.UdsDashboardRepository
import kotlinx.coroutines.flow.*

data class DashboardUiState(
    val globalStats: GlobalStats = GlobalStats(),
    val activeApps: List<AppRuntimeState> = emptyList(),
    val isLoading: Boolean = true,
    val isConnected: Boolean = false // 新增连接状态
)

class DashboardViewModel(
    // 提供一个默认的真实 Repository 实例
    private val repository: DashboardRepository = UdsDashboardRepository(viewModelScope)
) : ViewModel() {

    private val _uiState = MutableStateFlow(DashboardUiState())
    val uiState: StateFlow<DashboardUiState> = _uiState.asStateFlow()

    init {
        observeDashboardData()
    }

    private fun observeDashboardData() {
        viewModelScope.launch {
            // 合并两个流
            repository.getGlobalStatsStream()
                .combine(repository.getAppRuntimeStateStream()) { stats, apps ->
                    DashboardUiState(
                        globalStats = stats,
                        activeApps = apps,
                        isLoading = false,
                        isConnected = true // 收到数据意味着已连接
                    )
                }
                .onStart {
                    // 在开始收集前，可以设置一个超时来判断是否连接失败
                    // 这里简化处理，初始为loading
                    _uiState.value = DashboardUiState(isLoading = true)
                }
                .catch { e ->
                    // 流出现错误
                    _uiState.value = _uiState.value.copy(isLoading = false, isConnected = false)
                }
                .collect { newState ->
                    _uiState.value = newState
                }
        }
    }
}