# d52cbaca0ef8cf4fd3d6354deb5066970fb6511d02d18d15835e6014ed847fb0
# (c) Copyright 2016-2018 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information
# of Xilinx, Inc. and is protected under U.S. and
# international copyright and other intellectual property
# laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any
# rights to the materials distributed herewith. Except as
# otherwise provided in a valid license issued to you by
# Xilinx, and to the maximum extent permitted by applicable
# law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
# WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
# AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
# BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
# INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
# (2) Xilinx shall not be liable (whether in contract or tort,
# including negligence, or under any other theory of
# liability) for any loss or damage of any kind or nature
# related to, arising under or in connection with these
# materials, including for any direct, or any indirect,
# special, incidental, or consequential loss or damage
# (including loss of data, profits, goodwill, or any type of
# loss or damage suffered as a result of any action brought
# by a third party) even if such damage or loss was
# reasonably foreseeable or Xilinx had been advised of the
# possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-
# safe, or for use in any application requiring fail-safe
# performance, such as life-support or safety devices or
# systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any
# other applications that could lead to death, personal
# injury, or severe property or environmental damage
# (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and
# liability of any use of Xilinx products in Critical
# Applications, subject only to applicable laws and
# regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
# PART OF THIS FILE AT ALL TIMES.

proc generate {drv_handle} {

  generate_instance_headers $drv_handle

  xdefine_include_file $drv_handle "xparameters.h" "XSdFec" \
      "NUM_INSTANCES" \
      "DEVICE_ID" \
      "C_S_AXI_BASEADDR" \
      "C_S_AXI_HIGHADDR" \
      "DRV_STANDARD" \
      "DRV_INITIALIZATION_PARAMS"

  xdefine_config_file $drv_handle "xsd_fec_g.c" "XSdFec" \
      "DEVICE_ID" \
      "C_S_AXI_BASEADDR" \
      "DRV_STANDARD" \
      "DRV_INITIALIZATION_PARAMS"

  xdefine_canonical_xpars $drv_handle "xparameters.h" "XSdFec" \
      "DEVICE_ID" \
      "C_S_AXI_BASEADDR" \
      "C_S_AXI_HIGHADDR" \
      "DRV_STANDARD" \
      "DRV_INITIALIZATION_PARAMS"

}

proc generate_instance_headers {drv_handle} {

  # Get all peripherals connected to this driver
  # o Lifted this from xillib_sw.tcl
  # o It looks like this gets called once per driver, which may have multiple instances(IP) connected
  set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
  foreach periph $periphs {
    if { [common::get_property CONFIG.Turbo_Decode $periph] } {
      generate_turbo_header $periph
    } else {
      generate_ldpc_header $periph
    }
  }
}

proc generate_turbo_header {ipinst} {
  set inst_name [common::get_property NAME $ipinst]
  set params    [common::get_property CONFIG.DRV_TURBO_PARAMS $ipinst]
  set header_fn "x${inst_name}_turbo_params.h"
  set fh [::hsi::utils::open_include_file $header_fn]
  puts $fh ""
  puts $fh "#ifndef X[string toupper $inst_name]_TURBO_PARAMS_H"
  puts $fh "#define X[string toupper $inst_name]_TURBO_PARAMS_H"
  puts $fh "#include \"xsdfec.h\""
  puts $fh ""
  if { $params ne "" && $params ne "undefined"} {
    puts $fh "const XSdFecTurboParameters X[string toupper $inst_name]_TURBO_PARAMS = { [dict get $params algorithm] , [dict get $params scale] };"
  } else {
    puts $fh "// ERROR: Turbo driver parameters not defined on IP instance !!"
  }
  puts $fh ""
  puts $fh "#endif"
  close $fh
}

proc generate_ldpc_header {ipinst} {
  set inst_name [common::get_property NAME $ipinst]
  set params    [common::get_property CONFIG.DRV_LDPC_PARAMS $ipinst]
  if { $params ne "" && $params ne "undefined"} {
    # Header per LDPC code
    foreach { code_id config } $params {
      # set header_fn "x${inst_name}_${code_id}_params.h"
      set id "x[string tolower $inst_name]_[string tolower $code_id]"
      set header_fn "${id}_params.h"
      set fh [::hsi::utils::open_include_file $header_fn]
      puts $fh ""
      puts $fh "#ifndef X[string toupper $inst_name]_[string toupper $code_id]_PARAMS_H"
      puts $fh "#define X[string toupper $inst_name]_[string toupper $code_id]_PARAMS_H"
      puts $fh "#include \"xsdfec.h\""
      puts $fh ""
      # Tables first
      set table [dict get $config sc_table]
      set hex_table [list]
      foreach val $table {
        lappend hex_table [format "0x%08x" $val]
      }
      puts $fh "const u32 ${id}_sc_table_size = [llength $hex_table];"
      puts $fh "u32 ${id}_sc_table\[[llength $hex_table]\] = {"
      puts $fh "  [join $hex_table ",\n  "]"
      puts $fh "};"
      set table [dict get $config la_table]
      set hex_table [list]
      foreach val $table {
        lappend hex_table [format "0x%08x" $val]
      }
      puts $fh "const u32 ${id}_la_table_size = [llength $hex_table];"
      puts $fh "u32 ${id}_la_table\[[llength $hex_table]\] = {"
      puts $fh "  [join $hex_table ",\n  "]"
      puts $fh "};"
      set table [dict get $config qc_table]
      set hex_table [list]
      foreach val $table {
        lappend hex_table [format "0x%08x" $val]
      }
      puts $fh "const u32 ${id}_qc_table_size = [llength $hex_table];"
      puts $fh "u32 ${id}_qc_table\[[llength $hex_table]\] = {"
      puts $fh "  [join $hex_table ",\n  "]"
      puts $fh "};"
      # Struct
      # o Must match driver struct definition order
      puts $fh "XSdFecLdpcParameters ${id}_params = {"
      puts $fh "  [format "0x%08x" [dict get $config n              ]], // N"
      puts $fh "  [format "0x%08x" [dict get $config k              ]], // K"
      puts $fh "  [format "0x%08x" [dict get $config p              ]], // P"
      puts $fh "  [format "0x%08x" [dict get $config nlayers        ]], // NLayers"
      puts $fh "  [format "0x%08x" [dict get $config nqc            ]], // NQC"
      puts $fh "  [format "0x%08x" [dict get $config nmqc           ]], // NMQC"
      puts $fh "  [format "0x%08x" [dict get $config nm             ]], // NM"
      puts $fh "  [format "0x%08x" [dict get $config norm_type      ]], // NormType"
      puts $fh "  [format "0x%08x" [dict get $config no_packing     ]], // NoPacking"
      puts $fh "  [format "0x%08x" [dict get $config special_qc     ]], // SpecialQC"
      puts $fh "  [format "0x%08x" [dict get $config no_final_parity]], // NoFinalParity"
      puts $fh "  [format "0x%08x" [dict get $config max_schedule   ]], // MaxSchedule"
      puts $fh "  &${id}_sc_table\[0\],"
      puts $fh "  &${id}_la_table\[0\],"
      puts $fh "  &${id}_qc_table\[0\]"
      puts $fh "};"
      puts $fh ""
      puts $fh "#endif"
      close $fh
    }
  }

}
