#include "shell_executor.h"
#include "log.h"
#include <cstdio>
#include <string>
#include <memory>

namespace {
void run_command(const std::string& cmd) {
    LOGI("Executing shell command: %s", cmd.c_str());
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        LOGE("Failed to execute command: %s", cmd.c_str());
        return;
    }
    // 可以选择读取输出来进行调试
    // char buffer[128];
    // while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    //     LOGI("SHELL_OUT: %s", buffer);
    // }
    pclose(pipe);
}
}

namespace crfzit::cpp_shell {

void execute_network_policy(rust::Str package_name_rs, ffi::NetworkPolicy policy) {
    std::string package_name(package_name_rs);
    LOGI("Executing NETWORK POLICY for '%s' with policy %d", package_name.c_str(), static_cast<int>(policy));

    // 这是一个非常简化的示例，实际的 iptables/nftables 规则会更复杂
    // 需要获取应用的 UID，并基于 UID 设置规则
    // run_command("UID=$(pm dump " + package_name + " | grep userId= | cut -d= -f2)");
    // ... 然后使用 $UID
    
    switch (policy) {
        case ffi::NetworkPolicy::NET_BLOCK_ALL:
            // run_command("iptables -A OUTPUT -m owner --uid-owner $UID -j DROP");
            LOGW("iptables logic for NET_BLOCK_ALL is not fully implemented.");
            break;
        case ffi::NetworkPolicy::NET_ALLOW_ALL:
            // run_command("iptables -D OUTPUT -m owner --uid-owner $UID -j DROP"); // Example of removal
            LOGW("iptables logic for NET_ALLOW_ALL is not fully implemented.");
            break;
        case ffi::NetworkPolicy::NET_WIFI_ONLY:
            LOGW("iptables logic for NET_WIFI_ONLY is not implemented yet.");
            break;
        default:
            LOGW("Unsupported network policy.");
    }
}

} // namespace crfzit::cpp_shell