// 文件路径: /CRFzit/daemon/rust/build.rs

fn main() {
    cxx_build::bridge("src/lib.rs") // 包含 #[cxx::bridge] 的文件
        .file("src/logic_controller.rs") // 包含 "Rust" 函数实现的文件
        .flag_if_supported("-std=c++17")
        .compile("crfzit_cxx_bridge");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/logic_controller.rs");
}