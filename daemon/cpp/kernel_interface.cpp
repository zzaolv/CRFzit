#include "kernel_interface.h"
#include "log.h"
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <vector>

namespace {
// 辅助函数：根据包名获取所有PID
std::vector<pid_t> get_pids_for_package(const std::string& package_name) {
    // 这是一个简化的实现。在Android上，通常通过 `ps -A -o PID,NAME`
    // 然后过滤包名来获取。更可靠的方法是扫描 /proc/*/cmdline。
    // 为了演示，我们暂时留空。
    LOGW("get_pids_for_package for %s is a placeholder!", package_name.c_str());
    return {};
}
} // namespace

namespace crfzit::cpp_kernel {

void execute_freeze(rust::Str package_name_rs, ffi::FreezeMode mode) {
    std::string package_name(package_name_rs);
    LOGI("Executing FREEZE for package '%s' with mode %d", package_name.c_str(), static_cast<int>(mode));

    auto pids = get_pids_for_package(package_name);
    if (pids.empty()) {
        LOGW("No PIDs found for package '%s', cannot freeze.", package_name.c_str());
        return;
    }

    switch (mode) {
        case ffi::FreezeMode::SIGSTOP:
            LOGI("Using SIGSTOP to freeze %zu processes.", pids.size());
            for (pid_t pid : pids) {
                kill(pid, SIGSTOP);
            }
            break;
        case ffi::FreezeMode::V2_FREEZE:
        case ffi::FreezeMode::V1_FREEZE:
            // TODO: 实现 cgroup freezer 逻辑
            // 1. 找到正确的 cgroup 路径 (e.g., /dev/cpuset/...)
            // 2. 将 PID 写入 cgroup.procs
            // 3. 写入 "FROZEN" 到 freezer.state
            LOGW("cgroup freezer is not implemented yet.");
            break;
        default:
            LOGW("Unsupported freeze mode.");
            break;
    }
}

void execute_unfreeze(rust::Str package_name_rs) {
    std::string package_name(package_name_rs);
    LOGI("Executing UNFREEZE for package '%s'", package_name.c_str());
    
    auto pids = get_pids_for_package(package_name);
    if (pids.empty()) {
        return;
    }
    LOGI("Using SIGCONT to unfreeze %zu processes.", pids.size());
    for (pid_t pid : pids) {
        kill(pid, SIGCONT);
    }
    // TODO: 实现 cgroup unfreeze 逻辑 (写入 "THAWED")
}

void execute_kill(rust::Str package_name_rs) {
    std::string package_name(package_name_rs);
    LOGI("Executing KILL for package '%s'", package_name.c_str());

    auto pids = get_pids_for_package(package_name);
    if (pids.empty()) {
        return;
    }
    LOGI("Using SIGKILL to kill %zu processes.", pids.size());
    for (pid_t pid : pids) {
        kill(pid, SIGKILL);
    }
}

} // namespace crfzit::cpp_kernel