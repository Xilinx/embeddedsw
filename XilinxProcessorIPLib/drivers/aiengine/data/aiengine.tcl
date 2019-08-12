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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
###############################################################################

proc copy_files {dir} {
	foreach sub_dir [glob -nocomplain -directory $dir -type d *] {
		copy_files $sub_dir
	}
	foreach fn [glob -nocomplain [file join $dir *]] {
		file copy -force $fn "./src"
	}
}

proc copy_sub_files {dir} {
	foreach sub_dir [glob -nocomplain -directory $dir -type d *] {
		copy_files $sub_dir
	}
}

proc generate {libhandle} {
	copy_sub_files "./src"
	foreach dir [glob -nocomplain -directory "./src" -type d *] {
		file delete -force $dir
	}
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	puts $file_handle ""
	puts $file_handle "#include \"xaiengine/xparameters_aie.h\""
	puts $file_handle ""

	set config_file "src/xaieconfig.h"
	file delete -force $config_file
	set fid [open $config_file "w+"];
	puts $fid "#ifndef XCONFIG_H";
	puts $fid "#define XCONFIG_H";
    
	puts $fid "";
	puts $fid "#define __AIEBAREMTL__"    
	puts $fid "#endif";
	close $fid;

	# generate any parameters needed in xparameters.h or config file
}

