###############################################################################
## Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
## SPDX-License-Identifier: MIT
##
################################################################################
#
proc create_sub_dirs {dir tdir} {
	set dname [file tail ${dir}]
	file mkdir ${tdir}/${dname}
	foreach sub_dir [glob -nocomplain -directory $dir -type d *] {
		create_sub_dirs ${sub_dir} "${tdir}/${sub_dir}"
	}
}
proc generate {libhandle} {
	set tdir "../../include/xaiefal"
	file mkdir "${tdir}"
	foreach sub_dir [glob -nocomplain -directory "./src" -type d *] {
		create_sub_dirs ${sub_dir} ${tdir}
	}
}
