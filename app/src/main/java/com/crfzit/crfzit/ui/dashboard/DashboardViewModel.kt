package com.crfzit.crfzit.ui.dashboard

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.crfzit.crfzit.data.model.AppRuntimeState
import com.crfzit.crfzit.data.model.GlobalStats
import com.crfzit.crfzit.data.repository.DashboardRepository
import com.crfzit.crfzit.data.repository.UdsDashboardRepository
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch

data class DashboardUiState(
    val globalStats: GlobalStats = GlobalStats(),
    val activeApps: List<AppRuntimeState> = emptyList(),
    val isLoading: Boolean = true,
    val isConnected: Boolean = false
)

class DashboardViewModel(
    // 构造函数现在接受一个可空的 Repository，用于测试注入
    private val injectedRepository: DashboardRepository? = null
) : ViewModel() {

    // 使用 by lazy 实现懒加载，完美解决初始化顺序问题
    private val repository: DashboardRepository by lazy {
        // 如果外部传入了 repository (例如在测试中)，就用它
        // 否则，创建我们默认的真实 repository 实例，此时 viewModelScope 已可用
        injectedRepository ?: UdsDashboardRepository(viewModelScope)
    }

    private val _uiState = MutableStateFlow(DashboardUiState())
    val uiState: StateFlow<DashboardUiState> = _uiState.asStateFlow()

    init {
        observeDashboardData()
    }

    private fun observeDashboardData() {
        // 在这里调用 launch 时，repository 属性会通过 by lazy 被首次初始化
        viewModelScope.launch {
            repository.getGlobalStatsStream()
                .combine(repository.getAppRuntimeStateStream()) { stats, apps ->
                    DashboardUiState(
                        globalStats = stats,
                        activeApps = apps,
                        isLoading = false,
                        isConnected = true
                    )
                }
                .onStart {
                    _uiState.value = DashboardUiState(isLoading = true)
                }
                .catch { e ->
                    _uiState.value = _uiState.value.copy(isLoading = false, isConnected = false)
                }
                .collect { newState ->
                    _uiState.value = newState
                }
        }
    }
}