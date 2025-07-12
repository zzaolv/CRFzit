fn main() {
    // 告诉 cxx-build 在哪里寻找 bridge 定义
    cxx_build::bridge("src/lib.rs")
        .compile("crfzit_core"); // 定义编译产物的名称

    // 告诉 Cargo 何时重新运行此脚本
    println!("cargo:rerun-if-changed=src/lib.rs");
    // 如果有其他包含文件，也需要在这里添加
    // println!("cargo:rerun-if-changed=src/my_other_file.rs");
}