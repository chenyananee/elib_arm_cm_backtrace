/**
 * @file elib_arm_cm_backtrace_core.h
 * @brief ARM Cortex-M Stack Backtrace Library Core Definitions
 * @author ChenYanan
 * @license MIT
 */

#ifndef ARM_CM_BACKTRACE_CORE_H
#define ARM_CM_BACKTRACE_CORE_H

#include <stdint.h>
#include "elib_arm_cm_backtrace_config.h"

// 栈帧寄存器偏移量
#define ARM_CM_BACKTRACE_R0_OFFSET    0
#define ARM_CM_BACKTRACE_R1_OFFSET    4
#define ARM_CM_BACKTRACE_R2_OFFSET    8
#define ARM_CM_BACKTRACE_R3_OFFSET    12
#define ARM_CM_BACKTRACE_R12_OFFSET   16
#define ARM_CM_BACKTRACE_LR_OFFSET    20
#define ARM_CM_BACKTRACE_PC_OFFSET    24
#define ARM_CM_BACKTRACE_XPSR_OFFSET  28

// 现代ARM Cortex-M异常帧结构固定(基本异常帧总是32字节，无论是否有FPU)
// 在硬件异常时，SP已指向正确的异常栈帧基址(由硬件自动处理)

// ========== EXC_RETURN 相关定义 ==========
// EXC_RETURN 用于判断异常来源的栈类型
// 当异常发生时，LR 被设置为 EXC_RETURN 值
// bit 2: SPSEL - 0=返回后使用MSP, 1=返回后使用PSP
// bit 3: Mode - 0=返回到Thread模式, 1=返回到Handler模式

// 典型的 EXC_RETURN 值
#define ARM_CM_BACKTRACE_EXC_RETURN_THREAD_MSP   0xFFFFFFF9U  // Thread模式, 使用MSP
#define ARM_CM_BACKTRACE_EXC_RETURN_THREAD_PSP   0xFFFFFFFDU  // Thread模式, 使用PSP
#define ARM_CM_BACKTRACE_EXC_RETURN_HANDLER_MSP  0xFFFFFFF1U  // Handler模式, 使用MSP

// EXC_RETURN bit 定义
#define ARM_CM_BACKTRACE_EXC_RETURN_SPSEL_BIT    (1U << 2)   // bit 2: 栈指针选择
#define ARM_CM_BACKTRACE_EXC_RETURN_MODE_BIT     (1U << 3)   // bit 3: 模式选择

// 判断 EXC_RETURN 是否指示使用 PSP
#define ARM_CM_BACKTRACE_EXC_RETURN_USES_PSP(exc_ret) \
    (((exc_ret) & ARM_CM_BACKTRACE_EXC_RETURN_SPSEL_BIT) != 0)

// 判断 EXC_RETURN 是否来自 Thread 模式
#define ARM_CM_BACKTRACE_EXC_RETURN_FROM_THREAD(exc_ret) \
    (((exc_ret) & ARM_CM_BACKTRACE_EXC_RETURN_MODE_BIT) == 0)

#endif // ARM_CM_BACKTRACE_CORE_H