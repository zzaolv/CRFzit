// 文件路径: /CRFzit/daemon/rust/src/lib.rs

#[cxx::bridge(namespace = "crfzit::rust_bridge")]
pub mod ffi {
    
    // --- Rust 类型暴露给 C++ ---
    extern "Rust" {
        type LogicController;

        // 生命周期管理
        fn rust_init_controller(db_path: &str) -> UniquePtr<LogicController>;

        // 可以在这里添加其他需要从 C++ 调用的 Rust 函数
        // fn rust_some_function(controller: &LogicController, ...);
    }

    // --- C++ 函数暴露给 Rust ---
    unsafe extern "C++" {
        // 暂时为空，后续可以添加
        // fn cpp_some_function(param: i32) -> String;
    }
}

// 引入模块
mod config_manager;
mod logic_controller;

// 导入 LogicController 以便在 FFI 实现中使用
use crate::logic_controller::LogicController;
use cxx::UniquePtr;

// --- FFI 实现 ---

/// 初始化 Rust 核心，返回一个指向 LogicController 的智能指针。
/// 这个函数将从 C++ 的 main 函数中调用。
fn rust_init_controller(db_path: &str) -> UniquePtr<LogicController> {
    // 初始化日志记录器，如果已经初始化则忽略错误
    env_logger::builder()
        .filter_level(log::LevelFilter::Info)
        .try_init()
        .ok();

    log::info!("[Rust] Initializing controller with DB at: {}", db_path);

    // 创建 LogicController 实例
    let controller = LogicController::new(db_path);
    
    // 将 Rust 的 Box 转换为 C++ 的 UniquePtr
    UniquePtr::new(controller)
}