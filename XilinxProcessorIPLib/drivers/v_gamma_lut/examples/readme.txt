/** \page example Examples
You can refer to the below stated example applications for more details on how to use v_gamma_lut driver.

@section ex1 main.c
Contains an example on how to use the XV_gamma_lut driver directly.
This example demonstrates the usage of Gamma LUT IP.

For details, see main.c.

@section ex2 vgamma_lut_example.tcl
Contains a tcl file which automates the process of generating the
downloadable bit & elf files from the provided example hdf file.

Example application design source files (contained within "examples" folder)
are tightly coupled with the v_gamma_lut example design available in Vivado Catalogue.
To run this tcl
1. Copy the exported example design hdf file in the "examples" directory of
     the driver
2. Launch the xsct terminal
3. cd into the examples directory
4. source the tcl file
   @code xsct%>source vgamma_lut_example.tcl @endcode
5. execute the script
   @code xsct%>vgamma_lut_example <hdf_file_name.hdf> @endcode

Script will perform following operations
1. Create workspace
2. Create HW project
3. Create BSP
4. Create Application Project
5. Build BSP and Application Project

After the process is complete required files will be available in
@verbatim
  bit file -> vgamma_lut_example.sdk/vgamma_lut_example_hw_platform folder
  elf file -> vgamma_lut_example.sdk/vgamma_lut_example_design/{Debug/Release} folder
@endverbatim

When executed on the board the example application will perform following operations
1. Program Video Clock Generator to 1080p@60Hz
2. Program Gamma LUT to 1080p@60Hz
3. Check for Video Lock and report the status (PASS/FAIL)on UART
4. Repeat Steps 1-3 for 4KP@30Hz and 4KP@60Hz

@note Serial terminal baud rate should be set to 115200

For details, see vgamma_lut_example.tcl.
*/