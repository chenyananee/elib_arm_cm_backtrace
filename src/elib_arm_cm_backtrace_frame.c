/**
 * @file elib_arm_cm_backtrace_frame.c
 * @brief ARM Cortex-M Stack Frame Parsing Implementation
 * @author ChenYanan
 * @license MIT
 */

#include "elib_arm_cm_backtrace_frame.h"
#include "elib_arm_cm_backtrace_core.h"

void elib_arm_cm_backtrace_get_frame_from_exception(
    const void *exception_frame,
    elib_arm_cm_backtrace_frame_t *frame
) {
    const uint32_t *exc_frame = (const uint32_t *)exception_frame;

    // 提取基本寄存器（使用偏移量）
    frame->pc = exc_frame[ARM_CM_BACKTRACE_PC_OFFSET / 4];
    frame->lr = exc_frame[ARM_CM_BACKTRACE_LR_OFFSET / 4];

    frame->sp = (uint32_t)exception_frame;
    frame->fp = 0;
}