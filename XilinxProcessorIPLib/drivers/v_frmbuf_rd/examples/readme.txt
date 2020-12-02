vfrmbufrd_example.tcl automates the process of generating the downloadable bit & elf files from the provided example xsa file.

Example application design source files (contained within "examples" folder) are tightly coupled with the v_frmbuf_rd example design available in Vivado Catalog.
To run the provided tcl
  1. Copy the exported example design xsa file in the "examples" directory of the driver
  2. Launch the xsct terminal
  3. cd into the examples directory
  4. source the tcl file
		xsct%>source vfrmbufrd_example.tcl
  4. execute the script
		xsct%>vfrmbufrd_example <xsa_file_name.xsa>

Script will perform following operations
  1. Create workspace
  2. Create HW project
  3. Create BSP
  4. Create Application Project
  5. Build BSP and Application Project

After the process is complete required files will be available in
  bit file -> vfrmbufrd_example.sdk/vfrmbufrd_example_hw_platform folder
  elf file -> vfrmbufrd_example.sdk/vfrmbufrd_example_design/{Debug/Release} folder

When executed on the board the example application will perform following operations
	1. Program VTC for video stream parameters
	1. Program Frame Buffer
	3. Check for Video Lock
	4. Repeat Steps 1-3 for defined video streams (720p, 1080p, 4Kp30, 4Kp60)


Note: Serial terminal baud rate should be set to 115200
