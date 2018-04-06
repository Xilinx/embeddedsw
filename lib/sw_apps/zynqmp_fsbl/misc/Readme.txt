Building FSBL from git:

ZynqMP_FSBL has 3 directories.
	1. data - It contains files for SDK
	2. src  - It contains the FSBL source files
	3. misc - It contains miscellaneous files required to
		  compile ZynqMP FSBL.
		  Builds for zcu102,zcu102-es2 board are
		  supported.

How to compile ZynqMP FSBL:

	1.Go to the Fsbl src directory "lib/sw_apps/zynqmp_fsbl/src/"
	2.If executables and other artifacts from previous FSBL build with other
	  configuration (different processor/state) are present, run
	  make clean to delete them.
	2.Give build command in the following manner.
		a. Create fsbl_bsp by giving ./copy_bsp.sh <board> <processor> <32/64>  in misc folder
		b. Build bsp by giving make <board> <processor> <32/64>
		c. Go to fsbl src directory.
		d. make "BOARD=<>" "PROC=<>" "A53_STATE=<>"
		e. Value for BOARD can be zcu102 or zcu102-es2. (Default is zcu102)
		f. Value for PROC can be a53 or r5. (Default is a53)
		g. Value for A53_STATE can be 64 or 32. (Default: 64)
			A53_STATE is only to be given when processor is a53.
	3.Give "make" to compile the fsbl src. By default it is
	  built for zcu102 board.
	4.Below are the examples for compiling for different options
		a. To generate A53 64 bit Fsbl for zcu102 board
			i.make "BOARD=zcu102" "PROC=a53" "A53_STATE=64"
		b. To generate R5 Fsbl for zcu102 board with debug enable
			i.make "BOARD=zcu102" "PROC=r5" "CFLAGS+=-DFSBL_DEBUG_INFO"
		c. To generate A53 32 bit Fsbl for zcu102 board.
			i.make "BOARD=zcu102" "PROC=a53" "A53_STATE=32"
		d. To generate A53 64 bit Fsbl for zcu102-es2 board
			i.make "BOARD=zcu102-es2" "PROC=a53" "A53_STATE=64"
		e. To generate R5 Fsbl for zcu102-es2 board with debug enable
			i.make "BOARD=zcu102-es2" "PROC=r5" "CFLAGS+=-DFSBL_DEBUG_INFO"
		f. To generate A53 32 bit Fsbl for zcu102-es2 board.
			i.make "BOARD=zcu102-es2" "PROC=a53" "A53_STATE=32"
