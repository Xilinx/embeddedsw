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


How to compile versal_2ve_2vm PLM:
================================
1. Go to "lib/sw_apps/versal_plm/src/versal_2ve_2vm".
2. Run "make clean" if artifacts from a previous build are present.
3. Run "make" to build plm.elf with the checked-in BSP and XilPM subsystem
   runtime support, matching the default runtime mode of the SDT flow.

XILPM_RUNTIME_LIB selects the runtime mode:
  SUBSYS (default) - EEMI and subsystem support.
  EEMI or 1       - EEMI support without full subsystem support.
  0               - Boot-only support for bring-up configurations.

Use SUBSYS for reference images requiring subsystem support, including ASUFW.
Run "make clean" before changing the runtime mode, for example:
  make clean
  make XILPM_RUNTIME_LIB=0

This Makefile uses a checked-in hardware configuration. For custom hardware,
build the BSP and PLM from the matching System Device Tree using the SDT flow.
