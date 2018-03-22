proc vfrmbufwr_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vfrmbufwr_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk setws vfrmbufwr_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk createhw -name vfrmbufwr_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk createbsp -name vfrmbufwr_example_bsp -hwproject vfrmbufwr_example_hw_platform -proc processor_ss_processor -os standalone

		#create application project
		puts "Create Application Project"
		sdk createapp -name vfrmbufwr_example_design -hwproject vfrmbufwr_example_hw_platform -proc processor_ss_processor -os standalone -lang C -app {Empty Application} -bsp vfrmbufwr_example_bsp

		#copy example source files to app project
		puts "Get Example Design Source Files"
		sdk importsources -name vfrmbufwr_example_design -path ./src

		#build project
		puts "Build Project"
		sdk project -build -type all
	}
}
