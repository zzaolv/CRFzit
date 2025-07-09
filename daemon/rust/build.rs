// 文件路径: /CRFzit/daemon/rust/build.rs (修正版)

fn main() {
    cxx_build::bridge("src/lib.rs")
        // logic_controller.rs 包含 LogicController 的实现，必须被编译
//        .file("src/logic_controller.rs") 
        .flag_if_supported("-std=c++17")
        .compile("crfzit_cxx_bridge");

    println!("cargo:rerun-if-changed=src/lib.rs");
//    println!("cargo:rerun-if-changed=src/logic_controller.rs");
}