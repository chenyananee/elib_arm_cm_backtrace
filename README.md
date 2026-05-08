
# elib_arm_cm_backtrace

ARM Cortex-M 栈回溯库

## 项目简介

elib_arm_cm_backtrace 是一个用于 ARM Cortex-M 微控制器的栈回溯（Stack Backtrace）库。该项目旨在帮助嵌入式开发者诊断和调试程序崩溃、异常等问题的发生位置，通过回溯调用栈来定位问题代码。

## 功能特性

- ARM Cortex-M 栈帧 unwinding
- 异常/中断上下文下的堆栈回溯
- 调用链信息输出
- 支持多种 Cortex-M 架构 (M0/M3/M4/M7 等)
- **自动检测 MSP/PSP 双栈**（支持 RTOS 任务异常回溯）
- 多编译器支持 (GCC/Keil/IAR)
- 可在不激活帧指针的情况下在Cortex-M0上运行，但建议开启 `-fno-omit-frame-pointer` 以获得更高质量的回溯
- 在帧指针不可用或不可靠时可退回到栈扫描
- 在栈溢出条件下具有鲁棒性（Thumb地址验证）
- 内存安全设计

## 快速开始

### 引入项目

将 `elib_arm_cm_backtrace` 相关源文件添加到你的项目中：

```c
#include "elib_arm_cm_backtrace.h"
```

### 基本使用

在程序崩溃或异常发生时调用回溯函数：

```c
elib_arm_cm_backtrace();  // 自动检测 MSP/PSP
```

## 配置说明

根据你的芯片型号，可能需要配置以下参数：

- 堆栈起始地址
- Flash 和 RAM 区域

## 使用方法

### 1. 配置栈范围（可选）

```c
#define ARM_CM_BACKTRACE_STACK_START  0x20000000
#define ARM_CM_BACKTRACE_STACK_END    0x20030000
#define ARM_CM_BACKTRACE_MAX_DEPTH    32
```

### 2. 配置 Flash 区域（可选，用于代码地址验证）

```c
#define ARM_CM_BACKTRACE_FLASH_START  0x08000000
#define ARM_CM_BACKTRACE_FLASH_END    0x08100000
```

> 代码地址验证在栈溢出期间过滤垃圾数据，在M0环境下提升可靠性。

### 3. 编译要求

为了获得最佳结果，建议使用帧指针优化：

**GCC/Clang:**
```makefile
CFLAGS += -fno-omit-frame-pointer
```

**Keil (ARM Compiler 5/6):**
- 在 Target Options → ARM Compiler → Misc Controls 中添加: `-fno-omit-frame-pointer`
- 或在代码中使用: `#pragma import(__use_full_frame_pointer)`

**IAR Embedded Workbench:**
- Project → Options → C/C++ Compiler → Optimizations
- 启用 frame pointer 选项

**M0环境说明:**
- 库即使在未启用 `-fno-omit-frame-pointer` 的情况下也能工作，通过依赖栈扫描功能
- 如果应用程序使用中断，请考虑对栈使用的影响
- 确保为您的芯片正确配置了栈边界（ARM_CM_BACKTRACE_STACK_START/END）

### 4. 使用示例

#### 裸机应用（仅使用 MSP）

```c
#include "elib_arm_cm_backtrace.h"

// 实现输出回调（串口/RTT/其他）
void my_print(const char *fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // 发送到串口
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    elib_arm_cm_backtrace();  // 自动检测 MSP/PSP
    while(1);  // 死循环防止复位
}
```

#### RTOS 应用（使用 PSP）

库会自动检测异常来源使用的是 MSP 还是 PSP，无需额外配置：

```c
void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    elib_arm_cm_backtrace();  // 自动从正确的栈读取异常帧
    while(1);
}
```

#### 手动指定栈指针（高级用法）

```c
void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    
    // 方式1: 自动检测（推荐）
    elib_arm_cm_backtrace();
    
    // 方式2: 手动指定使用 PSP
    elib_arm_cm_backtrace_ex(ARM_CM_BACKTRACE_SP_PSP);
    
    // 方式3: 手动指定使用 MSP
    elib_arm_cm_backtrace_ex(ARM_CM_BACKTRACE_SP_MSP);
    
    while(1);
}
```

### 5. MSP/PSP 双栈支持

ARM Cortex-M 处理器使用双栈机制：

| 场景 | 使用的栈指针 | 库的支持 |
|------|-------------|---------|
| 裸机应用（线程模式） | MSP | 自动支持 |
| RTOS 任务（线程模式） | PSP | 自动支持 |
| 异常/中断处理程序 | MSP | 自动支持 |

库通过检查 `EXC_RETURN` 值自动判断异常帧位于哪个栈上，无需用户干预。

### 6. 输出格式

正常输出：
```
Stack Backtrace:0x08005ABC 0x08004F20 0x08003A84
```

在栈溢出或FP无效时（额外输出栈扫描结果）：
```
Stack Backtrace:0x08005ABC 0x08004F20
Stack Scan:0x08003A84 0x08002100
```

> `Stack Backtrace` - 可信的调用链（来自硬件异常帧和可用的帧指针）
> `Stack Scan` - 备选信息（来自栈内存扫描）

### 7. 离线解析

使用 addr2line 解析地址：

```bash
arm-none-eabi-addr2line -e your_firmware.elf -f 0x08005ABC 0x08004F20 0x08003A84
```

## API 参考

```c
// 初始化：注册输出回调
void elib_arm_cm_backtrace_init(elib_arm_cm_backtrace_print_fn print_fn);

// 栈回溯主函数（自动检测 MSP/PSP）
void elib_arm_cm_backtrace(void);

// 扩展栈回溯函数（可手动指定栈指针类型）
void elib_arm_cm_backtrace_ex(elib_arm_cm_backtrace_sp_type_t sp_type);

// 带异常栈帧的回溯（用户传入栈帧指针）
void elib_arm_cm_backtrace_from_frame(void *stack_frame);

// 获取当前 MSP 值
uint32_t elib_arm_cm_backtrace_get_msp(void);

// 获取当前 PSP 值
uint32_t elib_arm_cm_backtrace_get_psp(void);
```

## 许可证

MIT License
