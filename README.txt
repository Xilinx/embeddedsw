embeddedsw.git - repo for standalone software

The standalone software is divided into following directories:
	- lib
		contains bsp, software apps and software services
	- license.txt
		contains information about the various licenses and copyrights
	- doc/ChangeLog
		Contains change log information for releases
	- XilinxProcessorIPLib/drivers
		contains all drivers
	- ThirdParty
		software from third party like light weight IP stack
	- mcap/linux
		software for using MCAP interface on Ultra Scale boards to
		program 2nd level bitstream

Every driver, sw_apps and sw_services has one or more of these sub-directories:
1. data		- contains tcl, mdd, testapp tcl or header files used in Vitis
2. doc		- documentation of source code in form of pdf or html 
3. examples	- illustrating different use cases of driver
4. src		- driver interface code implementing functionality of IP

<repo>
|-LICENSES
|
|-ThirdParty
|	|- bsp
|		|- freertos10_xilinx
|			|- data
			|- examples
|			|- src
|				|- License
|				|- Source
|	|- sw_services
|		|- libmetal
|		|- lwip141
|		|- lwip202
|		|- openamp
|
|-XilinxProcessorIPLib
|	|- drivers
|		|- avbuf
|		|- ...
|		|- ...
|		|- zdma
|
|-doc
|-lib
|	|- bsp
|		|- standalone
|			|- data
|			|- doc
|			|- src
|				|- arm
|					|- common
|					|- ARMv8
|					|- cortexa9
|					|- cortexr5
|           	|- common
|				|- microblaze
|				|- profile
|	|- sw_apps
|		|- ddr_self_refresh
|		|- ....
|		|- ....
|		|- ....
|		|- ....
|		|- zynqmp_fsbl [described below]
|		|- zynqmp_pmufw [described below]
|		|- versal_plm [described below]
|		|- versal_psmfw [described below]
|	|- sw_services
|		|- xilffs
|		|- xilflash
|		|- xilfpga
|		|- xilisf
|		|- xilmfs
|		|- xilpm
|		|- xilrsa
|		|- xilsecure
|		|- xilskey
|
|	Note - All these are libraries and utilize drivers
|
|-mcap
|	|-linux


Building FSBL from git:
==============================
FSBL(zynq_fsbl/zynqmp_fsbl) has 3 directories.
	1. data - It contains files for Vitis
	2. src  - It contains the FSBL source files
	3. misc - It contains miscellaneous files required to
		  compile FSBL.
		  For zynq (zynq_fsbl), builds for zc702, zc706, zed are supported.
		  It also contains the ps7_init_gpl.[c/h] with gpl
		  header in respective board directories.
		  For zynqmp (zynqmp_fsbl), builds for zcu102,zcu102-es2 board are
		  supported.
		  

How to compile FSBL:
	Zynq:
	Please refer to the steps in Readme.txt which is at lib/sw_apps/zynq_fsbl/misc/ directory

	ZynqMP
	Please refer to the steps in Readme.txt which is at lib/sw_apps/zynqmp_fsbl/misc/ directory

Building PMUFW from git:
==============================
Please refer to the steps in Readme.txt which is at lib/sw_apps/zynqmp_pmufw/misc/ directory

Building Versal PLM from git:
==============================
Please refer to the steps in Readme.txt which is at lib/sw_apps/versal_plm/misc/ directory

Building Versal PSMFW from git:
==============================
Please refer to the steps in Readme.txt which is at lib/sw_apps/versal_psmfw/misc/ directory
