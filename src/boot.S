.section .text
.global _start

_start:
    // Read MPIDR_EL1 to check Core ID
    mrs x0, mpidr_el1
    and x0, x0, #3      
    cbz x0, master  

hang:
    wfe
    b hang

master:
    ldr x0, =vectors
    msr vbar_el1, x0
    // Setup Stack Pointer. RAM starts at 0x40000000.
    ldr x0, =_stack_top
    mov sp, x0

    ldr x0, =_bss_start
    ldr x1, =_bss_end

    cmp x0, x1
    b.eq run_kernel

clear_bss:
    str xzr, [x0], #8  
    cmp x0, x1
    b.lt clear_bss

run_kernel:
    bl kernel_main
    b hang
