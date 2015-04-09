###############################################################
#  (c) Copyright 2008-2014 Xilinx, Inc. All rights reserved.
#
#  This file contains confidential and proprietary information
#  of Xilinx, Inc. and is protected under U.S. and
#  international copyright and other intellectual property
#  laws.
#
#  DISCLAIMER
#  This disclaimer is not a license and does not grant any
#  rights to the materials distributed herewith. Except as
#  otherwise provided in a valid license issued to you by
#  Xilinx, and to the maximum extent permitted by applicable
#  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
#  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
#  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
#  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
#  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
#  (2) Xilinx shall not be liable (whether in contract or tort,
#  including negligence, or under any other theory of
#  liability) for any loss or damage of any kind or nature
#  related to, arising under or in connection with these
#  materials, including for any direct, or any indirect,
#  special, incidental, or consequential loss or damage
#  (including loss of data, profits, goodwill, or any type of
#  loss or damage suffered as a result of any action brought
#  by a third party) even if such damage or loss was
#  reasonably foreseeable or Xilinx had been advised of the
#  possibility of the same.
#
#  CRITICAL APPLICATIONS
#  Xilinx products are not designed or intended to be fail-
#  safe, or for use in any application requiring fail-safe
#  performance, such as life-support or safety devices or
#  systems, Class III medical devices, nuclear facilities,
#  applications related to the deployment of airbags, or any
#  other applications that could lead to death, personal
#  injury, or severe property or environmental damage
#  (individually and collectively, "Critical
#  Applications"). Customer assumes the sole risk and
#  liability of any use of Xilinx products in Critical
#  Applications, subject only to applicable laws and
#  regulations governing limitations on product liability.
#
#  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
#  PART OF THIS FILE AT ALL TIMES.
#  MODIFICATION HISTORY:
#  Ver      Who    Date     Changes
#  -------- ------ -------- ------------------------------------
#  3.0      adk    12/10/13 Updated as per the New Tcl API's
###############################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XOSD" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_NUM_LAYERS" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_LAYER0_TYPE" "C_LAYER1_TYPE" "C_LAYER2_TYPE" "C_LAYER3_TYPE" "C_LAYER4_TYPE" "C_LAYER5_TYPE" "C_LAYER6_TYPE" "C_LAYER7_TYPE" "C_LAYER0_IMEM_SIZE" "C_LAYER0_INS_BOX_EN" "C_LAYER0_INS_LINE_EN" "C_LAYER0_INS_TEXT_EN" "C_LAYER0_CLUT_SIZE" "C_LAYER0_CLUT_MEMTYPE" "C_LAYER0_FONT_NUM_CHARS" "C_LAYER0_FONT_WIDTH" "C_LAYER0_FONT_HEIGHT" "C_LAYER0_FONT_BPP" "C_LAYER0_FONT_ASCII_OFFSET" "C_LAYER0_TEXT_NUM_STRINGS" "C_LAYER0_TEXT_MAX_STRING_LENGTH" "C_LAYER1_IMEM_SIZE" "C_LAYER1_INS_BOX_EN" "C_LAYER1_INS_LINE_EN" "C_LAYER1_INS_TEXT_EN" "C_LAYER1_CLUT_SIZE" "C_LAYER1_CLUT_MEMTYPE" "C_LAYER1_FONT_NUM_CHARS" "C_LAYER1_FONT_WIDTH" "C_LAYER1_FONT_HEIGHT" "C_LAYER1_FONT_BPP" "C_LAYER1_FONT_ASCII_OFFSET" "C_LAYER1_TEXT_NUM_STRINGS" "C_LAYER1_TEXT_MAX_STRING_LENGTH" "C_LAYER2_IMEM_SIZE" "C_LAYER2_INS_BOX_EN" "C_LAYER2_INS_LINE_EN" "C_LAYER2_INS_TEXT_EN" "C_LAYER2_CLUT_SIZE" "C_LAYER2_CLUT_MEMTYPE" "C_LAYER2_FONT_NUM_CHARS" "C_LAYER2_FONT_WIDTH" "C_LAYER2_FONT_HEIGHT" "C_LAYER2_FONT_BPP" "C_LAYER2_FONT_ASCII_OFFSET" "C_LAYER2_TEXT_NUM_STRINGS" "C_LAYER2_TEXT_MAX_STRING_LENGTH" "C_LAYER3_IMEM_SIZE" "C_LAYER3_INS_BOX_EN" "C_LAYER3_INS_LINE_EN" "C_LAYER3_INS_TEXT_EN" "C_LAYER3_CLUT_SIZE" "C_LAYER3_CLUT_MEMTYPE" "C_LAYER3_FONT_NUM_CHARS" "C_LAYER3_FONT_WIDTH" "C_LAYER3_FONT_HEIGHT" "C_LAYER3_FONT_BPP" "C_LAYER3_FONT_ASCII_OFFSET" "C_LAYER3_TEXT_NUM_STRINGS" "C_LAYER3_TEXT_MAX_STRING_LENGTH" "C_LAYER4_IMEM_SIZE" "C_LAYER4_INS_BOX_EN" "C_LAYER4_INS_LINE_EN" "C_LAYER4_INS_TEXT_EN" "C_LAYER4_CLUT_SIZE" "C_LAYER4_CLUT_MEMTYPE" "C_LAYER4_FONT_NUM_CHARS" "C_LAYER4_FONT_WIDTH" "C_LAYER4_FONT_HEIGHT" "C_LAYER4_FONT_BPP" "C_LAYER4_FONT_ASCII_OFFSET" "C_LAYER4_TEXT_NUM_STRINGS" "C_LAYER4_TEXT_MAX_STRING_LENGTH" "C_LAYER5_IMEM_SIZE" "C_LAYER5_INS_BOX_EN" "C_LAYER5_INS_LINE_EN" "C_LAYER5_INS_TEXT_EN" "C_LAYER5_CLUT_SIZE" "C_LAYER5_CLUT_MEMTYPE" "C_LAYER5_FONT_NUM_CHARS" "C_LAYER5_FONT_WIDTH" "C_LAYER5_FONT_HEIGHT" "C_LAYER5_FONT_BPP" "C_LAYER5_FONT_ASCII_OFFSET" "C_LAYER5_TEXT_NUM_STRINGS" "C_LAYER5_TEXT_MAX_STRING_LENGTH" "C_LAYER6_IMEM_SIZE" "C_LAYER6_INS_BOX_EN" "C_LAYER6_INS_LINE_EN" "C_LAYER6_INS_TEXT_EN" "C_LAYER6_CLUT_SIZE" "C_LAYER6_CLUT_MEMTYPE" "C_LAYER6_FONT_NUM_CHARS" "C_LAYER6_FONT_WIDTH" "C_LAYER6_FONT_HEIGHT" "C_LAYER6_FONT_BPP" "C_LAYER6_FONT_ASCII_OFFSET" "C_LAYER6_TEXT_NUM_STRINGS" "C_LAYER6_TEXT_MAX_STRING_LENGTH" "C_LAYER7_IMEM_SIZE" "C_LAYER7_INS_BOX_EN" "C_LAYER7_INS_LINE_EN" "C_LAYER7_INS_TEXT_EN" "C_LAYER7_CLUT_SIZE" "C_LAYER7_CLUT_MEMTYPE" "C_LAYER7_FONT_NUM_CHARS" "C_LAYER7_FONT_WIDTH" "C_LAYER7_FONT_HEIGHT" "C_LAYER7_FONT_BPP" "C_LAYER7_FONT_ASCII_OFFSET" "C_LAYER7_TEXT_NUM_STRINGS" "C_LAYER7_TEXT_MAX_STRING_LENGTH"
    xdefine_config_file $drv_handle "xosd_g.c" "XOSD" "DEVICE_ID" "C_BASEADDR" "C_NUM_LAYERS" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_LAYER0_TYPE" "C_LAYER1_TYPE" "C_LAYER2_TYPE" "C_LAYER3_TYPE" "C_LAYER4_TYPE" "C_LAYER5_TYPE" "C_LAYER6_TYPE" "C_LAYER7_TYPE" "C_LAYER0_IMEM_SIZE" "C_LAYER0_INS_BOX_EN" "C_LAYER0_INS_LINE_EN" "C_LAYER0_INS_TEXT_EN" "C_LAYER0_CLUT_SIZE" "C_LAYER0_CLUT_MEMTYPE" "C_LAYER0_FONT_NUM_CHARS" "C_LAYER0_FONT_WIDTH" "C_LAYER0_FONT_HEIGHT" "C_LAYER0_FONT_BPP" "C_LAYER0_FONT_ASCII_OFFSET" "C_LAYER0_TEXT_NUM_STRINGS" "C_LAYER0_TEXT_MAX_STRING_LENGTH" "C_LAYER1_IMEM_SIZE" "C_LAYER1_INS_BOX_EN" "C_LAYER1_INS_LINE_EN" "C_LAYER1_INS_TEXT_EN" "C_LAYER1_CLUT_SIZE" "C_LAYER1_CLUT_MEMTYPE" "C_LAYER1_FONT_NUM_CHARS" "C_LAYER1_FONT_WIDTH" "C_LAYER1_FONT_HEIGHT" "C_LAYER1_FONT_BPP" "C_LAYER1_FONT_ASCII_OFFSET" "C_LAYER1_TEXT_NUM_STRINGS" "C_LAYER1_TEXT_MAX_STRING_LENGTH" "C_LAYER2_IMEM_SIZE" "C_LAYER2_INS_BOX_EN" "C_LAYER2_INS_LINE_EN" "C_LAYER2_INS_TEXT_EN" "C_LAYER2_CLUT_SIZE" "C_LAYER2_CLUT_MEMTYPE" "C_LAYER2_FONT_NUM_CHARS" "C_LAYER2_FONT_WIDTH" "C_LAYER2_FONT_HEIGHT" "C_LAYER2_FONT_BPP" "C_LAYER2_FONT_ASCII_OFFSET" "C_LAYER2_TEXT_NUM_STRINGS" "C_LAYER2_TEXT_MAX_STRING_LENGTH" "C_LAYER3_IMEM_SIZE" "C_LAYER3_INS_BOX_EN" "C_LAYER3_INS_LINE_EN" "C_LAYER3_INS_TEXT_EN" "C_LAYER3_CLUT_SIZE" "C_LAYER3_CLUT_MEMTYPE" "C_LAYER3_FONT_NUM_CHARS" "C_LAYER3_FONT_WIDTH" "C_LAYER3_FONT_HEIGHT" "C_LAYER3_FONT_BPP" "C_LAYER3_FONT_ASCII_OFFSET" "C_LAYER3_TEXT_NUM_STRINGS" "C_LAYER3_TEXT_MAX_STRING_LENGTH" "C_LAYER4_IMEM_SIZE" "C_LAYER4_INS_BOX_EN" "C_LAYER4_INS_LINE_EN" "C_LAYER4_INS_TEXT_EN" "C_LAYER4_CLUT_SIZE" "C_LAYER4_CLUT_MEMTYPE" "C_LAYER4_FONT_NUM_CHARS" "C_LAYER4_FONT_WIDTH" "C_LAYER4_FONT_HEIGHT" "C_LAYER4_FONT_BPP" "C_LAYER4_FONT_ASCII_OFFSET" "C_LAYER4_TEXT_NUM_STRINGS" "C_LAYER4_TEXT_MAX_STRING_LENGTH" "C_LAYER5_IMEM_SIZE" "C_LAYER5_INS_BOX_EN" "C_LAYER5_INS_LINE_EN" "C_LAYER5_INS_TEXT_EN" "C_LAYER5_CLUT_SIZE" "C_LAYER5_CLUT_MEMTYPE" "C_LAYER5_FONT_NUM_CHARS" "C_LAYER5_FONT_WIDTH" "C_LAYER5_FONT_HEIGHT" "C_LAYER5_FONT_BPP" "C_LAYER5_FONT_ASCII_OFFSET" "C_LAYER5_TEXT_NUM_STRINGS" "C_LAYER5_TEXT_MAX_STRING_LENGTH" "C_LAYER6_IMEM_SIZE" "C_LAYER6_INS_BOX_EN" "C_LAYER6_INS_LINE_EN" "C_LAYER6_INS_TEXT_EN" "C_LAYER6_CLUT_SIZE" "C_LAYER6_CLUT_MEMTYPE" "C_LAYER6_FONT_NUM_CHARS" "C_LAYER6_FONT_WIDTH" "C_LAYER6_FONT_HEIGHT" "C_LAYER6_FONT_BPP" "C_LAYER6_FONT_ASCII_OFFSET" "C_LAYER6_TEXT_NUM_STRINGS" "C_LAYER6_TEXT_MAX_STRING_LENGTH" "C_LAYER7_IMEM_SIZE" "C_LAYER7_INS_BOX_EN" "C_LAYER7_INS_LINE_EN" "C_LAYER7_INS_TEXT_EN" "C_LAYER7_CLUT_SIZE" "C_LAYER7_CLUT_MEMTYPE" "C_LAYER7_FONT_NUM_CHARS" "C_LAYER7_FONT_WIDTH" "C_LAYER7_FONT_HEIGHT" "C_LAYER7_FONT_BPP" "C_LAYER7_FONT_ASCII_OFFSET" "C_LAYER7_TEXT_NUM_STRINGS" "C_LAYER7_TEXT_MAX_STRING_LENGTH"
    xdefine_canonical_xpars $drv_handle "xparameters.h" "OSD" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_NUM_LAYERS" "C_LAYER0_TYPE" "C_LAYER1_TYPE" "C_LAYER2_TYPE" "C_LAYER3_TYPE" "C_LAYER4_TYPE" "C_LAYER5_TYPE" "C_LAYER6_TYPE" "C_LAYER7_TYPE" "C_LAYER0_IMEM_SIZE" "C_LAYER0_INS_BOX_EN" "C_LAYER0_INS_LINE_EN" "C_LAYER0_INS_TEXT_EN" "C_LAYER0_CLUT_SIZE" "C_LAYER0_CLUT_MEMTYPE" "C_LAYER0_FONT_NUM_CHARS" "C_LAYER0_FONT_WIDTH" "C_LAYER0_FONT_HEIGHT" "C_LAYER0_FONT_BPP" "C_LAYER0_FONT_ASCII_OFFSET" "C_LAYER0_TEXT_NUM_STRINGS" "C_LAYER0_TEXT_MAX_STRING_LENGTH" "C_LAYER1_IMEM_SIZE" "C_LAYER1_INS_BOX_EN" "C_LAYER1_INS_LINE_EN" "C_LAYER1_INS_TEXT_EN" "C_LAYER1_CLUT_SIZE" "C_LAYER1_CLUT_MEMTYPE" "C_LAYER1_FONT_NUM_CHARS" "C_LAYER1_FONT_WIDTH" "C_LAYER1_FONT_HEIGHT" "C_LAYER1_FONT_BPP" "C_LAYER1_FONT_ASCII_OFFSET" "C_LAYER1_TEXT_NUM_STRINGS" "C_LAYER1_TEXT_MAX_STRING_LENGTH" "C_LAYER2_IMEM_SIZE" "C_LAYER2_INS_BOX_EN" "C_LAYER2_INS_LINE_EN" "C_LAYER2_INS_TEXT_EN" "C_LAYER2_CLUT_SIZE" "C_LAYER2_CLUT_MEMTYPE" "C_LAYER2_FONT_NUM_CHARS" "C_LAYER2_FONT_WIDTH" "C_LAYER2_FONT_HEIGHT" "C_LAYER2_FONT_BPP" "C_LAYER2_FONT_ASCII_OFFSET" "C_LAYER2_TEXT_NUM_STRINGS" "C_LAYER2_TEXT_MAX_STRING_LENGTH" "C_LAYER3_IMEM_SIZE" "C_LAYER3_INS_BOX_EN" "C_LAYER3_INS_LINE_EN" "C_LAYER3_INS_TEXT_EN" "C_LAYER3_CLUT_SIZE" "C_LAYER3_CLUT_MEMTYPE" "C_LAYER3_FONT_NUM_CHARS" "C_LAYER3_FONT_WIDTH" "C_LAYER3_FONT_HEIGHT" "C_LAYER3_FONT_BPP" "C_LAYER3_FONT_ASCII_OFFSET" "C_LAYER3_TEXT_NUM_STRINGS" "C_LAYER3_TEXT_MAX_STRING_LENGTH" "C_LAYER4_IMEM_SIZE" "C_LAYER4_INS_BOX_EN" "C_LAYER4_INS_LINE_EN" "C_LAYER4_INS_TEXT_EN" "C_LAYER4_CLUT_SIZE" "C_LAYER4_CLUT_MEMTYPE" "C_LAYER4_FONT_NUM_CHARS" "C_LAYER4_FONT_WIDTH" "C_LAYER4_FONT_HEIGHT" "C_LAYER4_FONT_BPP" "C_LAYER4_FONT_ASCII_OFFSET" "C_LAYER4_TEXT_NUM_STRINGS" "C_LAYER4_TEXT_MAX_STRING_LENGTH" "C_LAYER5_IMEM_SIZE" "C_LAYER5_INS_BOX_EN" "C_LAYER5_INS_LINE_EN" "C_LAYER5_INS_TEXT_EN" "C_LAYER5_CLUT_SIZE" "C_LAYER5_CLUT_MEMTYPE" "C_LAYER5_FONT_NUM_CHARS" "C_LAYER5_FONT_WIDTH" "C_LAYER5_FONT_HEIGHT" "C_LAYER5_FONT_BPP" "C_LAYER5_FONT_ASCII_OFFSET" "C_LAYER5_TEXT_NUM_STRINGS" "C_LAYER5_TEXT_MAX_STRING_LENGTH" "C_LAYER6_IMEM_SIZE" "C_LAYER6_INS_BOX_EN" "C_LAYER6_INS_LINE_EN" "C_LAYER6_INS_TEXT_EN" "C_LAYER6_CLUT_SIZE" "C_LAYER6_CLUT_MEMTYPE" "C_LAYER6_FONT_NUM_CHARS" "C_LAYER6_FONT_WIDTH" "C_LAYER6_FONT_HEIGHT" "C_LAYER6_FONT_BPP" "C_LAYER6_FONT_ASCII_OFFSET" "C_LAYER6_TEXT_NUM_STRINGS" "C_LAYER6_TEXT_MAX_STRING_LENGTH" "C_LAYER7_IMEM_SIZE" "C_LAYER7_INS_BOX_EN" "C_LAYER7_INS_LINE_EN" "C_LAYER7_INS_TEXT_EN" "C_LAYER7_CLUT_SIZE" "C_LAYER7_CLUT_MEMTYPE" "C_LAYER7_FONT_NUM_CHARS" "C_LAYER7_FONT_WIDTH" "C_LAYER7_FONT_HEIGHT" "C_LAYER7_FONT_BPP" "C_LAYER7_FONT_ASCII_OFFSET" "C_LAYER7_TEXT_NUM_STRINGS" "C_LAYER7_TEXT_MAX_STRING_LENGTH"
}


#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [get_property NAME $periph]]
        lappend peripherals $peripheral_name
    }

    # Get possible canonical names for all the peripherals connected to this driver
    set device_id 0
    foreach periph $periphs {
        set canonical_name [string toupper [format "%s_%s" $drv_string $device_id]]
        lappend canonicals $canonical_name

        # Create a list of IDs of the peripherals whose hardware instance name
        # doesn't match the canonical name. These IDs can be used later to
        # generate canonical definitions
        if { [lsearch $peripherals $canonical_name] < 0 } {
            lappend indices $device_id
        }
        incr device_id
    }

    set i 0
    foreach periph $periphs {
        set periph_name [string toupper [get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

    # The commented out rvalue is the name of the instance-specific constant
    #           set rvalue [::hsi::utils::get_ip_param_name $periph $arg]

                # The rvalue set below is the actual value of the parameter
                set rvalue [get_property CONFIG.$arg $periph]
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"

            }
            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
