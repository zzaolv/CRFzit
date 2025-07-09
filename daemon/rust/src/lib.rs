// 文件路径: /CRFzit/daemon/rust/src/lib.rs (最终修正版)
use once_cell::sync::OnceCell;
use std::sync::Mutex;

// --- 模块声明 ---
// 【关键修正】: 在这里声明 logic_controller 和 config_manager 文件是 lib.rs 的子模块。
mod logic_controller;
mod config_manager;

// 现在可以成功地 use 了
use crate::logic_controller::LogicController;

// --- 全局静态实例 ---
static CONTROLLER: OnceCell<Mutex<LogicController>> = OnceCell::new();

#[cxx::bridge(namespace = "crfzit::rust_bridge")]
pub mod ffi {
    extern "Rust" {
        fn rust_init_controller(db_path: &str);
        fn rust_log_test_message();
    }
    unsafe extern "C++" {}
}

// --- FFI 实现 ---
pub fn rust_init_controller(db_path: &str) {
    env_logger::builder()
        .filter_level(log::LevelFilter::Info)
        .try_init()
        .ok();

    log::info!("[Rust] rust_init_controller called with DB at: {}", db_path);
    let controller = LogicController::new(db_path);
    if CONTROLLER.set(Mutex::new(controller)).is_err() {
        log::warn!("[Rust] Controller has already been initialized.");
    } else {
        log::info!("[Rust] Global controller initialized successfully.");
    }
}

pub fn rust_log_test_message() {
    if let Some(controller_mutex) = CONTROLLER.get() {
        let controller = controller_mutex.lock().unwrap();
        controller.log_message("Hello from a C++ call!");
    } else {
        log::error!("[Rust] Controller not initialized when rust_log_test_message was called.");
    }
}