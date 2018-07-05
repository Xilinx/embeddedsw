###############################################################################
#
# Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################

## \file vdemosaic_example.tcl
#Automates the process of generating the downloadable bit & elf files from the provided example hdf file.
## Documented procedure \c vdemosaic_example .
# The code is inserted here:
#\code

proc vdemosaic_example args {

	if {[llength $args] != 1} {
		puts "error: hdf file name missing from command line"
		puts "Please specify hdf to process"
		puts "Example Usage: vdemosaic_example.tcl design1.hdf"
	} else {

		set hdf [lindex $args 0]

		#set workspace
		puts "Create Workspace"
		sdk setws vdemosaic_example.sdk

		#create hw project
		puts "Create HW Project"
		sdk createhw -name vdemosaic_example_hw_platform -hwspec ./$hdf

		#create bsp
		puts "Create BSP"
		sdk createbsp -name vdemosaic_example_bsp -hwproject vdemosaic_example_hw_platform -proc processor_ss_processor -os standalone

		#create application project
		puts "Create Application Project"
		sdk createapp -name vdemosaic_example_design -hwproject vdemosaic_example_hw_platform -proc processor_ss_processor -os standalone -lang C -app {Empty Application} -bsp vdemosaic_example_bsp

		#copy example source files tp app project
		puts "Get Example Design Source Files"
		sdk importsources -name vdemosaic_example_design -path ./src

		#build project
		puts "Build Project"
		sdk project -build -type all
	}
}

#\endcode
# endoffile
