Building Zynq FSBL from git:

Zynq FSBL has 3 directories.
	1. data - It contains files for SDK
	2. src  - It contains the FSBL source files
	3. misc - It contains miscellaneous files required to
		  compile FSBL.
		  Builds for zc702, zc706 and zed boards are supported.

How to compile Zynq FSBL:

	1.Go to the Fsbl src directory "lib/sw_apps/zynq_fsbl/src/"
	2. make "BOARD=<>" "CC=<>"
		a. Values for BOARD  are zc702, zc706, zed
		b. Value for CC is arm-none-eabi-gcc. Default value is also same.
	3.Give "make" to compile the fsbl with BSP. By default it is
	  built for zc702 board with arm-none-eabi-gcc compiler
	4.Below are the examples for compiling for different options
		a. To generate Fsbl for zc706 board
			i.make "BOARD=zc706"
		b.To generate Fsbl for zc702 board with debug enable
		  and RSA support
			i.make "BOARD=zc702" "CFLAGS=-DFSBL_DEBUG_INFO -DRSA_SUPPORT"
		c.To generate Fsbl for zc706 board and compile with arm-none-eabi-gcc
		  with MMC support
			i.make "BOARD=zc706" "CC=arm-none-eabi-gcc" "CFLAGS=-DMMC_SUPPORT"
