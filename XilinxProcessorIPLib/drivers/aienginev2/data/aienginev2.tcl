###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

proc copy_files {dir} {
	foreach sub_dir [glob -nocomplain -directory $dir -type d *] {
		copy_files $sub_dir
	}
	foreach fn [glob -nocomplain [file join $dir * -type f *]] {
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
	file mkdir "../../include/xaiengine"
}
