// File: ~/projects/CRFzit/daemon/rust/build.rs

fn main() {
    cxx_build::bridge("src/lib.rs")
        .file("src/cxx_bridge_impl.rs") // 未来可以把实现放在这里
        .compile("crfzit_cxx");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/cxx_bridge_impl.rs");
}