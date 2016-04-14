embeddedsw.git - repo for standalone software

All software is version less and divided into three directories
	- lib
		contains bsp, zynq fsbl and software services like xilisf
	- license.txt
		contains information about the various licenses and copyrights
	- XilinxProcessorIPLib
		contains all drivers
	- ThirdParty
		software from third party like light weight IP stack
	- mcap
		software for using MCAP interface on Ultra Scale boards to
		program 2nd level bitstream

Every driver/lib/apps/services has these sub-directories

1. data		- contains tcl, mdd, testapp tcl or header files used in SDK
2. doc		- documentation of source code in form of pdf or html
3. examples	- illustrating different use cases of driver
4. src		- driver interface code implementing functionality of IP


<repo>
|-XilinxProcessorIPLib
|	|- drivers
|		|- uartps
|			|- data
|			|- src
|			|- doc
|			|- examples
|
|-lib
|	|- bsp
|		|- standalone
|			|- data
|			|- src
|				|- cortexa9
|				|- cortexa53
|				|- cortexr5
|				|- microblaze
|				|- common
|				|- profile
|			|- doc
|		|- xilkernel
|			|- data
|			|- doc
|			|- src
|	|- sw_apps
|		|- zynq_fsbl [described below]
		|- zynqmp_fsbl [described below]
|	|- sw_services
|		|- xilffs
|		|- xilskey
|		|- xilmfs
|		|- xilrsa
|		|- xilflash
|		|- xilisf
|		|- xilsecure
|
|	Note - All these are libraries and utilize drivers
|
|-ThirdParty
|	|- sw_services
|        	|- lwip140
|
|-mcap
|	|-linux


Building FSBL from git:

FSBL(zynq_fsbl/zynqmp_fsbl) has 3 directories.
	1. data - It contains files for SDK
	2. src  - It contains the FSBL source files
	3. misc - It contains miscellaneous files required to
		  compile FSBL.
		  For zynq (zynq_fsbl), builds for zc702, zc706, zed and
		  microzed boards are supported.
		  For zynqmp (zynqmp_fsbl), builds for zcu102 board are
		  supported.
		  It also contains the ps7_init_gpl.[c/h] with gpl
		  header in respective board directories.


How to compile FSBL:
	Zynq
	1.Go to the Fsbl src directory "lib/sw_apps/zynq_fsbl/src/"
	2. make "BOARD=<>" "CC=<>"
		a. Values for BOARD  are zc702, zc706, zed, microzed
		b. Value for CC is arm-xilinx-eabi-gcc. Default value is also same.
	3.Give "make" to compile the fsbl with BSP. By default it is
	  built for zc702 board with arm-xilinx-eabi-gcc compiler
	4.Below are the examples for compiling for different options
		a. To generate Fsbl for zc706 board
			i.make "BOARD=zc706"
		b.To generate Fsbl for zc702 board with debug enable
		  and RSA support
			i.make "BOARD=zc702" "CFLAGS=-DFSBL_DEBUG_INFO -DRSA_SUPPORT"
		c.To generate Fsbl for zc706 board and compile with arm-xilinx-eabi-gcc
		  with MMC support
			i.make "BOARD=zc706" "CC=arm-xilinx-eabi-gcc" "CFLAGS=-DMMC_SUPPORT"

	ZynqMP
	1.Go to the Fsbl src directory "lib/sw_apps/zynqmp_fsbl/src/"
	2.If executables and other artifacts from previous FSBL build with other
	  configuration (different processor/state) are present, run
	  make clean to delete them.
	2.Give build command in the following manner.
		a. make "BOARD=<>" "PROC=<>" "A53_STATE=<>"
		a. Value for BOARD is zcu102. (Default is zcu102)
		b. Value for PROC can be a53 or r5. (Default is a53)
		c. Value for A53_STATE can be 64 or 32. (Default: 64)
			A53_STATE is only to be given when processor is a53.
	3.Give "make" to compile the fsbl with BSP. By default it is
	  built for zcu102 board.
	4.Below are the examples for compiling for different options
		a. To generate A53 64 bit Fsbl for zcu102 board
			i.make "BOARD=zcu102" "PROC=a53" "A53_STATE=64"
		b.To generate R5 Fsbl for zcu102 board with debug enable
			i.make "BOARD=zcu102" "PROC=r5" "CFLAGS+=-DFSBL_DEBUG_INFO"
		c.To generate A53 32 bit Fsbl for zcu102 board.
			i.make "BOARD=zcu102" "PROC=a53" "A53_STATE=32"
