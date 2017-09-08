## \file vgamma_lut_example.tcl
#Automates the process of generating the downloadable bit & elf files from the provided example hdf file.
## Documented procedure \c vgamma_lut_example .
# The code is inserted here:
#\code

proc vgamma_lut_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vgamma_lut_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk setws vgamma_lut_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk createhw -name vgamma_lut_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk createbsp -name vgamma_lut_example_bsp -hwproject vgamma_lut_example_hw_platform -proc microblaze_0 -os standalone

		#create application project
		puts "Create Application Project"
		sdk createapp -name vgamma_lut_example_design -hwproject vgamma_lut_example_hw_platform -proc microblaze_0 -os standalone -lang C -app {Empty Application} -bsp vgamma_lut_example_bsp

		#copy example source files tp app project
		puts "Get Example Design Source Files"
		sdk importsources -name vgamma_lut_example_design -path ./src

		#build project
		puts "Build Project"
		sdk project -build -type all
	}
}

#\endcode
# endoffile
