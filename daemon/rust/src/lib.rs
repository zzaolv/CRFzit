// File: ~/projects/CRFzit/daemon/rust/src/lib.rs

#[cxx::bridge]
mod ffi {
    // Rust -> C++
    extern "C++" {
        // 引用 C++ 定义的日志函数
        include!("daemon/cpp/logger.h");
        fn log_info(message: &str);
    }

    // C++ -> Rust
    extern "Rust" {
        // 暴露给 C++ 的核心逻辑函数
        fn get_core_version() -> String;
        fn process_event_from_cpp(event_type: i32, package_name: &str);
    }
}

// --- Rust 实现 ---

pub fn get_core_version() -> String {
    "CRFzit Core v1.0.0-alpha".to_string()
}

pub fn process_event_from_cpp(event_type: i32, package_name: &str) {
    let event_str = match event_type {
        1 => "BROADCAST",
        2 => "WAKELOCK",
        _ => "UNKNOWN",
    };
    let message = format!("[Rust Core] Received event '{}' for package '{}'", event_str, package_name);
    // 调用 C++ 层的日志函数
    ffi::log_info(&message);
}