vpss_example.tcl automates the process of generating the downloadable bit & elf files from the provided vpss example hdf file.

Example application design source files (contained within "examples/src" folder) are tightly coupled with the video processing subsystem example design available in Vivado Catalogue.
To run the provided tcl
  1. Copy the exported example design hdf file in the "examples" directory of the driver
     (a) Default hdf name is "design_1_v_proc_ss_0_0_design_synth_wrapper.hdf". If generated hdf name is different then vpss_example.tcl needs to be updated with the new name
  2. Launch the xsct terminal
  3. cd into the examples directory
  4. run the tcl
       xsct%>source vpss_example.tcl

Script will perform following operations
  1. Create workspace
  2. Create HW project
  3. Create BSP
  4. Create Application Project
  5. Build BSP and Application Project

After the process is complete required files will be available in
  bit file -> vpss_example.sdk/vpss_example_hw_platform folder
  elf file -> vpss_example.sdk/vpss_example_design/{Debug/Release} folder

When executed on the board the example application will determine the video processing subsystem topology set the input and output stream configuration accordingly. Test pattern generator IP is used to generated the input stream. Video Lock Monitor IP will then monitor the output of the subsystem (to vidout) to determine if lock is achieved and present the status on the terminal

Note: Serial terminal baud rate should be set to 9600
