/**
 * @file elib_arm_cm_backtrace_config.h
 * @brief ARM Cortex-M Stack Backtrace Library Configuration
 * @author ChenYanan
 * @license MIT
 */

#ifndef ARM_CM_BACKTRACE_CONFIG_H
#define ARM_CM_BACKTRACE_CONFIG_H

// ========== 地址范围配置 ==========
// 栈范围配置 (用户可重定义)
#ifndef ARM_CM_BACKTRACE_STACK_START
#define ARM_CM_BACKTRACE_STACK_START  0x20000000
#endif

#ifndef ARM_CM_BACKTRACE_STACK_END
#define ARM_CM_BACKTRACE_STACK_END    0x20010000
#endif

// Flash 区域配置（用于代码地址验证）
#ifndef ARM_CM_BACKTRACE_FLASH_START
#define ARM_CM_BACKTRACE_FLASH_START  0x08000000
#endif

#ifndef ARM_CM_BACKTRACE_FLASH_END
#define ARM_CM_BACKTRACE_FLASH_END    0x08100000
#endif

// 回溯最大深度
#ifndef ARM_CM_BACKTRACE_MAX_DEPTH
#define ARM_CM_BACKTRACE_MAX_DEPTH    20
#endif

#endif // ARM_CM_BACKTRACE_CONFIG_H