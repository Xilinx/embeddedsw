###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

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
