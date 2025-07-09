// 文件路径: /CRFzit/daemon/rust/src/lib.rs (修正版)
use cxx::UniquePtr;
use once_cell::sync::OnceCell; // 需要添加这个依赖
use std::sync::Mutex;
use crate::logic_controller::LogicController;

// --- 全局静态实例 ---
// 使用 OnceCell 来安全地、懒加载地初始化我们的全局控制器。
// Mutex 保证了多线程访问时的内部可变性。
static CONTROLLER: OnceCell<Mutex<LogicController>> = OnceCell::new();

#[cxx::bridge(namespace = "crfzit::rust_bridge")]
pub mod ffi {
    
    // --- Rust 类型暴露给 C++ ---
    extern "Rust" {
        // 不再暴露 LogicController 类型给 C++
        // type LogicController;

        // 初始化函数现在不返回任何东西
        fn rust_init_controller(db_path: &str);

        // 我们在这里添加一个测试函数，以验证 C++ 可以调用并与控制器交互
        fn rust_log_test_message();
    }

    // --- C++ 函数暴露给 Rust ---
    unsafe extern "C++" {
        // 暂时为空
    }
}


// --- FFI 实现 ---

/// 初始化 Rust 核心。此函数将创建并初始化全局的 LogicController 实例。
/// 这个函数将从 C++ 的 main 函数中调用。
pub fn rust_init_controller(db_path: &str) {
    // 初始化日志记录器
    env_logger::builder()
        .filter_level(log::LevelFilter::Info)
        .try_init()
        .ok();

    log::info!("[Rust] rust_init_controller called with DB at: {}", db_path);

    // 创建 LogicController 实例
    let controller = LogicController::new(db_path);
    
    // 将实例放入 OnceCell。如果已经有值，这个操作会失败，但没关系。
    if CONTROLLER.set(Mutex::new(controller)).is_err() {
        log::warn!("[Rust] Controller has already been initialized.");
    } else {
        log::info!("[Rust] Global controller initialized successfully.");
    }
}

/// 一个测试函数，演示如何从 FFI 函数访问全局控制器。
pub fn rust_log_test_message() {
    if let Some(controller_mutex) = CONTROLLER.get() {
        // 获取锁
        let controller = controller_mutex.lock().unwrap();
        // 调用控制器的方法
        controller.log_message("Hello from a C++ call!");
    } else {
        log::error!("[Rust] Controller not initialized when rust_log_test_message was called.");
    }
}