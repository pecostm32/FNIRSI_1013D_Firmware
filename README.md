# 27-02-2025
# Check out EEVblog on this: https://www.eevblog.com/forum/testgear/fnirsi-1013d-100mhz-tablet-oscilloscope/2725/#lastPost
# Member Atlan there has made improvements to my code and has it's own github page for it. https://github.com/Atlan4/Fnirsi1013D/tree/main/latest%20firmware%20version


# FNIRSI_1013D_Firmware
New firmware for the FNIRSI-1013D osciloscope.

This new firmware is offered without any warrenty and I take no responsibility for any damage.

!!! It is only suited for the 1013D. It won't work on the 1014D. !!!

This repository is a result of the hacking of the original FNIRSI 1013D firmware. To make it easier to just get the new firmware code, this repository is created.

During the hacking and development phase discoveries where made that there are differences between the oscilloscopes in the field. An important one is the different displays that are used. To make it work with these different models a sector on the SD card has been allocated to hold the display configuration. To this moment only one major deviation needed this mod. The more standard types can use the default configuration.

In the "fnirsi_1013d_scope" dist folder there are the configuration files for both the deviated and the standard one. In the "How_to_load_scope.txt" file you can find the instructions for loading these files to the SD card. The file "configuration_file.txt" explains the configuration file.

In version 0.004 extra settings are added to the display configuration file to allow for swapping the touch coordinates.

Firmware is at this location: https://github.com/pecostm32/FNIRSI_1013D_Firmware/tree/main/fnirsi_1013d_scope/dist/Debug/GNU_ARM-Linux

There are four folders with source code projects of which a minimum of two are needed to build a binary that can be loaded onto the SD card that is housed in the scope.

For a version with a startup screen that shows PECOs sCOPE three projects are needed:
1) "fnirsi_1013d_sd_card_bootloader" which loads the startup screen code and executes that
2) "fnirsi_1013d_startup_screen" which shows the startup screen and loads and executes the actual scope code
3) "fnirsi_1013d_scope" this is the actual scope code

For a version without the startup screen only two projects are needed:
1) "fnirsi_1013d_startup_from_sd_card" which starts the FPGA and loads and executes the actual scope code
2) "fnirsi_1013d_scope" this is the actual scope code

The second option is the fastest since it does not wait to show the startup screen, but this project has not been adapted for the new display configuration setup nor has it been tested with the latest code.

!!! Be aware that all dd actions with the SD card mentioned below are done on the block device and not a partition. So for example /dev/sdc and not /dev/sdc1. The umount command has to be done on the partition(s) !!!

To load the new firmware on the scope one has to make sure the SD card is partioned correctly.

1)  Connect the scope to the computer via USB.
2)  Turn on the scope and start the USB connection via the main menu option.
3)  Wait until the file manager window opens. (Only if auto mount is working properly)
4)  Close the file manager window.
5)  Open a terminal window (ctrl + alt + t) and type the "lsblk" command (!do not use the quotes!) and check which device the scope is on. (~8GB disk)
6)  Copy the files from the card to have a backup on your computer.
7)  Un-mount the partition. ("sudo umount /dev/sdc1" in my case)
8)  Just to be more safe make a backup with dd. ("sudo dd bs=4M if=/dev/sdc of=sd_card_backup.bin" again in my case)
9)  Open gparted and check if the device is properly formated. (Use right mouse and information to see the sector info)
10) If not delete the partition and make a new one leaving 1M free at the start. Format is fat32.
11) When the partition remounts after the previous step un-mount it again.
12) Use dd to place the firmware package on the SD card. ("sudo dd if=fnirsi_1013d.bin of=/dev/sdc bs=1024 seek=8")
13) This will re-mount the partition. Un-mount the partition again. ("sudo umount /dev/sdc1" in my case)
14) Turn of the scope and turn it back on. This will start the new scope firmware

Removing the new firmware is easy:
1) Perform the first steps of the install. (1,2,3,4,5,7)
2) Remove the program with "sudo dd if=/dev/zero of=/dev/sdc bs=1024 seek=8 count=1"

Further updates of the firmware don't require the partitioning, since that is already correctly setup for the first time of loading the new firmware.
So skip steps 6,8,9,10,11.

When using a SD card reader/writer directly coupled to your Linux machine don't forget to use the umount command. It is needed to have dd work properly.

For more information take a look here:
1) https://www.eevblog.com/forum/testgear/fnirsi-1013d-100mhz-tablet-oscilloscope/msg3807689/#msg3807689
2) https://www.eevblog.com/forum/testgear/fnirsi-1013d-100mhz-tablet-oscilloscope/msg3809966/#msg3809966
3) https://www.eevblog.com/forum/testgear/fnirsi-1013d-100mhz-tablet-oscilloscope/msg3908555/#msg3908555

For a view at the history and the flash file packer tool look here:
https://github.com/pecostm32/FNIRSI-1013D-Hack

The V0.005_Windows.7z file is from an external source and is not verified by me but EEVBlog members have used it.

---------------------------------------------------------------------------------------------------------------------
Januari 12 2023
Merged in a change made Michal Derkacz (ziutek) who improved on the RMS measurement. This brings the version op to
V0.006. There is no image file for it like the V0.005_Windows.7z file, so the binary https://github.com/pecostm32/FNIRSI_1013D_Firmware/tree/main/fnirsi_1013d_scope/dist/Debug/GNU_ARM-Linux/fnirsi_1013d.bin needs to be used.
