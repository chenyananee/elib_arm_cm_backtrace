/**
 * @file elib_arm_cm_backtrace.c
 * @brief ARM Cortex-M Stack Backtrace Library
 * @author ChenYanan
 * @license MIT
 */

#include "elib_arm_cm_backtrace.h"
#include "elib_arm_cm_backtrace_core.h"
#include <stdint.h>
#include <string.h>

// ========== 编译器检测 ==========
#if defined(__GNUC__) || defined(__clang__)
    #define ARM_CM_BT_COMPILER_GCC    1
#elif defined(__ARMCC_VERSION) || defined(__KEIL__)
    #define ARM_CM_BT_COMPILER_KEIL   1
#elif defined(__IAR_SYSTEMS_ICC__)
    #define ARM_CM_BT_COMPILER_IAR    1
#else
    #define ARM_CM_BT_COMPILER_GCC    1  // 默认使用GCC
#endif

// 全局回调函数指针
static elib_arm_cm_backtrace_print_fn g_print_fn = NULL;

// 初始化回调函数
void elib_arm_cm_backtrace_init(elib_arm_cm_backtrace_print_fn print_fn) {
    g_print_fn = print_fn;
}

// ========== 编译器特定的内联汇编实现 ==========

// GCC / Clang 内联汇编
#if defined(ARM_CM_BT_COMPILER_GCC)
static inline uint32_t get_fp(void) {
    register uint32_t result;
    // 如果不使用帧指针，尝试直接使用汇编读取r7，如果不存在则返回0
    __asm volatile ("mov %0, r7" : "=r" (result));
    return result;
}

static inline uint32_t get_sp(void) {
    register uint32_t result;
    __asm volatile ("mov %0, sp" : "=r" (result));
    return result;
}

static inline uint32_t get_msp(void) {
    register uint32_t result;
    __asm volatile ("mrs %0, msp" : "=r" (result));
    return result;
}

static inline uint32_t get_psp(void) {
    register uint32_t result;
    __asm volatile ("mrs %0, psp" : "=r" (result));
    return result;
}

static inline uint32_t get_exc_return(void) {
    register uint32_t result;
    __asm volatile ("mov %0, lr" : "=r" (result));
    return result;
}

// Keil ARM Compiler
#elif defined(ARM_CM_BT_COMPILER_KEIL)
static inline uint32_t get_fp(void) {
    // Keil编译器获取r7值的替代方法
    register uint32_t result __asm("r7");
    return result;
}

static inline uint32_t get_sp(void) {
    register uint32_t result __asm("sp");
    return result;
}

static inline uint32_t get_msp(void) {
    register uint32_t result;
    __asm volatile ("mrs result, msp");
    return result;
}

static inline uint32_t get_psp(void) {
    register uint32_t result;
    __asm volatile ("mrs result, psp");
    return result;
}

static inline uint32_t get_exc_return(void) {
    register uint32_t result __asm("lr");
    return result;
}

// IAR Embedded Workbench
#elif defined(ARM_CM_BT_COMPILER_IAR)
static inline uint32_t get_fp(void) {
    uint32_t result;
    // 尝试直接使用寄存器变量
    __asm("mov %0, r7" : "=r" (result));
    return result;
}

static inline uint32_t get_sp(void) {
    uint32_t result;
    __asm("mov %0, sp" : "=r" (result));
    return result;
}

static inline uint32_t get_msp(void) {
    uint32_t result;
    __asm("mrs %0, msp" : "=r" (result));
    return result;
}

static inline uint32_t get_psp(void) {
    uint32_t result;
    __asm("mrs %0, psp" : "=r" (result));
    return result;
}

static inline uint32_t get_exc_return(void) {
    uint32_t result;
    __asm("mov %0, lr" : "=r" (result));
    return result;
}
#endif

// 检查地址是否在有效范围内
static int is_valid_address(uint32_t addr) {
    return (addr >= ARM_CM_BACKTRACE_STACK_START &&
            addr < ARM_CM_BACKTRACE_STACK_END);
}

// 检查地址是否4字节对齐
static int is_aligned(uint32_t addr) {
    return (addr % 4) == 0;
}

// 检查是否是有效的Thumb指令地址
// ARM Thumb指令地址最低位必须为1
static int is_valid_code_address(uint32_t addr) {
    return (addr >= ARM_CM_BACKTRACE_FLASH_START &&
            addr < ARM_CM_BACKTRACE_FLASH_END &&
            (addr & 1));
}

