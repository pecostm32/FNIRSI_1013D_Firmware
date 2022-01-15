MEMORY
{
	ram  : org = 0x000000000, len = 32K
}

SECTIONS
{
  . = ORIGIN(ram);
  .text : 
  {
    build/Debug/GNU_ARM-Linux/start.o (.text)
    *(.text);
  } >ram

	__bootloader_size = SIZEOF(.text);
}