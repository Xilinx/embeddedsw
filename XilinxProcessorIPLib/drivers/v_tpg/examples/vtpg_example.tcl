proc vtpg_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vtpg_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk set_workspace vtpg_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk create_hw_project -name vtpg_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk create_bsp_project -name vtpg_example_bsp -hwproject vtpg_example_hw_platform -proc microblaze_0 -os standalone

		#create application project
		puts "Create Application Project"
		sdk create_app_project -name vtpg_example_design -hwproject vtpg_example_hw_platform -proc microblaze_0 -os standalone -lang C -app {Empty Application} -bsp vtpg_example_bsp

		#copy example source files tp app project
		puts "Get Example Design Source Files"
		file copy main.c ./vtpg_example.sdk/vtpg_example_design/src

		#build project
		puts "Build Project"
		sdk build_project -type all
	}
}
