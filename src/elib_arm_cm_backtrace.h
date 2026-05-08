/**
 * @file elib_arm_cm_backtrace.h
 * @brief ARM Cortex-M Stack Backtrace Library
 * @author ChenYanan
 * @license MIT
 */

#ifndef ARM_CM_BACKTRACE_H
#define ARM_CM_BACKTRACE_H

#include <stdarg.h>
#include "elib_arm_cm_backtrace_frame.h"

// 输出回调函数类型
typedef void (*elib_arm_cm_backtrace_print_fn)(const char *fmt, ...);

// 栈指针类型
typedef enum {
    ARM_CM_BACKTRACE_SP_AUTO = 0,   // 自动检测（通过 EXC_RETURN）
    ARM_CM_BACKTRACE_SP_MSP,        // 使用 Main Stack Pointer
    ARM_CM_BACKTRACE_SP_PSP         // 使用 Process Stack Pointer
} elib_arm_cm_backtrace_sp_type_t;

// 初始化：注册输出回调
void elib_arm_cm_backtrace_init(elib_arm_cm_backtrace_print_fn print_fn);

// 栈回溯主函数（自动检测 MSP/PSP）
void elib_arm_cm_backtrace(void);

// 扩展栈回溯函数（可指定栈指针类型）
void elib_arm_cm_backtrace_ex(elib_arm_cm_backtrace_sp_type_t sp_type);

// 带异常栈帧的回溯（用户传入栈帧指针）
void elib_arm_cm_backtrace_from_frame(void *stack_frame);

// 获取当前 MSP 值
uint32_t elib_arm_cm_backtrace_get_msp(void);

// 获取当前 PSP 值
uint32_t elib_arm_cm_backtrace_get_psp(void);

#endif // ARM_CM_BACKTRACE_H