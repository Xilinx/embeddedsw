###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
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

	file mkdir "../../include/xaiengine"

	# generate any parameters needed in xparameters.h or config file
}

