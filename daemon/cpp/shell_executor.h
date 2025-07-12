#ifndef CRFZIT_SHELL_EXECUTOR_H
#define CRFZIT_SHELL_EXECUTOR_H

#include <string>
#include "crfzit/daemon/rust/src/lib.rs.h" // 包含 cxx 生成的头文件

namespace crfzit::cpp_shell {

// 实现 lib.rs 中声明的 extern "C++" 函数
void execute_network_policy(rust::Str package_name, ffi::NetworkPolicy policy);

} // namespace crfzit::cpp_shell

#endif //CRFZIT_SHELL_EXECUTOR_H