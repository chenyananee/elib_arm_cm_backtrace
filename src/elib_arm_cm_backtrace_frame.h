/**
 * @file elib_arm_cm_backtrace_frame.h
 * @brief ARM Cortex-M Stack Frame Definitions
 * @author ChenYanan
 * @license MIT
 */

#ifndef ARM_CM_BACKTRACE_FRAME_H
#define ARM_CM_BACKTRACE_FRAME_H

#include <stdint.h>

// 栈帧结构
typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
} elib_arm_cm_backtrace_exception_frame_t;

// 回溯帧结构
typedef struct {
    uint32_t pc;
    uint32_t lr;
    uint32_t sp;
    uint32_t fp;
} elib_arm_cm_backtrace_frame_t;

// 从异常栈帧提取初始回溯帧
void elib_arm_cm_backtrace_get_frame_from_exception(
    const void *exception_frame,
    elib_arm_cm_backtrace_frame_t *frame
);

#endif // ARM_CM_BACKTRACE_FRAME_H