// 扫描栈寻找有效的返回地址（用于内存溢出时的备用回溯）
// 从栈底向栈顶扫描，查找所有可能是返回地址的Thumb指令地址
static int scan_stack_for_return_addrs(uint32_t stack_bottom, uint32_t stack_top,
                                        uint32_t *addrs, int max_addrs) {
    int count = 0;
    uint32_t *p = (uint32_t *)stack_bottom;

    // 确保4字节对齐
    if (!is_aligned((uint32_t)p)) {
        p = (uint32_t *)(((uint32_t)p + 3) & ~3);
    }

    // 从栈底向栈顶扫描（低地址到高地址）
    // 每次步进4字节
    while (p < (uint32_t *)stack_top && count < max_addrs) {
        uint32_t val = *p;
        if (is_valid_code_address(val)) {
            // 排除第一个（可能是异常地址）
            addrs[count++] = val;
        }
        p++;
    }

    return count;
}

// 执行实际的栈回溯
static void do_backtrace(elib_arm_cm_backtrace_frame_t *frame) {
    if (g_print_fn == NULL) {
        return;
    }

    uint32_t depth = 0;
    uint32_t fp = frame->fp;
    uint32_t sp = frame->sp;
    uint32_t pc = frame->pc;
    uint32_t lr = frame->lr;

    // 输出前缀 "Stack Backtrace:"
    g_print_fn("Stack Backtrace:");

    // 优先使用LR作为第一个回溯地址
    // PC可能指向触发异常的无效指令（如调用NULL函数指针）
    // LR是异常返回地址，通常是有效的调用者位置
    uint32_t first_addr = lr;
    if (!is_valid_code_address(lr) && is_valid_code_address(pc)) {
        first_addr = pc;
    }
    g_print_fn("0x%08X", first_addr);
    depth++;

    // 通过FP链回溯
    int fp_chain_valid = 1;  // 标记FP链是否有效
    int skip_fp_unwind = 0;  // 指示是否应跳过FP回溯（在M0或没使能FP时有用）

    // 如果FP无效（0或超出栈范围），则跳过FP链回溯，直接使用栈扫描
    if (fp == 0 || !is_valid_address(fp) || !is_aligned(fp)) {
        skip_fp_unwind = 1;
        fp_chain_valid = 0;
    }

    if (!skip_fp_unwind) {
        while (depth < ARM_CM_BACKTRACE_MAX_DEPTH) {
            // 检查FP有效性
            if (fp == 0) {
                fp_chain_valid = 0;
                break;
            }

            // FP应该在栈范围内，且4字节对齐
            if (!is_valid_address(fp) || !is_aligned(fp)) {
                fp_chain_valid = 0;
                break;
            }

            // 从FP+4获取LR（上一帧的返回地址）
            uint32_t *fp_ptr = (uint32_t *)fp;
            uint32_t prev_lr = fp_ptr[1];

            // 额外的安全检查：LR应该是有效的代码地址（Thumb模式）
            // 内存溢出时，栈数据可能被破坏，通过代码地址验证可以过滤垃圾数据
            if (!is_valid_code_address(prev_lr)) {
                fp_chain_valid = 0;
                break;
            }

            // 输出地址
            g_print_fn(" 0x%08X", prev_lr);
            depth++;

            // 移动到上一帧：FP指向当前帧的栈底，即上一帧的SP
            // 在帧指针链中，FP指向保存的FP，FP+4是保存的LR
            // FP本身的值就是上一帧的FP
            fp = fp_ptr[0];
        }
    }

    g_print_fn("\r\n");

    // 对于Cortex-M0或其他未启用FP的情况，如果FP链无法建立，或者FP看起来不正确，
    // 或者FP链不提供太多额外信息，则使用栈扫描作为主要的补充手段
    int fp_provides_value = 1;  // 检查FP是否可靠

    // 如果FP是0或不在栈内（表明可能未初始化FP或使用Cortex-M0），优先使用栈扫描
    if (frame->fp == 0 || !is_valid_address(frame->fp)) {
        fp_provides_value = 0;
    }

    // 即使FP链有效，但在某些系统上（如M0），栈扫描仍然是获取更多回溯信息的好方式
    if (!fp_chain_valid || !fp_provides_value) {
        uint32_t scan_addrs[16];
        int scan_count = scan_stack_for_return_addrs(
            sp,
            ARM_CM_BACKTRACE_STACK_END,
            scan_addrs,
            16
        );

        if (scan_count > 0) {
            g_print_fn("Stack Scan:");
            for (int i = 0; i < scan_count; i++) {
                // 排除已经在FP链中输出的地址
                int duplicate = 0;
                if (fp_chain_valid) {
                    // 如果FP链有效，检查是否重复
                    // 简单的重复检查：仅检查首次地址是否相同
                    if (i == 0 && scan_addrs[i] == first_addr) {
                        duplicate = 1;
                    }
                }

                if (!duplicate) {
                    g_print_fn("0x%08X", scan_addrs[i]);
                    if (i < scan_count - 1) {
                        g_print_fn(" ");
                    }
                }
            }
            g_print_fn("\r\n");
        }
    }
}

