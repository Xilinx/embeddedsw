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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
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
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  pkp  12/03/15 Initial Release
##############################################################################

#---------------------------------------------
# openamp_drc
#---------------------------------------------
proc openamp_drc {libhandle} {
    # check processor type
    set proc_instance [hsi::get_sw_processor]
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]
    if { ( $proc_type != "psu_cortexr5" ) && ( $proc_type != "ps7_cortexa9" ) } {
                error "ERROR: This library is supported only for CortexR5 and CortexA9 processors."
                return
    }

    # make sure libmetal is available
    set librarylist_1 [hsi::get_libs -filter "NAME==libmetal"]

    if { [llength $librarylist_1] == 0 } {
        # do not error "OpenAMP library requires Libmetal library."
        # simply add the required library
        # The GUI/SDK seem to be unable to handle the dependency
        hsi::add_library libmetal
    } elseif { [llength $librarylist_1] > 1} {
        error "Multiple Libmetal libraries present."
    }

}

#-------
# generate: called after OS and library files are copied into project dir
# 	we need to generate the following:
#		1. Makefile options
#		2. System Arch settings for OpenAMP to use
#-------
proc generate {libhandle} {
	# Get the processor
	set proc_instance [hsi::get_sw_processor]
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os [common::get_property NAME [hsi::get_os]]
	set compiler_str [common::get_property CONFIG.compiler -object ${proc_instance}]
	set compiler_l [split ${compiler_str}]
	set compiler [lindex ${compiler_l} 0]
	set crosscompile [string map {gcc ""} "${compiler}"]
	set c_flags [common::get_property CONFIG.compiler_flags -object ${proc_instance}]
	set extra_flags [common::get_property CONFIG.extra_compiler_flags -object ${proc_instance}]
	set linclude [file normalize "../.."]
	set extra_flags_oamp "${extra_flags} -I${linclude}/include"
	set with_proxy [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == WITH_PROXY } ] ]

	puts "WITH_PROXY=${with_proxy}"

	if { "${with_proxy}" == "true" } {
		if {[string match "*-DUNDEFINE_FILE_OPS*" $extra_flags] != 1} {
			set extra_flags "$extra_flags -DUNDEFINE_FILE_OPS"
			common::set_property -name VALUE -value $extra_flags -objects  [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ]
			puts "updated extra flags=${extra_flags}"
		}
	} else {
		if {[string match "*-DUNDEFINE_FILE_OPS*" $extra_flags] == 1} {
			regsub -- {-DUNDEFINE_FILE_OPS} $extra_flags {} extra_flags
			common::set_property -name VALUE -value $extra_flags -objects  [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ]
			puts "updated extra flags=${extra_flags}"
		}
	}

	# Generate cmake toolchain file
	set toolchain_cmake "toolchain"
	set fd [open "src/open-amp/cmake/platforms/${toolchain_cmake}.cmake" w]

	if { "${proc_type}" == "psu_cortexr5" } {
		puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"arm\" CACHE STRING \"\")"
		puts $fd "set (MACHINE \"zynqmp_r5\")"
	} elseif { "${proc_type}" == "ps7_cortexa9" } {
		puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"arm\" CACHE STRING \"\")"
		puts $fd "set (MACHINE \"zynq7\")"
	}
	puts $fd "set (CROSS_PREFIX \"${crosscompile}\" CACHE STRING \"\")"
	puts $fd "set (CMAKE_C_FLAGS \"${c_flags} ${extra_flags_oamp}\" CACHE STRING \"\")"
	if { [string match "freertos*" "${os}"] > 0 } {
		puts $fd "set (CMAKE_SYSTEM_NAME \"FreeRTOS\" CACHE STRING \"\")"
	} else {
		puts $fd "set (CMAKE_SYSTEM_NAME \"Generic\" CACHE STRING \"\")"
	}
	puts $fd "include (CMakeForceCompiler)"
	puts $fd "CMAKE_FORCE_C_COMPILER (\"\$\{CROSS_PREFIX\}gcc\" GNU)"
	puts $fd "CMAKE_FORCE_CXX_COMPILER (\"\$\{CROSS_PREFIX\}g++\" GNU)"

	puts $fd "set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING \"\")"
	puts $fd "set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER CACHE STRING \"\")"
	puts $fd "set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER CACHE STRING \"\")"
	close $fd

	# Run cmake to generate make file
	set bdir "build_openamp"
	if { [catch {file mkdir "${bdir}"} msg] } {
		error "Failed to create OpenAMP build directory."
	}
	set workdir [pwd]
	cd "${bdir}"
	set cmake_cmd "../src/run_cmake"
	set os_platform_type "$::tcl_platform(platform)"

	if { [string match -nocase "windows*" "${os_platform_type}"] == 0 } {
		# Linux
		file attributes ${cmake_cmd} -permissions ugo+rx

		set cmake_opt "-DCMAKE_TOOLCHAIN_FILE=toolchain -DCMAKE_INSTALL_PREFIX=/ -DCMAKE_VERBOSE_MAKEFILE=on -DWITH_LIBMETAL_FIND=off -DWITH_EXT_INCLUDES_FIND=off -DWITH_PROXY=${with_proxy}"
		if { [catch {exec ${cmake_cmd} "../src/open-amp" ${cmake_opt}} msg] } {
			puts "${msg}"
			error "Failed to generate cmake files."
		} else {
			puts "${msg}"
		}

	} else {
		# Windows
		if { [catch {exec ${cmake_cmd} -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=toolchain -DCMAKE_INSTALL_PREFIX=/ -DCMAKE_VERBOSE_MAKEFILE=on -DWITH_LIBMETAL_FIND=off -DWITH_EXT_INCLUDES_FIND=off -DWITH_PROXY=${with_proxy} "../src/open-amp" } msg] } {
			puts "${msg}"
			error "Failed to generate cmake files."
		} else {
			puts "${msg}"
		}
	}

}

#-------
# post_generate: called after generate called on all libraries
#-------
proc post_generate {libhandle} {

}

#-------
# execs_generate: called after BSP's, libraries and drivers have been compiled
#	This procedure builds the libopen_amp.a library
#-------
proc execs_generate {libhandle} {

}
