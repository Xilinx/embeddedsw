###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# 
#
############################################################################### 

proc mfs_drc {lib_handle} {
    puts "MFS DRC ..."
}

proc mfs_open_include_file {file_name} {
    set filename [file join "../../include/" $file_name]
    if {[file exists $filename]} {
	set config_inc [open $filename a]
    } else {
	set config_inc [open $filename a]
	::hsi::utils::write_c_header $config_inc "MFS Parameters"
    }
    return $config_inc
}

proc generate {lib_handle} {

    puts "WARNING: XilMFS library is being deprecated from 2019.1. In the subsequent releases, it will be made obsolete. Please select XilFFS."
    puts "MFS generate ..."
    file copy "src/xilmfs.h"  "../../include/xilmfs.h" 

    set conffile  [mfs_open_include_file "mfs_config.h"]

    puts $conffile "#ifndef _MFS_CONFIG_H" 
    puts $conffile "#define _MFS_CONFIG_H" 
    set need_utils [common::get_property CONFIG.need_utils $lib_handle]
    if {$need_utils} { 
        # tell libgen or xps that the hardware platform needs to provide stdio functions 
        # inbyte and outbyte to support utils 
	puts $conffile "#include <stdio.h>" 
    } 
    puts $conffile "#include <xilmfs.h>"
    set value  [common::get_property CONFIG.numbytes $lib_handle]
    puts  $conffile "#define MFS_NUMBYTES  $value"
    set value  [common::get_property CONFIG.base_address $lib_handle]
    puts  $conffile "#define MFS_BASE_ADDRESS $value"
    set value  [common::get_property CONFIG.init_type $lib_handle]
    puts  $conffile "#define MFS_INIT_TYPE  $value"
    puts $conffile "#endif" 
    close $conffile 
}
