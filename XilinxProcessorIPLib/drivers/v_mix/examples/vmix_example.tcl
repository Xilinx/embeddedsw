## \file vmix_example.tcl
#Automates the process of generating the downloadable bit & elf files from the provided example hdf file.
## Documented procedure \c vmix_example .
# The code is inserted here:
#\code

proc vmix_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vmix_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk setws vmix_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk createhw -name vmix_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk createbsp -name vmix_example_bsp -hwproject vmix_example_hw_platform -proc processor_ss_processor -os standalone

		#create application project
		puts "Create Application Project"
		sdk createapp -name vmix_example_design -hwproject vmix_example_hw_platform -proc processor_ss_processor -os standalone -lang C -app {Empty Application} -bsp vmix_example_bsp

		#copy example source files to app project
		puts "Get Example Design Source Files"
		sdk importsources -name vmix_example_design -path ./src

		#build project
		puts "Build Project"
		sdk project -build -type all
	}
}

#\endcode
# endoffile
