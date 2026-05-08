/**
 * @file usage.c
 * @brief ARM Cortex-M Stack Backtrace Usage Example
 * @author ChenYanan
 * @license MIT
 */

#include "elib_arm_cm_backtrace.h"
#include <stdio.h>

// 示例：串口输出回调
void uart_print(const char *fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // 这里应该替换为实际的串口发送函数
    // 例如：HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    // 或者：SEGGER_RTT_WriteString(0, buffer);
    printf("%s", buffer);
}

// ========================================
// 场景1: 裸机应用（仅使用 MSP）
// ========================================
void HardFault_Handler_BareMetal(void) {
    elib_arm_cm_backtrace_init(uart_print);
    elib_arm_cm_backtrace();  // 自动检测，裸机环境下会从 MSP 读取

    // 死循环防止复位，便于查看输出
    while (1);
}

// ========================================
// 场景2: RTOS 应用（任务使用 PSP）
// ========================================
// RTOS 任务通常使用 PSP（Process Stack Pointer）
// 当任务触发异常时，异常帧被压入 PSP
// elib_arm_cm_backtrace() 会自动检测并从 PSP 读取异常帧
void HardFault_Handler_RTOS(void) {
    elib_arm_cm_backtrace_init(uart_print);
    elib_arm_cm_backtrace();  // 自动检测 PSP/MSP

    while (1);
}

// ========================================
// 场景3: 手动指定栈指针（高级用法）
// ========================================
void HardFault_Handler_Manual(void) {
    elib_arm_cm_backtrace_init(uart_print);

    // 方式1: 自动检测（推荐）
    elib_arm_cm_backtrace();

    // 方式2: 强制使用 PSP
    // elib_arm_cm_backtrace_ex(ARM_CM_BACKTRACE_SP_PSP);

    // 方式3: 强制使用 MSP
    // elib_arm_cm_backtrace_ex(ARM_CM_BACKTRACE_SP_MSP);

    while (1);
}

// ========================================
// 场景4: 使用自定义栈帧调用
// ========================================
void my_fault_handler(void *exc_frame) {
    elib_arm_cm_backtrace_init(uart_print);
    elib_arm_cm_backtrace_from_frame(exc_frame);

    while (1);
}

// ========================================
// 场景5: 调试时查看 MSP/PSP 值
// ========================================
void debug_print_stack_pointers(void) {
    uint32_t msp = elib_arm_cm_backtrace_get_msp();
    uint32_t psp = elib_arm_cm_backtrace_get_psp();

    printf("MSP: 0x%08X\r\n", msp);
    printf("PSP: 0x%08X\r\n", psp);
}
