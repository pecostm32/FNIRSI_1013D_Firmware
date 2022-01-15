openocd -f STM32F103C8T6.cfg -c init -c targets -c halt -c "flash write_image erase stm_usb.elf" -c "verify_image stm_usb.elf" -c "reset run"
