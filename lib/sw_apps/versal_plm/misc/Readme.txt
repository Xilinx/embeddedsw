Building PLM from git:
===============================

PLM(versal_plm) has 3 directories.
	1. data - It contains files for Vitis
	2. src  - It contains the PLM source files
	    - common
	       - It contains common code between versal and versal_net
	    - versal
	       - It contains versal specific code
	    - versal_net
	       - It contains versal_net specific code
	3. misc - It contains miscellaneous files required to
		  compile PLM.

How to compile versal PLM:
===============================
1. Go to the PLM src directory "lib/sw_apps/versal_plm/src/versal"
2. If executables and other artifacts from previous PLM build are present,
   run "make clean" to delete them.
3. Give "make" to compile the PLM with BSP.
4. This will create "plm.elf" in the PLM src/versal directory.

How to compile versal_net PLM:
===============================
1. Go to the PLM src directory "lib/sw_apps/versal_plm/src/versal_net"
2. If executables and other artifacts from previous PLM build are present,
   run "make clean" to delete them.
3. Give "make" to compile the PLM with BSP.
4. This will create "plm.elf" in the PLM src/versal_net directory.
