// 文件路径: /CRFzit/daemon/rust/src/logic_controller.rs (修正版)
use crate::config_manager::ConfigManager;
use std::sync::Arc;

pub struct LogicController {
    config_manager: Arc<ConfigManager>,
}

impl LogicController {
    pub fn new(db_path: &str) -> Self {
        let config_manager =
            Arc::new(ConfigManager::new(db_path).expect("Failed to initialize ConfigManager"));
        
        log::info!("[Rust LC] ConfigManager initialized.");

        Self {
            config_manager,
        }
    }

    // 新增一个简单的测试方法
    pub fn log_message(&self, msg: &str) {
        log::info!("[Rust LC] Received message: {}", msg);
    }
}