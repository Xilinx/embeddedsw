Building PLM from git:
===============================

PLM(versalnet_plm) has 3 directories.
	1. data - It contains files for Vitis
	2. src  - It contains the PLM source files for versal_net
	3. misc - It contains miscellaneous files required to
		  compile PLM for versal_net

How to compile versal_net PLM:
===============================
1. Go to the PLM src directory "lib/sw_apps/versalnet_plm/src"
2. If executables and other artifacts from previous PLM build are present,
   run "make clean" to delete them.
3. Give "make" to compile the PLM with BSP.
4. This will create "plm.elf" in the PLM src directory.
