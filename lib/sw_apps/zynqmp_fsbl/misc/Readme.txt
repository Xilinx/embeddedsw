Building FSBL from git:

ZynqMP_FSBL has 3 directories.
	1. data - It contains files for SDK
	2. src  - It contains the FSBL source files
	3. misc - It contains miscellaneous files required to
		  compile ZynqMP FSBL.
		  Builds for zcu102 board is  supported.

How to compile ZynqMP FSBL:

	1.Go to the Fsbl src directory "lib/sw_apps/zynqmp_fsbl/src/"
	2.If executables and other artifacts from previous FSBL build with other
	  configuration (different processor/state) are present, run
	  make clean to delete them.
	2.Give build command in the following manner.
		a. Go to fsbl src directory.
		b. make "BOARD=<>" "PROC=<>" "A53_STATE=<>"
		c. Value for BOARD is zcu102. (Default is zcu102)
		d. Value for PROC can be a53 or r5. (Default is a53)
		e. Value for A53_STATE can be 64 or 32. (Default: 64)
			A53_STATE is only to be given when processor is a53.
		f. Make command will trigger the build of fsbl_bsp first and then build
		   fsbl.
	3.Give "make" to compile the fsbl src. By default it is
	  built for zcu102 board.
	4.Below are the examples for compiling for different options
		a. To generate A53 64 bit Fsbl for zcu102 board
			i.make "BOARD=zcu102" "PROC=a53" "A53_STATE=64"
		b. To generate R5 Fsbl for zcu102 board with debug enable
			i.make "BOARD=zcu102" "PROC=r5" "CFLAGS+=-DFSBL_DEBUG_INFO"
		c. To generate A53 32 bit Fsbl for zcu102 board.
			i.make "BOARD=zcu102" "PROC=a53" "A53_STATE=32"
	5. FSBL build is now supported with armclang compiler. Armclang build is only
	   supported for A53-64 FSBL for ZCU102 boards. CROSS_COMP is the command line
	   variable introduced for the purpose. Default value of CROSSS_COMP is gcc.
	   Example for armclang build:
		make clean all CROSS_COMP=armclang
