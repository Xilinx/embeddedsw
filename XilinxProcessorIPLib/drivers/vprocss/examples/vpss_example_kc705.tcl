## \file vpss_example.tcl for kc705
#Automates the process of generating the downloadable bit & elf files from the provided example xsa file.
## Documented procedure \c vpss_example .
# The code is inserted here:
#\code

proc vpss_example args {

	if {[llength $args] != 1} {
		puts "error: xsa file name missing from command line"
		puts "Please specify xsa to process"
		puts "Example Usage: vpss_example.tcl design1.xsa"
	} else {

		set xsa [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		setws vpss_example.sdk
		set a [getws]
		set app "Empty Application"

		app create -name vpss_example_design -template ${app} -hw ./$xsa -os standalone -proc processor_ss_processor

		#copy example source files tp app project
		puts "Get Example Design Source Files"
		importsources -name vpss_example_design -path ./src

		#build project
		puts "Build Project"
		app build -name vpss_example_design
	}
}

#\endcode
# endoffile
