# elib_arm_cm_backtrace

ARM Cortex-M Stack Backtrace Library

## Project Overview

elib_arm_cm_backtrace is a stack backtrace library designed for ARM Cortex-M microcontrollers. This project aims to assist embedded developers in diagnosing and debugging program crashes and exceptions by tracing the call stack to locate the problematic code.

## Features

- ARM Cortex-M stack frame unwinding
- Stack backtrace under exception/interrupt context
- Output of call chain information
- Support for multiple Cortex-M architectures (M0/M3/M4/M7, etc.)
- Multi-compiler support (GCC/Keil/IAR)
- Works on Cortex-M0 without frame pointer activation, though `-fno-omit-frame-pointer` compilation option is recommended for better backtrace quality
- Can fallback to stack scanning when frame pointer is unavailable or unreliable
- Robust under stack overflow conditions (Thumb address validation)
- Memory-safe design

## Quick Start

### Include the Project

Add the `elib_arm_cm_backtrace` source files to your project:

```c
#include "elib_arm_cm_backtrace.h"
```

### Basic Usage

Call the backtrace function when a crash or exception occurs:

```c
elib_arm_cm_backtrace();
```

## Configuration Notes

Depending on your chip model, you may need to configure the following parameters:

- Stack start address
- Flash and RAM regions

## Usage

### 1. Configure Stack Range (Optional)

```c
#define ARM_CM_BACKTRACE_STACK_START  0x20000000
#define ARM_CM_BACKTRACE_STACK_END    0x20030000
#define ARM_CM_BACKTRACE_MAX_DEPTH    32
```

### 2. Configure Flash Region (Optional, for code address validation)

```c
#define ARM_CM_BACKTRACE_FLASH_START  0x08000000
#define ARM_CM_BACKTRACE_FLASH_END    0x08100000
```

> Code address validation filters garbage data during stack overflow and improves reliability in M0 environments.

### 3. Build Requirements

For best results, use frame pointer optimization:

## Quick Start

### Include the Project

Add the `elib_arm_cm_backtrace` source files to your project:

```c
#include "elib_arm_cm_backtrace.h"
```

### Basic Usage

Call the backtrace function when a crash or exception occurs:

```c
elib_arm_cm_backtrace();
```

## Configuration Notes

Depending on your chip model, you may need to configure the following parameters:

- Stack start address
- Flash and RAM regions

## Usage

### 1. Configure Stack Range (Optional)

```c
#define ARM_CM_BACKTRACE_STACK_START  0x20000000
#define ARM_CM_BACKTRACE_STACK_END    0x20030000
#define ARM_CM_BACKTRACE_MAX_DEPTH    32
```

### 2. Configure Flash Region (Optional, for code address validation)

```c
#define ARM_CM_BACKTRACE_FLASH_START  0x08000000
#define ARM_CM_BACKTRACE_FLASH_END    0x08100000
```

> Code address validation filters garbage data during stack overflow.

### 3. Build Requirements

For best results, use frame pointer optimization:

**GCC/Clang:**
```makefile
CFLAGS += -fno-omit-frame-pointer
```

**Notes for M0 Environments:**
- The library works even without `-fno-omit-frame-pointer` by relying on stack scanning
- If your application uses interrupts, consider the impact of stack usage
- Ensure that stack boundaries (ARM_CM_BACKTRACE_STACK_START/END) are properly configured for your chip

### 4. Usage Example

```c
#include "elib_arm_cm_backtrace.h"

// Implement output callback (UART/RTT/other)
void my_print(const char *fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Send to UART
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    elib_arm_cm_backtrace();
    while(1);  // Infinite loop to prevent reset
}
```

### 5. Special Considerations for M0 Chips

For Cortex-M0 platforms, the output may look slightly different from more advanced cores since the frame pointer chain method might not be available:
```
Stack Backtrace:0x08005ABC 0x08004F20
Stack Scan:0x08003A84 0x08002100 0x080011A0
```

> In M0 environments, the primary backtrace might be shorter but can be supplemented by the additional addresses found through stack scanning in the `Stack Scan` section.

### 6. Offline Parsing

Normal output:
```
Stack Backtrace:0x08005ABC 0x08004F20 0x08003A84
```

On stack overflow or FP failure (additional stack scan output):
```
Stack Backtrace:0x08005ABC 0x08004F20
Stack Scan:0x08003A84 0x08002100
```

> `Stack Backtrace` - trusted call chain (from hardware exception frame and available frame pointers)
> `Stack Scan` - fallback info (from stack memory scan)

**Keil (ARM Compiler 5/6):**
- Target Options → ARM Compiler → Misc Controls: add `-fno-omit-frame-pointer`
- Or in code: `#pragma import(__use_full_frame_pointer)`

**IAR Embedded Workbench:**
- Project → Options → C/C++ Compiler → Optimizations
- Enable frame pointer option

### 6. Usage Example

```c
#include "elib_arm_cm_backtrace.h"

// Implement output callback (UART/RTT/other)
void my_print(const char *fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Send to UART
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    elib_arm_cm_backtrace();
    while(1);  // Infinite loop to prevent reset
}
```

### 5. Offline Parsing

Normal output:
```
Stack Backtrace:0x08005ABC 0x08004F20 0x08003A84
```

On stack overflow (FP chain fails), additional stack scan output:
```
Stack Backtrace:0x08005ABC 0x08004F20
Stack Scan:0x08003A84 0x08002100
```

> `Stack Backtrace` - trusted call chain (from frame pointer chain)
> `Stack Scan` - fallback info (from stack memory scan)

Use addr2line to parse:
```bash
arm-none-eabi-addr2line -e your_firmware.elf -f 0x08005ABC 0x08004F20 0x08003A84
```

## License

MIT License

