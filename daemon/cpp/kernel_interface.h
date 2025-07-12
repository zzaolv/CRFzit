#ifndef CRFZIT_KERNEL_INTERFACE_H
#define CRFZIT_KERNEL_INTERFACE_H

#include <string>
#include "crfzit/daemon/rust/src/lib.rs.h" // 包含 cxx 生成的头文件

namespace crfzit::cpp_kernel {

// 实现 lib.rs 中声明的 extern "C++" 函数
void execute_freeze(rust::Str package_name, ffi::FreezeMode mode);
void execute_unfreeze(rust::Str package_name);
void execute_kill(rust::Str package_name);

} // namespace crfzit::cpp_kernel

#endif //CRFZIT_KERNEL_INTERFACE_H