// 主回溯函数
void elib_arm_cm_backtrace(void) {
    // 获取 EXC_RETURN（当前 LR 值）
    // 在异常处理程序中，LR 被硬件设置为 EXC_RETURN
    uint32_t exc_return = get_exc_return();

    // 根据 EXC_RETURN 判断异常来源使用的栈指针
    // bit 2: SPSEL - 0=使用MSP, 1=使用PSP
    uint32_t sp;
    if (ARM_CM_BACKTRACE_EXC_RETURN_USES_PSP(exc_return)) {
        // 异常来自使用 PSP 的线程模式（如 RTOS 任务）
        // 异常帧被压入 PSP
        sp = get_psp();
    } else {
        // 异常来自使用 MSP 的线程模式或嵌套异常
        // 异常帧被压入 MSP
        sp = get_msp();
    }

    // 只有条件性地获取FP，如果平台不支持可以跳过
    uint32_t fp = get_fp();

    // 验证FP是否有效：如果不指向合理的栈空间，可能是垃圾值（如M0上未初始化的寄存器）
    if (!is_valid_address(fp) || fp == 0) {
        fp = 0; // 将无效的FP设置为0表示不可用
    }

    // 创建初始帧
    elib_arm_cm_backtrace_frame_t frame = {
        .pc = 0,  // 需要从栈帧获取
        .lr = 0,  // 需要从栈帧获取
        .sp = sp,
        .fp = fp
    };

    // 从异常栈帧提取PC和LR
    // 硬件在异常发生时自动压栈，SP已指向栈帧起始位置（无论是否有FPU）
    const uint32_t *exc_frame = (const uint32_t *)sp;
    frame.pc = exc_frame[ARM_CM_BACKTRACE_PC_OFFSET / 4];
    frame.lr = exc_frame[ARM_CM_BACKTRACE_LR_OFFSET / 4];

    do_backtrace(&frame);
}

// 扩展栈回溯函数
void elib_arm_cm_backtrace_ex(elib_arm_cm_backtrace_sp_type_t sp_type) {
    uint32_t sp;

    switch (sp_type) {
        case ARM_CM_BACKTRACE_SP_PSP:
            sp = get_psp();
            break;
        case ARM_CM_BACKTRACE_SP_MSP:
            sp = get_msp();
            break;
        case ARM_CM_BACKTRACE_SP_AUTO:
        default:
            {
                // 自动检测：通过 EXC_RETURN 判断
                uint32_t exc_return = get_exc_return();
                if (ARM_CM_BACKTRACE_EXC_RETURN_USES_PSP(exc_return)) {
                    sp = get_psp();
                } else {
                    sp = get_msp();
                }
            }
            break;
    }

    // 只有条件性地获取FP，如果平台不支持可以跳过
    uint32_t fp = get_fp();

    // 验证FP是否有效：如果不指向合理的栈空间，可能是垃圾值（如M0上未初始化的寄存器）
    if (!is_valid_address(fp) || fp == 0) {
        fp = 0; // 将无效的FP设置为0表示不可用
    }

    // 创建初始帧
    elib_arm_cm_backtrace_frame_t frame = {
        .pc = 0,  // 需要从栈帧获取
        .lr = 0,  // 需要从栈帧获取
        .sp = sp,
        .fp = fp
    };

    // 从异常栈帧提取PC和LR
    const uint32_t *exc_frame = (const uint32_t *)sp;
    frame.pc = exc_frame[ARM_CM_BACKTRACE_PC_OFFSET / 4];
    frame.lr = exc_frame[ARM_CM_BACKTRACE_LR_OFFSET / 4];

    do_backtrace(&frame);
}

// 获取 MSP
uint32_t elib_arm_cm_backtrace_get_msp(void) {
    return get_msp();
}

// 获取 PSP
uint32_t elib_arm_cm_backtrace_get_psp(void) {
    return get_psp();
}

// 从指定栈帧开始回溯
void elib_arm_cm_backtrace_from_frame(void *stack_frame) {
    elib_arm_cm_backtrace_frame_t frame;
    elib_arm_cm_backtrace_get_frame_from_exception(stack_frame, &frame);

    // 从栈帧获取FP（如果有的话）
    // 这里简化处理，假设用户传入的是异常栈帧
    uint32_t fp = get_fp();
    frame.fp = fp;

    do_backtrace(&frame);
}