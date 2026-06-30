ENTRY(_start)

PHDRS
{
  text   PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data   PT_LOAD FLAGS(6);
}

SECTIONS {
  . = 0x80000;
  __image_start = .;

  .text : {
    KEEP(*(.text.boot))
    *(.text*)
  } :text
  . = ALIGN(0x1000);
  .rodata : {
    *(.rodata*)
  } :rodata
  . = ALIGN(0x1000);
  __code_end = .;

  .data : {
    *(.data*)
  } :data
  . = ALIGN(0x1000);
  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(64);
    __smp_mailbox_start = .;
    . += 0x10000;
    __smp_mailbox_end = .;
    . = ALIGN(8);
    __bss_end = .;
  } :data
  . = ALIGN(0x1000);
  __image_end = .;

  . = ALIGN(16);
    __cpu_stack_base = .;
    . += 0x4000 * 4;
    __cpu_stack_end = .;

    __proc_stacks_base = .;
    . += 0x10000 * 20 * 4;
}
