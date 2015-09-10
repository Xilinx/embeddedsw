vtpg_example.tcl automates the process of generating the downloadable bit & elf files from the provided example hdf file.

Example application design source files (contained within "examples" folder) are tightly coupled with the v_tpg example design available in Vivado Catalogue.
To run the provided tcl
  1. Copy the exported example design hdf file in the "examples" directory of the driver
  2. Launch the xsct terminal
  3. cd into the examples directory
  4. source the tcl file
		xsct%>source vtpg_example.tcl
  4. execute the script
		xsct%>vtpg_example <hdf_file_name.hdf>

Script will perform following operations
  1. Create workspace
  2. Create HW project
  3. Create BSP
  4. Create Application Project
  5. Build BSP and Application Project

After the process is complete required files will be available in
  bit file -> vtpg_example.sdk/vtpg_example_hw_platform folder
  elf file -> vtpg_example.sdk/vtpg_example_design/{Debug/Release} folder

When executed on the board the example application will perform following operations
	1. Program Video Clock Generator to 1080p@60Hz
	2. Program TPG0 & TPG1 to 1080p@60Hz
	3. Check for Video Lock and report the status (PASS/FAIL)on UART
	4. Repeat Steps 1-3 for 4KP@30Hz and 4KP@60Hz

Note: Serial terminal baud rate should be set to 9600
