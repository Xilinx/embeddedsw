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
|                               |- arm
|                                     |- common
|				      |- cortexa9
|				      |- cortexa53
|				      |- cortexr5
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
		|- zynqmp_pmufw [described below]
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
==============================
FSBL(zynq_fsbl/zynqmp_fsbl) has 3 directories.
	1. data - It contains files for SDK
	2. src  - It contains the FSBL source files
	3. misc - It contains miscellaneous files required to
		  compile FSBL.
		  For zynq (zynq_fsbl), builds for zc702, zc706, zed and
		  microzed boards are supported.
		  For zynqmp (zynqmp_fsbl), builds for zcu102,zcu102-es2 board are
		  supported.
		  It also contains the ps7_init_gpl.[c/h] with gpl
		  header in respective board directories.


How to compile FSBL:
	Zynq:
	Please refer to the steps in Readme.txt which is at lib/sw_apps/zynq_fsbl/misc/ directory

	ZynqMP
	Please refer to the steps in Readme.txt which is at lib/sw_apps/zynqmp_fsbl/misc/ directory

Building PMUFW from git:
==============================
Please refer to the steps in Readme.txt which is at lib/sw_apps/zynqmp_pmufw/misc/ directory
