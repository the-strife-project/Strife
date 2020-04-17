JBoot is the bootloader for jotaOS.
I'd rather not use grub, as it's quite big, and I want a tiny ISO.
The purpose was to make this REALLY simple.
JBoot consists in two stages.

Stage 1 basically loads the CD filesystem and runs the stage 2.
Stage 2 enables A20, goes into unreal mode, loads the kernel into memory,
prepares the information that will be necessary later, goes into protected
mode and gives control to the kernel.

Once the kernel has the control, it can get important data left by JBoot.
At 0x9000, there's a byte containing the boot drive ID.
At 0xA000, there's the list returned by INT 0x15, EAX = 0xE820.
The kernel will need this last one in order to figure out how much RAM is available.

There are actually two differnet stage 1 bootloaders:
- CD stage1, which boots from a CD (ISO9660).
- HDD stage1, which boots from the JOTAFS formatted drive.
