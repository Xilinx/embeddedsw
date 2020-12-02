Building PMUFW from git:
============================

PMUFW(zynqmp_pmufw) has 3 directories.
	1. data - It contains files for Vitis
	2. src  - It contains the PMUFW source files
	3. misc - It contains miscellaneous files required to
		  compile PMUFW.


How to compile ZynqMP PMUFW:
============================
1. Go to the PMUFW src directory "lib/sw_apps/zynqmp_pmufw/src/"
2. If executables and other artifacts from previous PMUFW build are present,
   run make clean to delete them.
3. Give "make" to compile the PMUFW with BSP.
4. This will create "executable.elf" in the PMUFW src directory.
