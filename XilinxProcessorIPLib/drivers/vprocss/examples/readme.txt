vpss_example.tcl automates the process of generating the downloadable bit & elf files from the provided vpss example hdf file.

Example application design source files (contained within "examples/src" folder) are tightly coupled with the video processing subsystem example design available in Vivado Catalogue.
To run the provided tcl
  1. Copy the exported example design hdf file in the "examples" directory of the driver
  2. Launch the xsct terminal
  3. cd into the examples directory
  4. source the tcl file
		xsct%>source vpss_example.tcl
  4. execute the script
		xsct%>vpss_example <hdf_file_name.hdf>

Script will perform following operations
  1. Create workspace
  2. Create HW project
  3. Create BSP
  4. Create Application Project
  5. Build BSP and Application Project

After the process is complete required files will be available in
  bit file -> vpss_example.sdk/vpss_example_hw_platform folder
  elf file -> vpss_example.sdk/vpss_example_design/{Debug/Release} folder

When executed on the board the example application will determine the video processing subsystem topology and set the input and output stream configuration accordingly. Test pattern generator IP is used to generate the input stream. Video Lock Monitor IP will then monitor the output of the subsystem (to vidout) to determine if lock is achieved and present the status (Pass/Fail) on the terminal

Note: Serial terminal baud rate should be set to 9600
