# Simple OS - Technical Documentation (Refactored)

## Overview
This document details the internal architecture of the refactored **Simple OS**, a bare-metal ARM64 microkernel. The system has been split from a monolithic file into a modular structure to improve maintainability and separation of concerns. It still runs at **EL1 (Exception Level 1)**.

---

## 1. Core Assembly Files

### `src/boot.S` (The Bootloader)
**Purpose:** The hardware entry point. It bridges the gap between the QEMU reset vector and the C environment.
* **`_start`**: The very first instruction executed.
* **Core Selection**: Reads `MPIDR_EL1` to check the CPU ID. If it is not Core 0, it parks the core in an infinite `wfe` (Wait For Event) loop.
* **Stack Setup**: Loads the address of `_stack_top` (defined in the linker) into the Stack Pointer (`sp`).
* **BSS Clearing**: Zeros out the `.bss` section to ensure uninitialized global variables in C are actually zero.
* **Vector Table Install**: Writes the address of `vectors` to `vbar_el1` (Vector Base Address Register).
* **Kernel Jump**: Branches (`bl`) to `kernel_main`.

### `src/vectors.S` (Exception Vector Table)
**Purpose:** Defines how the CPU handles crashes, system calls, and hardware interrupts.
* **Alignment**: The table is aligned to 2KB (`.align 11`), a strict requirement of AArch64.
* **`kernel_entry` Macro**:
    * Saves generic registers `x0`-`x30` to the stack.
    * **Crucial**: Saves `elr_el1` (Return Address) and `spsr_el1` (Processor State) to preserve the interrupted task's context.
* **`kernel_exit` Macro**: Restores all registers and executes `eret` to return to the task.
* **Handlers**:
    * `__handle_sync`: Catches crashes (Data Aborts) or bad instructions.
    * `__handle_irq`: Catches hardware interrupts (Timer, UART).

### `src/switch.S` (Context Switching)
**Purpose:** The low-level mechanism to swap execution between two threads.
* **`cpu_switch_to(struct task* prev, struct task* next)`**:
    * This function is **cooperative**. It is called explicitly by the scheduler.
    * It only saves **Callee-Saved Registers** (`x19`-`x28`, `fp`, `lr`, `sp`).
    * *Why?* Because Caller-Saved registers (`x0`-`x18`) are already presumed "dead" or saved by the ISR before `schedule()` was called.

---

## 2. Modular C Implementation

The monolithic `kernel.c` has been split into the following components:

### Kernel Core
* **`src/kernel.c`**: The main entry point `kernel_main`. It is now responsible **only** for calling initialization functions (`mmu_init`, `gic_init`, `timer_init`, `task_create`) and launching the shell.
* **`src/shell.c`**: Contains the interactive `shell_loop`, the `ramdisk` file system, and user applications like `app_calculator`.
* **`src/sched.c`**: Contains the Scheduler logic (`schedule`), Process Control Blocks (`struct task`), and the `task_create` function.
* **`src/utils.c`**: Helper functions including `atoi`, `putint`, `strcmp`, `gets` (input buffering), and `power_off` (PSCI).

### Drivers (`src/drivers/`)
* **`src/drivers/uart.c`**:
    * Handles UART hardware directly (`UART0_DR`, `UART0_FR` registers).
    * Implements `uart_putc` (polled output) and the low-level ISR `uart_handle_irq` which feeds the ring buffer.
* **`src/drivers/gic.c`**:
    * Configures the GICv2 Distributor and CPU Interface.
    * Implements the Master Interrupt Handler `handle_irq`, which routes interrupts to the specific driver handlers (UART vs Timer).
* **`src/drivers/timer.c`**:
    * Configures the ARM Generic Timer (`cntp_tval_el0`).
    * Implements `timer_handler` which resets the timer and triggers a context switch (`schedule`).

### Memory Management
* **`src/mm.c`**:
    * **`mmu_init()`**: Sets up 1:1 Identity Mapping using 4KB pages.
    * **Device Memory**: Mapped as `nGnRnE` (Non-Gathering, Non-Reordering). Vital for MMIO.
    * **Normal Memory**: Mapped as Cacheable. Vital for performance.

---

## 3. Headers & Interfaces (`include/`)
All public API definitions are now centralized in the `include/` directory:
* `uart.h`: UART I/O functions.
* `gic.h`: Interrupt controller init and EOI.
* `timer.h`: Timer init and handler.
* `sched.h`: Task structures and scheduling functions.
* `mm.h`: MMU initialization.
* `utils.h`: String and helper utilities.

---

## 4. Build & Layout

### `linker.ld` (Memory Layout)
**Purpose:** Tells the linker how to map the compiled code into physical memory.
* **Start Address**: `0x40000000` (Start of RAM on QEMU virt).
* **Sections**:
    * `.text`: Executable code.
    * `.rodata`: Read-only strings/constants.
    * `.data`: Initialized global variables.
    * `.bss`: Uninitialized globals.
* **Stack Alignment**: Ensures the stack top is aligned to **16 bytes**. Misalignment here causes instant crashes in AArch64.

### `Makefile`
**Purpose:** Automation of the build process.
* **Updated SRCS**: Now includes all sub-directories: `src/*.c` and `src/drivers/*.c`.
* **Includes**: Added `-I include` flag to allow source files to find headers cleanly.
* **Flags**:
    * `-ffreestanding`: No standard library.
    * `-nostdlib`: No startup files.
    * **`-mgeneral-regs-only`**: CRITICAL. Prevents usage of FPU/SIMD registers (`q0`-`q31`) in kernel mode.
