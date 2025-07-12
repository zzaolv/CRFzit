// 使用 cxx::bridge 宏定义 FFI 接口
#[cxx::bridge(namespace = "crfzit::rust_bridge")]
mod ffi {
    // --- Enums and Structs Shared between C++ and Rust ---
    // 这些枚举需要与 protobuf 中的定义保持一致，但使用 cxx 支持的类型
    // 注意：cxx 不支持 proto 枚举直接转换，所以我们通常在 C++ 端进行映射。
    // 这里我们使用 i32 作为替代，或者定义新的 cxx 兼容枚举。

    enum FreezeMode {
        V2_FREEZE,
        V1_FREEZE,
        SIGSTOP,
        KILL,
    }

    enum NetworkPolicy {
        NET_ALLOW_ALL,
        NET_WIFI_ONLY,
        NET_BLOCK_ALL,
    }

    // --- Rust functions exposed to C++ ---
    // C++ 代码将能够调用这些函数。
    // 它们是进入 Rust 核心逻辑的入口点。
    extern "Rust" {
        /// 初始化 Rust 核心逻辑，包括日志、状态管理器等。
        /// 必须在调用任何其他 rust_* 函数之前调用。
        fn rust_init_core();

        /// C++ gRPC 服务端收到来自 Observer 的事件后调用此函数。
        /// Rust 核心将根据事件内容和应用配置进行决策。
        fn rust_on_framework_event(uid: i32, package_name: &str, event_type: i32);

        /// C++ 层调用此函数来强制更新仪表盘数据。
        /// Rust 会收集当前状态并调用 C++ 的回调函数来推送数据。
        fn rust_update_dashboard_data();
    }

    // --- C++ functions callable from Rust ---
    // Rust 代码将能够调用这些函数。
    // 它们是 Rust 决策引擎执行操作的出口。
    // 我们将它们组织在不同的 C++ 命名空间中，以保持清晰。
    unsafe extern "C++" {
        // --- 内核接口回调 ---
        #[namespace = "crfzit::cpp_kernel"]
        fn execute_freeze(package_name: &str, mode: FreezeMode);

        #[namespace = "crfzit::cpp_kernel"]
        fn execute_unfreeze(package_name: &str);

        #[namespace = "crfzit::cpp_kernel"]
        fn execute_kill(package_name: &str);

        // --- Shell 执行器回调 ---
        #[namespace = "crfzit::cpp_shell"]
        fn execute_network_policy(package_name: &str, policy: NetworkPolicy);

        // --- gRPC 推送回调 ---
        // Rust 逻辑准备好数据后，调用这些 C++ 函数将数据推送回客户端。
        // 注意：我们不直接传递 protobuf 对象，而是传递基本类型或 cxx 支持的结构体。
        // C++ 层负责将这些数据打包成 protobuf 消息。
        #[namespace = "crfzit::cpp_grpc"]
        fn push_global_stats(cpu_percent: f32, mem_kb: i64, net_down_bps: i64, net_up_bps: i64);

        // --- 日志回调 ---
        #[namespace = "crfzit::cpp_log"]
        fn log_info(tag: &str, message: &str);

        #[namespace = "crfzit::cpp_log"]
        fn log_error(tag: &str, message: &str);
    }
}

// -----------------------------------------------------------------------------
// --- Rust 侧的模块化实现 ---
// -----------------------------------------------------------------------------
mod core; // 引入 core 模块

// Rust FFI 入口函数的实际实现
// 这些函数将调用到我们真正的业务逻辑模块中（例如 state_manager, policy_engine）。

fn rust_init_core() {
    // 设置日志记录器
    env_logger::builder().filter_level(log::LevelFilter::Info).init();
    log::info!("Rust Core: Initializing...");

    // 初始化核心模块
    core::initialize();

    log::info!("Rust Core: Initialized successfully.");
}

fn rust_on_framework_event(uid: i32, package_name: &str, event_type: i32) {
    log::info!(
        "Rust Core: Received framework event for package '{}' (uid={}), type={}",
        package_name, uid, event_type
    );
    // 将事件分派给策略引擎
    core::get_policy_engine().handle_event(package_name, event_type);
}

fn rust_update_dashboard_data() {
    log::info!("Rust Core: Updating dashboard data...");
    // 触发资源监视器更新数据
    core::get_resource_monitor().trigger_update();
}