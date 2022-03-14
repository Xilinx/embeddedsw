Building Versal PSMFW from git:
===============================

PSMFW(versal_psmfw) has 3 directories.
	1. data - It contains files for Vitis
	2. src  - It contains the PSMFW source files
	3. misc - It contains miscellaneous files required to
		  compile PSMFW.

How to compile Versal PSMFW:
===============================
1. Go to the PSMFW src directory "lib/sw_apps/versal_psmfw/src/"
2. If executables and other artifacts from previous PSMFW build are present,
   run "make clean" to delete them.
3. Give "make" to compile the PSMFW with BSP.
4. This will create "psmfw.elf" in the PSMFW src directory.