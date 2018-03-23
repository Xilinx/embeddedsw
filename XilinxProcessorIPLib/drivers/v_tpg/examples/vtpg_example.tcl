## \file vtpg_example.tcl
#Automates the process of generating the downloadable bit & elf files from the provided example hdf file.
## Documented procedure \c vtpg_example .
# The code is inserted here:
#\code

proc vtpg_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vtpg_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk setws vtpg_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk createhw -name vtpg_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk createbsp -name vtpg_example_bsp -hwproject vtpg_example_hw_platform -proc processor_ss_processor -os standalone

		#create application project
		puts "Create Application Project"
		sdk createapp -name vtpg_example_design -hwproject vtpg_example_hw_platform -proc processor_ss_processor -os standalone -lang C -app {Empty Application} -bsp vtpg_example_bsp

		#copy example source files tp app project
		puts "Get Example Design Source Files"
		file copy main.c ./vtpg_example.sdk/vtpg_example_design/src

		#build project
		puts "Build Project"
		sdk projects -build -type all
	}
}

#\endcode
# endoffile
