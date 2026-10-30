[简体中文](README.md) | **English**

# elib_arm_cm_backtrace

ARM Cortex-M stack backtrace library

## Project Overview

elib_arm_cm_backtrace is a stack backtrace library for ARM Cortex-M microcontrollers. The project helps embedded developers diagnose and debug where program crashes and exceptions happen, locating the faulty code by walking the call stack.

## Features

- ARM Cortex-M stack frame unwinding
- Backtracing in exception/interrupt context
- Call chain output
- Support for multiple Cortex-M architectures (M0/M3/M4/M7, etc.)
- **Automatic MSP/PSP dual-stack detection** (enables backtracing from RTOS task exceptions)
- Multi-compiler support (GCC/Keil/IAR)
- Runs on Cortex-M0 without activating the frame pointer, though `-fno-omit-frame-pointer` is recommended for higher-quality backtraces
- Falls back to stack scanning when the frame pointer is unavailable or unreliable
- Robust under stack overflow conditions (Thumb address validation)
- Memory-safe design

## Quick Start

### Adding the Project

Add the relevant `elib_arm_cm_backtrace` source files to your project:

```c
#include "elib_arm_cm_backtrace.h"
```

### Basic Usage

Call the backtrace function when the program crashes or an exception occurs:

```c
elib_arm_cm_backtrace();  // auto-detect MSP/PSP
```

## Configuration

Depending on your chip model, you may need to configure the following parameters:

- Stack start address
- Flash and RAM regions

## Usage

### 1. Configure the Stack Range (optional)

```c
#define ARM_CM_BACKTRACE_STACK_START  0x20000000
#define ARM_CM_BACKTRACE_STACK_END    0x20030000
#define ARM_CM_BACKTRACE_MAX_DEPTH    32
```

### 2. Configure the Flash Region (optional, for code address validation)

```c
#define ARM_CM_BACKTRACE_FLASH_START  0x08000000
#define ARM_CM_BACKTRACE_FLASH_END    0x08100000
```

> Code address validation filters garbage data during a stack overflow and improves reliability in an M0 environment.

### 3. Build Requirements

For best results, frame pointer optimization is recommended:

**GCC/Clang:**
```makefile
CFLAGS += -fno-omit-frame-pointer
```

**Keil (ARM Compiler 5/6):**
- Add this under Target Options → ARM Compiler → Misc Controls: `-fno-omit-frame-pointer`
- Or use this in code: `#pragma import(__use_full_frame_pointer)`

**IAR Embedded Workbench:**
- Project → Options → C/C++ Compiler → Optimizations
- Enable the frame pointer option

**M0 environment notes:**
- The library works even without `-fno-omit-frame-pointer` enabled, relying on the stack scanning feature
- If the application uses interrupts, consider the effect on stack usage
- Make sure the stack bounds are configured correctly for your chip (ARM_CM_BACKTRACE_STACK_START/END)

### 4. Usage Example

#### Bare-Metal Application (MSP only)

```c
#include "elib_arm_cm_backtrace.h"

// Implement the output callback (UART/RTT/other)
void my_print(const char *fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Send to the UART
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    elib_arm_cm_backtrace();  // auto-detect MSP/PSP
    while(1);  // spin forever to prevent a reset
}
```

#### RTOS Application (PSP)

The library detects automatically whether the exception came from MSP or PSP; no extra configuration is needed:

```c
void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);
    elib_arm_cm_backtrace();  // read the exception frame from the right stack automatically
    while(1);
}
```

#### Specifying the Stack Pointer Manually (advanced usage)

```c
void HardFault_Handler(void) {
    elib_arm_cm_backtrace_init(my_print);

    // Option 1: auto-detect (recommended)
    elib_arm_cm_backtrace();

    // Option 2: force PSP
    elib_arm_cm_backtrace_ex(ARM_CM_BACKTRACE_SP_PSP);

    // Option 3: force MSP
    elib_arm_cm_backtrace_ex(ARM_CM_BACKTRACE_SP_MSP);

    while(1);
}
```

### 5. MSP/PSP Dual-Stack Support

ARM Cortex-M processors use a dual-stack mechanism:

| Scenario | Stack pointer used | Library support |
|------|-------------|---------|
| Bare-metal application (thread mode) | MSP | Automatic |
| RTOS task (thread mode) | PSP | Automatic |
| Exception/interrupt handler | MSP | Automatic |

The library inspects the `EXC_RETURN` value to tell which stack holds the exception frame, with no user intervention.

### 6. Output Format

Normal output:
```
Stack Backtrace:0x08005ABC 0x08004F20 0x08003A84
```

On stack overflow or an invalid FP (stack scan results are printed as well):
```
Stack Backtrace:0x08005ABC 0x08004F20
Stack Scan:0x08003A84 0x08002100
```

> `Stack Backtrace` - the trustworthy call chain (from the hardware exception frame and any usable frame pointers)
> `Stack Scan` - fallback information (from scanning stack memory)

### 7. Offline Resolution

Resolve the addresses with addr2line:

```bash
arm-none-eabi-addr2line -e your_firmware.elf -f 0x08005ABC 0x08004F20 0x08003A84
```

## API Reference

```c
// Initialize: register the output callback
void elib_arm_cm_backtrace_init(elib_arm_cm_backtrace_print_fn print_fn);

// Main backtrace function (auto-detects MSP/PSP)
void elib_arm_cm_backtrace(void);

// Extended backtrace function (the stack pointer type can be forced)
void elib_arm_cm_backtrace_ex(elib_arm_cm_backtrace_sp_type_t sp_type);

// Backtrace with an exception stack frame (the frame pointer is passed in by the user)
void elib_arm_cm_backtrace_from_frame(void *stack_frame);

// Get the current MSP value
uint32_t elib_arm_cm_backtrace_get_msp(void);

// Get the current PSP value
uint32_t elib_arm_cm_backtrace_get_psp(void);
```

## License

MIT License
