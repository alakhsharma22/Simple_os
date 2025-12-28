ENTRY(_start)

SECTIONS
{
    /* QEMU load address for virt machine is 0x40000000 */
    . = 0x40000000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    }

    /* Allocate stack space (e.g., 4KB) */
    . = ALIGN(16);
    . = . + 0x1000;
    _stack_top = .;
}
