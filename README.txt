embeddedsw.git - repo for standalone software

This is Beta Version for Versal

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

Every driver, sw_apps and sw_services has one or more of these sub-directories:
1. data		- contains tcl, mdd, testapp tcl or header files used in SDK
2. doc		- documentation of source code in form of pdf or html
3. examples	- illustrating different use cases of driver
4. src		- driver interface code implementing functionality of IP

