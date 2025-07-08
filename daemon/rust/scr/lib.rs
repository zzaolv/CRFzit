// /CRFzit/daemon/rust/src/lib.rs
#[cxx::bridge(namespace = "crfzit::rust_bridge")]
mod ffi {
    // 这个函数将从C++调用
    extern "Rust" {
        fn get_initial_message() -> String;
    }
}

// 这是暴露给C++的函数的Rust实现
pub fn get_initial_message() -> String {
    "Hello from Rust! CRFzit core logic is alive.".to_string()
}

// 这个文件只是为了让 cxx-build 正常工作，我们后续会在这里添加更多内容
// /CRFzit/daemon/rust/src/cxx_bridge.rs
// (暂时为空)