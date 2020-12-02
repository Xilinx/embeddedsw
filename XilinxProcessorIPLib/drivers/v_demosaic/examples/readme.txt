/** \page example Examples
You can refer to the below stated example applications for more details on how to use v_demosaic driver.

@section ex1 main.c
Contains an example on how to use the XV_demosaic driver directly.
This example demonstrates the usage of Demosaic IP.

For details, see main.c.

@section ex2 vdemosaic_example.tcl
Contains a tcl file which automates the process of generating the
downloadable bit & elf files from the provided example xsa file.

Example application design source files (contained within "examples" folder)
are tightly coupled with the v_demosaic example design available in Vivado Catalogue.
To run this tcl
1. Copy the exported example design xsa file in the "examples" directory of
     the driver
2. Launch the xsct terminal
3. cd into the examples directory
4. source the tcl file
   @code xsct%>source vdemosaic_example.tcl @endcode
5. execute the script
   @code xsct%>vdemosaic_example <xsa_file_name.xsa> @endcode

Script will perform following operations
1. Create workspace
2. Create HW project
3. Create BSP
4. Create Application Project
5. Build BSP and Application Project

After the process is complete required files will be available in
@verbatim
  bit file -> vdemosaic_example.sdk/vdemosaic_example_hw_platform folder
  elf file -> vdemosaic_example.sdk/vdemosaic_example_design/{Debug/Release} folder
@endverbatim

When executed on the board the example application will perform following operations
1. Program Video Clock Generator to 1080p@60Hz
2. Program Demosaic to 1080p@60Hz
3. Check for Video Lock and report the status (PASS/FAIL)on UART
4. Repeat Steps 1-3 for 4KP@30Hz and 4KP@60Hz

@note Serial terminal baud rate should be set to 115200

For details, see vdemosaic_example.tcl.
*/
