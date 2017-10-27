/** \page example Examples
You can refer to the below stated example applications for more details on how to use vprocss driver.

@section ex1 main.c
Contains an example on how to use the XVprocss driver directly.
This example is the main file for the Video Processing Subsystem.

For details, see main.c.

@section ex2 periph.c
Contains an example on how to use the XVprocss driver directly.
This example is the top level resource file that will initialize all system level
peripherals.

For details, see periph.c.

@section ex3 periph.h
This is header for top level resource file that will initialize all system
level peripherals.

For details, see periph.h.

@section ex4 system.c
Contains an example on how to use the XVprocss driver directly.
This example is the top level resource file that will initialize all system level
peripherals.

For details, see system.c.

@section ex5 system.h
This is header for top level resource file that will initialize all system
level peripherals.

For details, see system.h.

@section ex6 vpss_example.tcl
Contains a tcl file which automates the process of generating the
downloadable bit & elf files from the provided example hdf file.

Example application design source files (contained within "examples/src" folder) are
tightly coupled with the video processing subsystem example design available in Vivado
Catalogue.
To run this tcl
1. Copy the exported example design hdf file in the "examples" directory of
     the driver
2. Launch the xsct terminal
3. cd into the examples directory
4. source the tcl file
   @code xsct%>source vpss_example.tcl @endcode
5. execute the script
   @code xsct%>vpss_example <hdf_file_name.hdf> @endcode

Script will perform following operations
1. Create workspace
2. Create HW project
3. Create BSP
4. Create Application Project
5. Build BSP and Application Project

After the process is complete required files will be available in
@verbatim
  bit file -> vpss_example.sdk/vpss_example_hw_platform folder
  elf file -> vpss_example.sdk/vpss_example_design/{Debug/Release} folder
@endverbatim

When executed on the board the example application will determine the video
processing subsystem topology and set the input and output stream
configuration accordingly. Test pattern generator IP is used to generate
the input stream. Video Lock Monitor IP will then monitor the output of
the subsystem (to vidout) to determine if lock is achieved and present the
status (Pass/Fail) on the terminal.

@note Serial terminal baud rate should be set to 115200

For details, see vpss_example.tcl.
*/


