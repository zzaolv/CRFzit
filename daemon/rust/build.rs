// /CRFzit/daemon/rust/build.rs
fn main() {
    cxx_build::bridge("src/lib.rs")
        .file("src/cxx_bridge.rs") // 假设我们有一个专门的文件处理cxx相关的实现
        .flag_if_supported("-std=c++17")
        .compile("crfzit_cxx_bridge");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/cxx_bridge.rs");
    println!("cargo:rerun-if-changed=include/cxx_bridge.h");
}