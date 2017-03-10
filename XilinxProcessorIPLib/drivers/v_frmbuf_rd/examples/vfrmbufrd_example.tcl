proc vfrmbufrd_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vfrmbufrd_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk setws vfrmbufrd_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk createhw -name vfrmbufrd_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk createbsp -name vfrmbufrd_example_bsp -hwproject vfrmbufrd_example_hw_platform -proc microblaze_ss_microblaze_0 -os standalone

		#create application project
		puts "Create Application Project"
		sdk createapp -name vfrmbufrd_example_design -hwproject vfrmbufrd_example_hw_platform -proc microblaze_ss_microblaze_0 -os standalone -lang C -app {Empty Application} -bsp vfrmbufrd_example_bsp

		#copy example source files to app project
		puts "Get Example Design Source Files"
		sdk importsources -name vfrmbufrd_example_design -path ./src

		#build project
		puts "Build Project"
		sdk project -build -type all
	}
}
