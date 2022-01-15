MEMORY
{
	ram  : org = 0x7FFFFFE0, len = 32M
}

SECTIONS
{
  . = ORIGIN(ram);
  PRGM_START = .;
  .text : 
  {
    build/Debug/GNU_ARM-Linux/start.o (.text)
    *(.text);
  } >ram

  .data :
  {
    *(.data);
    *(.rodata);
  } >ram
  PRGM_END = .;

  BSS_START = .;
  .bss : 
  {
    *(.bss);
  } >ram
  BSS_END = .;
}

__program_size = PRGM_END - PRGM_START;
