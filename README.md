# In the name of god
# Build
```
# Specially thanks to https://github.com/eternalinsomnia/uefi_tetris 
# First of all using main.c and MakeFile
# Using vscode and install extension 
edk2-vscode , UEFI Firmware Extension Pack
```
# Launch
## QEMU
```
# apt install qemu ovmf gnu-efi
# first make sure where is bootable flash mounted
# format to fat32
# make a folder as a top named EFI
# in that folder mae another folder called BOOT
# in that you must have a file named BOOTX64.efi
# now based on last documantaion copy OVMF.fd in a folder
# in that folder build a test.img
# use command :
$ sudo qemu-system-x86_64 -L . --bios ~/Documents/OVMF.fd -hda test.img -hdb /dev/sdd1 -boot d -m 256
# be carefull /dev/sdd1 means where youre flash mouted
# finish :)
```
## Create a bootable usb drive
```
$ mkfs.vfat -F32 /dev/sdx
$ mkdir -p mnt
$ mount /dev/sdx mnt/
$ mkdir -p mnt/EFI/BOOT
$ cp tetris.efi mnt/EFI/BOOT/BOOTx64.EFI
$ umount mnt 
$ mount /dev/sdx mnt/
```
## Run sequence:
modify main c file
$ make

## Make an virtual drive : 
specify the patch: (for me thats ~/Documents)

$ cd ~/Documents

create img file
$ qemu-img create test.img 1G

mount that image
$ sudo mount -t auto -o loop test.img /mnt/test
$ sudo mount /dev/loop42 /mnt/test

if error occures install 
$ sudo apt-get install kpartx
$ sudo kpartx -av test.img

check your devices and see the name of you mounted point. (for me : /dev/loop42)
now forma fat32 :

$ sudo mkfs.vfat -F32 /dev/loop42

Making folder /mnt/test/EFI/BOOT if not exist

$ cd /mnt/test
$ sudo mkdir EFI
$ cd EFI
$ sudo mkdir BOOT

Now after run the "make" command you have a .efi file name BOOTX64.efi (you need that path example: ~/Documents/BOOTX64.efi)
copy that to path /mnt/test/EFI/BOOT/BOOTx64.efi

$ sudo cp ~/Documents/BOOTX64.efi /mnt/test/EFI/BOOT/BOOTx64.efi

unmount mount :)
$ cd ~/Documents
$ sudo umount /mnt/test
$ sudo mount /dev/loop42 /mnt/test

know specify where is the OVMF.fd and test.img
then run this command with your path:


$ sudo qemu-system-x86_64 -L . --bios ~/Documents/OVMF.fd -hda ~/Documents/test.img -hdb /dev/loop42 -boot d -m 256