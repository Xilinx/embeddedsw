#
#
#       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
#       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
#       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
#       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
#       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
#       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
#       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
#       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
#       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
#       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
#       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
#       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#       FOR A PARTICULAR PURPOSE.
#
#       (c) Copyright 2002 Xilinx Inc.
#       All rights reserved.

# $Id: xilmfs_v2_1_0.tcl,v 1.3.16.6 2005/11/15 23:41:09 salindac Exp $ 
# 
# 

proc mfs_drc {lib_handle} {
    puts "MFS DRC ..."
}

proc mfs_open_include_file {file_name} {
    set filename [file join "../../include/" $file_name]
    if {[file exists $filename]} {
	set config_inc [open $filename a]
    } else {
	set config_inc [open $filename a]
	xprint_generated_header $config_inc "MFS Parameters"
    }
    return $config_inc
}

proc generate {lib_handle} {

    puts "MFS generate ..."
    file copy "src/xilmfs.h"  "../../include/xilmfs.h" 

    set conffile  [mfs_open_include_file "mfs_config.h"]

    puts $conffile "#ifndef _MFS_CONFIG_H" 
    puts $conffile "#define _MFS_CONFIG_H" 
    set need_utils [get_property CONFIG.need_utils $lib_handle] 
    if {$need_utils} { 
        # tell libgen or xps that the hardware platform needs to provide stdio functions 
        # inbyte and outbyte to support utils 
	puts $conffile "#include <stdio.h>" 
    } 
    puts $conffile "#include <xilmfs.h>"
    set value  [get_property CONFIG.numbytes $lib_handle]
    puts  $conffile "#define MFS_NUMBYTES  $value"
    set value  [get_property CONFIG.base_address $lib_handle]
    puts  $conffile "#define MFS_BASE_ADDRESS $value"
    set value  [get_property CONFIG.init_type $lib_handle]
    puts  $conffile "#define MFS_INIT_TYPE  $value"
    puts $conffile "#endif" 
    close $conffile 
}
