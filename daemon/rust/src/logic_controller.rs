// 文件路径: /CRFzit/daemon/rust/src/logic_controller.rs
use crate::config_manager::ConfigManager;
use std::sync::Arc;

/// 这是我们所有状态和逻辑的核心结构体
pub struct LogicController {
    config_manager: Arc<ConfigManager>,
    // 后续会添加 runtime_states, event_logger 等
}

impl LogicController {
    /// 创建一个新的 LogicController 实例
    pub fn new(db_path: &str) -> Self {
        let config_manager =
            Arc::new(ConfigManager::new(db_path).expect("Failed to initialize ConfigManager"));
        
        log::info!("[Rust] ConfigManager initialized.");

        Self {
            config_manager,
        }
    }
}