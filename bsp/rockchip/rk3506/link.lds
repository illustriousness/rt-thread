OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_reset)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = RTT_FIRMWARE_BASE;

    __text_start = .;
    .text :
    {
        KEEP(*(.vectors))
        *(.text)
        *(.text.*)
        *(.init)
        *(.fini)

        . = ALIGN(4);
        __rt_utest_tc_tab_start = .;
        KEEP(*(UtestTcTab))
        __rt_utest_tc_tab_end = .;

        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        . = ALIGN(4);
        __ve_table_start = .;
        KEEP(*(SORT(*.VarExpTab.*)))
        __ve_table_end = .;

        . = ALIGN(4);
        __rtmsymtab_start = .;
        KEEP(*(RTMSymTab))
        __rtmsymtab_end = .;

        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;
    } :text = 0
    __text_end = .;

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } :text

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } :text

    . = ALIGN(4);
    __rodata_start = .;
    .rodata : { *(.rodata) *(.rodata.*) } :text
    __rodata_end = .;

    . = ALIGN(4);
    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        PROVIDE(__ctors_end__ = .);
    } :data

    .dtors :
    {
        PROVIDE(__dtors_start__ = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE(__dtors_end__ = .);
    } :data

    . = ALIGN(8);
    __data_start = .;
    .data :
    {
        *(.data)
        *(.data.*)
    } :data
    __data_end = .;

    . = ALIGN(8);
    __bss_start = .;
    .bss (NOLOAD) :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
    } :data
    __bss_end = .;

    _end = .;

    .stab 0 : { *(.stab) } :NONE
    .stabstr 0 : { *(.stabstr) } :NONE
    .comment 0 : { *(.comment) } :NONE
}
