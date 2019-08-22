# This tcl file creates BSP and Application and targetted for kc705 board.


proc vfrmbufwr_example args {

	if {[llength $args] != 1} {
		puts "error: xsa file name missing from command line"
		puts "Please specify xsa to process"
		puts "Example Usage: vfrmbufwr_example.tcl design1.xsa"
	} else {

		set xsa [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		setws vfrmbufwr_example.sdk
		set a [getws]
		set app "Empty Application"

		app create -name vfrmbufwr_example_design -template ${app} -hw ./$xsa -os standalone -proc processor_ss_processor

		#copy example source files to app project
		puts "Get Example Design Source Files"
		importsources -name vfrmbufwr_example_design -path ./src

		#build project
		puts "Build Project"
		app build -name vfrmbufwr_example_design
	}
}
