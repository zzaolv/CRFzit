// 文件路径: /CRFzit/daemon/rust/src/config_manager.rs
use rusqlite::{Connection, Result};
use std::sync::{Arc, Mutex};

pub struct ConfigManager {
    conn: Arc<Mutex<Connection>>,
}

impl ConfigManager {
    /// 创建一个新的 ConfigManager 并初始化数据库
    pub fn new(db_path: &str) -> Result<Self> {
        let conn = Connection::open(db_path)?;
        let manager = ConfigManager {
            conn: Arc::new(Mutex::new(conn)),
        };
        manager.init_db()?;
        Ok(manager)
    }

    /// 创建数据库表（如果不存在）
    fn init_db(&self) -> Result<()> {
        let conn = self.conn.lock().unwrap();
        // 对应 AppConfig protobuf 消息
        conn.execute(
            "CREATE TABLE IF NOT EXISTS app_configs (
                package_name TEXT PRIMARY KEY,
                freeze_mode INTEGER NOT NULL,
                network_policy INTEGER NOT NULL,
                oom_priority INTEGER NOT NULL,
                foreground_policy INTEGER NOT NULL,
                lenient_allow_playback BOOLEAN NOT NULL,
                lenient_allow_notification BOOLEAN NOT NULL,
                lenient_allow_network BOOLEAN NOT NULL,
                is_whitelisted BOOLEAN NOT NULL,
                allow_fcm_wakeup BOOLEAN NOT NULL
            )",
            [],
        )?;
        log::info!("[Rust DB] 'app_configs' table initialized.");
        // 其他表...
        Ok(())
    }
}