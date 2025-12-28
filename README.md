# Simple ARM64 OS (QEMU Virt) - Refactored

Welcome to the **refactored version** of **Simple OS**, a minimal "bare metal" operating system designed for the **AArch64 (ARMv8)** architecture. This project runs on the **QEMU Emulator** using the generic `virt` machine board.

It has been restructured from a monolithic kernel into a modular design with proper separation of drivers, headers, and kernel logic.

---

##  Features

### Phase 1: Bare Metal Boot & Exceptions
- **Bootloader (`boot.S`)**: Handles core parking, stack setup (`sp`), and BSS zeroing.
- **Exception Vector Table (`vectors.S`)**: A fully compliant ARM64 vector table handling Synchronous (SVC/Data Abort) and Asynchronous (IRQ) exceptions.
- **Robust Context Saving**: Saves General Purpose Registers (`x0-x30`) and Exception Link Registers (`ELR_EL1`, `SPSR_EL1`) for crash stability.

### Phase 2: Interrupt Controller (GIC)
- **GICv2 Support**: Configures the Distributor (GICD) and CPU Interface (GICC) in `src/drivers/gic.c`.
- **UART Interrupts**: Replaces polling with interrupt-driven I/O to enable non-blocking shell input.

### Phase 3: The Timer & Heartbeat
- **ARM Generic Timer**: Uses the EL1 Physical Timer (`CNTP_EL0`) to generate periodic interrupts (100Hz), managed in `src/drivers/timer.c`.
- **System Tick**: Drives the Scheduler, acting as the heartbeat for multitasking.

### Phase 4: Memory Management (MMU)
- **Virtual Memory**: Enables the MMU with 4KB paging via `src/mm.c`.
- **Identity Mapping**:
    - **Device Memory**: `0x00000000` - `0x3FFFFFFF` (nGnRnE, Strict Ordering).
    - **Normal Memory**: `0x40000000` - `0x7FFFFFFF` (Cacheable, Write-Back).

### Phase 5: Multitasking & Scheduler
- **Round-Robin Scheduler**: Automatically switches execution between tasks every 10ms, implemented in `src/sched.c`.
- **Process Control Blocks (PCB)**: Tracks `pid`, `state`, and `stack_pointer` for multiple tasks.
- **Background Workers**: Runs concurrent worker threads (Task A/B) alongside the Shell.

### Phase 6: Interactive Shell & IO
- **Ring Buffer**: Decouples high-speed ISRs from the Shell logic in `src/utils.c` and `src/shell.c`.
- **Power Efficiency**: Uses `wfi` (Wait For Interrupt) to sleep the CPU when idle.

### Phase 7: Filesystem & Apps
- **RamFS (Read-Only Filesystem)**: An in-memory virtual filesystem supporting `ls` (list) and `cat` (read) commands.
- **Calculator App**: A built-in sub-application with its own input loop and logic.
- **System Shutdown**: Implements the `exit` command using the **PSCI** (Power State Coordination Interface) standard.

---

##  Prerequisites

### macOS (Apple Silicon)
Install via Homebrew:
```bash
brew install qemu aarch64-elf-gcc
```

### Linux (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install qemu-system-arm gcc-aarch64-linux-gnu
```

---

## Building & Running

### 1. Build the Kernel

Navigate to the `Simple_OS` directory and run `make`:

```bash
cd Simple_OS
make
```

**Artifact**: `kernel.elf`

### 2. Run in QEMU

Launch the emulator with the kernel attached.

```bash
make run
```

You can now explore the filesystem and run apps:

```
--- Simple OS Kernel Booting ---
[INIT] Initializing MMU... Done.
[INIT] Initializing GIC... Done.
[INIT] Creating Background Tasks... Done.

Interactive Shell Ready.
OS> ls
Files:
  readme.txt
  credits.txt
  calc.app*

OS> calc
--- Simple Calc v1.0 ---
CALC> 10 + 50
Result: 60
CALC> exit

OS> exit
Shutting down...
```

**Exit QEMU**: The `exit` command now works! (Or press `Ctrl+A`, `x`).

### 3. Clean

```bash
make clean
```

---

## Architecture Overview

### Memory Map (Virtual = Physical)

We utilize a 1:1 Identity Map for simplicity.

| Region | Start Address | Type | Description |
| --- | --- | --- | --- |
| **Flash** | `0x00000000` | Device | Boot ROM |
| **GIC Dist** | `0x08000000` | Device | GIC Distributor |
| **GIC CPU** | `0x08010000` | Device | GIC CPU Interface |
| **UART0** | `0x09000000` | Device | PL011 Serial Port |
| **RAM** | `0x40000000` | Normal | Kernel Code, RamFS, Task Stacks |

### File Structure

The project now follows a modular directory structure:

```
Simple_OS/
├── Makefile            # Build configuration
├── README.md           # Project documentation
├── linker.ld           # Address space layout
├── include/            # Header files (API Definitions)
│   ├── gic.h
│   ├── mm.h
│   ├── sched.h
│   ├── timer.h
│   ├── uart.h
│   └── utils.h
└── src/
    ├── boot.S          # Bootloader
    ├── vectors.S       # Vector Table
    ├── switch.S        # Context Switch
    ├── kernel.c        # Main Kernel Entry
    ├── shell.c         # Shell & Apps
    ├── sched.c         # Scheduler
    ├── mm.c            # Memory Management
    ├── utils.c         # Utilities
    └── drivers/        # Hardware Drivers
        ├── gic.c       # Interrupt Controller
        ├── timer.c     # System Timer
        └── uart.c      # One-way & Buffered UART
```

---

## Next Steps (Future Work)

* **User Mode (EL0)**: Drop from EL1 to EL0 to run safer user applications.
* **System Calls (`svc`)**: Allow User Mode apps to request OS services (print, exit).
* **Dynamic Memory**: Implement a simple `malloc` using a bump allocator.
