## In the name of God

### Using Clion for Development Uefi Application
Please follow the steps explained in **CreateProjectClion** folder.


### Runnig EFI Project By hand 
If you have any problem following the steps explaind in **CreateProjectClion** please follow the steps explained in 
**CreateQemuByFIle** and copy the efi application built, on to the usb connected

### Runnig Qmeu with usb connected example:
In this part you can emulate the usb connect to qemu In addition to the Readme.md explained on above directories.

```bash
	our dummy drive called test2.img:
$ qemu-img create test2.img 1G
	run with dummy usb connected :
$ sudo qemu-system-x86_64 -L . --bios ~/Documents/OVMF.fd -hda ~/Documents/test.img -hdb /dev/loop45 -boot d -m 256 -device piix3-usb-uhci \
    -drive id=my_usb_disk,file=test2.img,if=none \
    -device usb-storage,drive=my_usb_disk

```
