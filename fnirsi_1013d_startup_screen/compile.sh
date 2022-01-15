arm-none-eabi-gcc -Wall -Wno-write-strings -Wno-char-subscripts -fno-stack-protector -DNO_STDLIB=1 -O3 -mcpu=cortex-m3 -mthumb -c *.c
arm-none-eabi-ld -T stm32f103-64k.ld -nostdlib -Map=test.map -o stm_usb.elf *.o
arm-none-eabi-objcopy stm_usb.elf -O ihex stm_usb.hex
arm-none-eabi-size stm_usb.elf
rm *.o
rm *.map
#rm *.elf

#openocd -f STM32F103C8T6.cfg -c init -c targets -c halt -c "flash write_image erase stm_usb.elf" -c "verify_image stm_usb.elf" -c "reset run"
