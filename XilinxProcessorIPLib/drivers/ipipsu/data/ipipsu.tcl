###############################################################################
#
# Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00 mjr  02/03/15  Created
# 2.3  ms   04/11/17  Modified tcl file to add U suffix for all macros
#                     of ipipsu in xparameters.h
##############################################################################

#uses "xillib.tcl"

proc ipi_format_hexmask {bitpos} {
	return [format "0x%08X" [expr 1<<$bitpos]]
}
proc ipi_define_xpar {inst param} {
	set uSuffix "U"
	set param_name [string range $param [string length "CONFIG."] [string length $param]]
	set name [string range $param_name 2 end]
	set param_value [common::get_property $param [hsi::get_cells -hier $inst]]
	if { [string compare $name "BIT_POSITION"] == 0} {
		set name "BIT_MASK"
		set param_value [ipi_format_hexmask $param_value]
	}
	return  [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $inst] $name $param_value]
}

#Generate Config file with data structures describing the HW
proc ipi_generate_config {drv_handle file_name} {

	#Driver Prefix String
	set drv_string "XIpiPsu"

	#The current processor
	set sw_proc_handle [::hsi::get_sw_processor]
	set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]

	# List of IPIs owned by this processor
	set proc_ipi_list [lsearch -all -inline [get_property SLAVES $hw_proc_handle] psu_ipi_*]

	# List of all IPIs on SoC
	set ipi_list [get_cells -hier -filter { IP_NAME == "psu_ipi" }]

	set cfgfilename [file join "src" $file_name]
	set config_file [open $cfgfilename w]

	# Common Header
	::hsi::utils::write_c_header $config_file "Driver configuration"
	puts $config_file "#include \"xparameters.h\""
	puts $config_file "#include \"[string tolower $drv_string].h\""

	# Start generating the  Config table
	puts $config_file "\n/*"
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"
	set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
	puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
	puts $config_file "\{"

	set comma ""
	foreach ipi_inst $proc_ipi_list {
		puts $config_file $comma
		puts $config_file "\t\{"
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "DEVICE_ID,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BASE_ADDRESS,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BIT_MASK,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BUFFER_INDEX,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "INT_ID,"]

		# Target Count
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $drv_string] "NUM_TARGETS,"]

		# Start generating Target Table with list if all IPI slots on SoC
		puts $config_file "\t\t\{"

		set comma ""
		foreach ipi_inst $ipi_list {
			puts $config_file $comma
			puts $config_file "\t\t\t\{"
			puts $config_file [format "\t\t\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BIT_MASK,"]
			puts $config_file [format "\t\t\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BUFFER_INDEX"]
			puts -nonewline $config_file "\t\t\t\}"
			set comma ","
		}
		puts $config_file "\n\t\t\}"
		puts -nonewline $config_file "\t\}"
		set comma ",\n"
	}
	puts $config_file "\n\};"

	close $config_file
}
proc ipi_generate_params {file_name} {
	#Driver Prefix String
	set drv_string "XIpiPsu"
	set uSuffix "U"

	# open the xparameters.h file
	set file_handle [::hsi::utils::open_include_file $file_name]

	# List of all IPIs on SoC
	set ipi_list [get_cells -hier -filter { IP_NAME == "psu_ipi" }]

	#List of all processors on SoC
	set proc_list [get_cells -hier -filter { IP_TYPE == "PROCESSOR" }]

	#The current processor
	set sw_proc_handle [::hsi::get_sw_processor]
	set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]


	# List of IPIs owned by this processor
	set proc_ipi_list [lsearch -all -inline [get_property SLAVES $hw_proc_handle] psu_ipi_*]

	#Total number of IPIs assigned to this proc
	puts $file_handle [format "#define  XPAR_XIPIPSU_NUM_INSTANCES  %s$uSuffix" [llength $proc_ipi_list]]
	puts $file_handle ""

	# Generate all params for IPIs owned by this proc
	#Idx is used to track DEVICE_ID as we loop through the list
	set idx 0

	foreach ipi_inst $proc_ipi_list {
		puts $file_handle [format "/* Parameter definitions for peripheral %s */" $ipi_inst]
		puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $ipi_inst] "DEVICE_ID" $idx]
		puts $file_handle [ipi_define_xpar $ipi_inst CONFIG.C_BASE_ADDRESS]
		puts $file_handle [ipi_define_xpar $ipi_inst CONFIG.C_BIT_POSITION]
		puts $file_handle [ipi_define_xpar $ipi_inst CONFIG.C_BUFFER_INDEX]
		puts $file_handle [ipi_define_xpar $ipi_inst CONFIG.C_INT_ID]
		puts $file_handle ""
		incr idx
	}

	set idx 0
	foreach ipi_inst $proc_ipi_list {
		puts $file_handle [format "/* Canonical definitions for peripheral %s */" $ipi_inst]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "DEVICE_ID" [string toupper $ipi_inst] "DEVICE_ID"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "BASE_ADDRESS" [string toupper $ipi_inst] "BASE_ADDRESS"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "BIT_MASK" [string toupper $ipi_inst] "BIT_MASK"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "BUFFER_INDEX" [string toupper $ipi_inst] "BUFFER_INDEX"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "INT_ID" [string toupper $ipi_inst] "INT_ID"]
		puts $file_handle ""
		incr idx
	}

	#Total number of IPIs assigned to this proc
	puts $file_handle [format "#define  XPAR_XIPIPSU_NUM_TARGETS  %s$uSuffix" [llength $ipi_list]]
	puts $file_handle ""

	foreach ipi_inst $ipi_list {
		puts $file_handle [ipi_define_xpar $ipi_inst CONFIG.C_BIT_POSITION]
		puts $file_handle [ipi_define_xpar $ipi_inst CONFIG.C_BUFFER_INDEX]
		puts ""
	}
	# Generate Canonical definitions to map IPI instance -> Processors
	puts $file_handle "/* Target List for referring to processor IPI Targets */"
	puts $file_handle ""
	foreach proc $proc_list {
		# List of IPIs owned by this processor
		set proc_slave_list [lsearch -all -inline [get_property SLAVES $proc] psu_ipi_*]

		set idx 0
		foreach ipi_slave $proc_slave_list {
			puts $file_handle [format "#define  XPAR_XIPIPS_TARGET_%s_CH%s_MASK  XPAR_%s_BIT_MASK" [string toupper $proc] $idx [string toupper $ipi_slave]]
			puts $file_handle [format "#define  XPAR_XIPIPS_TARGET_%s_CH%s_INDEX  %s$uSuffix" [string toupper $proc] $idx [lsearch $ipi_list $ipi_slave]]
			puts ""
			incr idx
		}
		puts $file_handle ""
	}

	# close the include file
	close $file_handle

}


#This is called by HSI while generating the driver
proc generate {drv_handle} {
	ipi_generate_params "xparameters.h"
	ipi_generate_config $drv_handle "xipipsu_g.c"
}
