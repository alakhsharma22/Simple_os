.global cpu_switch_to

cpu_switch_to:
    /* x0 = pointer to 'prev' task struct */
    /* x1 = pointer to 'next' task struct */

    /* 1. Save 'prev' context */
    /* 
       pid (8 bytes), state (4 bytes + 4 padding) = 16 bytes offset.
    */
    add x8, x0, #16  // x8 = &prev->context
    
    stp x19, x20, [x8, #0]
    stp x21, x22, [x8, #16]
    stp x23, x24, [x8, #32]
    stp x25, x26, [x8, #48]
    stp x27, x28, [x8, #64]
    stp x29, x30, [x8, #80] // FP and LR
    
    mov x9, sp
    str x9, [x8, #96]      // Save SP

    /* 2. Load 'next' context */
    add x8, x1, #16  // x8 = &next->context

    ldp x19, x20, [x8, #0]
    ldp x21, x22, [x8, #16]
    ldp x23, x24, [x8, #32]
    ldp x25, x26, [x8, #48]
    ldp x27, x28, [x8, #64]
    ldp x29, x30, [x8, #80]

    ldr x9, [x8, #96]      // Load SP
    mov sp, x9

    ret // Jumps to the address stored in x30 (LR)
