/*
 * Linker script for Renesas RA4M1 (R7FA4M1AB)
 * 256KB code flash, 32KB SRAM
 */

MEMORY
{
    /* Code flash: 0x00000000 - 0x0003FFFF (256 KB) */
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K

    /* SRAM0: 0x20000000 - 0x20007FFF (32 KB) */
    RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 32K

    /* Data flash: 0x40100000 - 0x40101FFF (8 KB) - for NVM */
    DATA_FLASH (r) : ORIGIN = 0x40100000, LENGTH = 8K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx*)
        . = ALIGN(4);
    } > FLASH

    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
        . = ALIGN(4);
    } > FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array*)))
        PROVIDE_HIDDEN(__init_array_end = .);
        . = ALIGN(4);
    } > FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
        . = ALIGN(4);
    } > FLASH

    _etext = .;
    _sidata = .;

    .data : AT(_etext)
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __heap_start = .;
        . = . + 0x1000; /* 4 KB heap */
        __heap_end = .;
    } > RAM

    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        __stack_start = .;
        . = . + 0x1000; /* 4 KB stack */
        __stack_end = .;
    } > RAM

    /* Data flash for NVM */
    .data_flash (NOLOAD) :
    {
        *(.data_flash*)
    } > DATA_FLASH

    /* Heap/Stack limits */
    PROVIDE(__heap_start__ = __heap_start);
    PROVIDE(__heap_end__ = __heap_end);
    PROVIDE(__stack_end__ = __stack_end__);

    /* Top of stack for startup */
    _estack = __stack_end__;
}

/* Flash sector info for NVM */
PROVIDE(FLASH_BASE = 0x00000000);
PROVIDE(FLASH_SIZE = 256K);
PROVIDE(FLASH_END  = 0x0003FFFF);
PROVIDE(DATA_FLASH_BASE = 0x40100000);
PROVIDE(DATA_FLASH_SIZE = 8K);
PROVIDE(SRAM_BASE = 0x20000000);
PROVIDE(SRAM_SIZE = 32K);