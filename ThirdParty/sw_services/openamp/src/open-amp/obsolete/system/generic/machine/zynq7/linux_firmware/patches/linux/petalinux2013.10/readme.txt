

GIC Patch
---------------

The patch file gic.patch updates the xlnx-3.8/arch/arm/common/gic.c file within the Petalinux kernel to support Linux remote kernel in unicore mode.

DevTree Patch
---------------

The patch file devtree.patch updates the xlnx-3.8/arch/arm/kernel/devtree.c file within the Petalinux kernel to support Linux remote kernel in unicore mode.

The above patches are applicable for the following linux version:

Distribution:  petalinux-v2013.10-final (Xilinx-ZC702-2013_3)

Linux kernel: 3.8.11


Device Tree File
---------------

The file system.dts is an example device tree source file used with PetaLinux unicore remote kernel build to demonstrate the Linux remote use case.
