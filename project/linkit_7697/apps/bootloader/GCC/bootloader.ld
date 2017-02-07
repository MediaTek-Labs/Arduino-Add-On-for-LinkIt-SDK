
/*
 *        MT7687 CM4 Loader Memory Map
 *
 */

ENTRY(ResetISR)

MEMORY
{
    ROM_CODE        (arx) : ORIGIN = 0x0,           LENGTH = 0x00005000        /* 20k  */
    TCMRAM          (arw) : ORIGIN = 0x00100000,    LENGTH = 0x00010000        /* 64k  */
    XIP_LOADER_CODE (arx) : ORIGIN = 0x10000000,    LENGTH = 0x00008000        /* 32k  */
    XIP_CODE        (arx) : ORIGIN = 0x1006C000,    LENGTH = 0x00040000        /* 256k */
    SYSRAM          (arw) : ORIGIN = 0x20000000,    LENGTH = 0x00040000        /* 256k */

    SYSRAM_DATA_BSS (arw) : ORIGIN = 0x2003C000,    LENGTH = 0x00004000        /* 16k for loader and rom's data section */

    /* for debug/temperoary */
    SYSRAM_MIDDLE   (arw) : ORIGIN = 0x20020000,    LENGTH = 0x00020000

    /* XIP(0x10000000) remapping by SYSRAM bus*/
    XIP_SYSRAM_CODE (arx) : ORIGIN = 0x30000000,    LENGTH = 0x00100000        /* 1MB  */
}

SECTIONS
{
    . = ORIGIN(SYSRAM);

    _text_lma_start = LOADADDR(.text);
    .text :
    {
        _text_start = .;

        *(.except_vectors)
        *(.reset_isr)
        *(.text .text.*)
        *(.rodata .rodata.*)

        _text_end = .;

    } > SYSRAM AT > XIP_LOADER_CODE

    . = ALIGN(4);
    _data_lma_start = LOADADDR(.data);
    .data :
    {
        _data_start = .;
        KEEP(*(.ramTEXT))

        . = ALIGN(4);

        *(.data .data.*)

        _data_end = .;

    } > SYSRAM

    . = ALIGN(4);
    .bss :
    {
        _bss_start = .;

        *(.bss .bss.*)
        *(COMMON)

        _bss_end   = .;

    } > SYSRAM

    . = ALIGN(4);
    .heap :
    {
        __end__ = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    }
}

