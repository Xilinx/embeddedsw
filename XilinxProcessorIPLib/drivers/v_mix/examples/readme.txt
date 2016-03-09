vmix_example.tcl automates the process of generating the downloadable bit & elf files from the provided example hdf file.

Example application design source files (contained within "examples" folder) are tightly coupled with the v_mix example design available in Vivado Catalogue.
To run the provided tcl
  1. Copy the exported example design hdf file in the "examples" directory of the driver
  2. Launch the xsct terminal
  3. cd into the examples directory
  4. source the tcl file
		xsct%>source vmix_example.tcl
  4. execute the script
		xsct%>vmix_example <hdf_file_name.hdf>

Script will perform following operations
  1. Create workspace
  2. Create HW project
  3. Create BSP
  4. Create Application Project
  5. Build BSP and Application Project

After the process is complete required files will be available in
  bit file -> vmix_example.sdk/vmix_example_hw_platform folder
  elf file -> vmix_example.sdk/vmix_example_design/{Debug/Release} folder

When executed on the board the example application will perform following operations
	1. Set Input video stream
	2. Program VTC for video stream parameters
	3. Program Mixer for video stream parameters
	4. Program TPG to generate required video stream
	5. Check for Video Lock
	6. If Locked, run defined tests on video mixer IP
	7. Repeat Steps 1-6 for defined video streams (1080p/60 and 4k2k/30)

Note: Serial terminal baud rate should be set to 9600
