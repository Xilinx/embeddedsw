##############################################################################
#
# (c) Copyright 2005-2014 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information of Xilinx, Inc.
# and is protected under U.S. and international copyright and other
# intellectual property laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any rights to the
# materials distributed herewith. Except as otherwise provided in a valid
# license issued to you by Xilinx, and to the maximum extent permitted by
# applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
# FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
# MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
# and (2) Xilinx shall not be liable (whether in contract or tort, including
# negligence, or under any other theory of liability) for any loss or damage
# of any kind or nature related to, arising under or in connection with these
# materials, including for any direct, or any indirect, special, incidental,
# or consequential loss or damage (including loss of data, profits, goodwill,
# or any type of loss or damage suffered as a result of any action brought by
# a third party) even if such damage or loss was reasonably foreseeable or
# Xilinx had been advised of the possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe performance, such as life-support or
# safety devices or systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any other applications
# that could lead to death, personal injury, or severe property or
# environmental damage (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and liability of any use of
# Xilinx products in Critical Applications, subject only to applicable laws
# and regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 4.0     adk    12/10/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_I
##
##  - include header files
##    
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_H
##
##  - Added support for generation of multiple applications.
##    All TCL procedures are now required to have a software
##    project type as its first argument
##  - Declared delay variable in WriteToGPOutput() function as volatile.
##    This causes the variable to be loaded from memory with each access
##    and will cause a sufficient delay when run on hardware.
##    
## @END_CHANGELOG
##
## @BEGIN_CHANGELOG EDK_H_SP1
##  - Generate GPIO testcode for PeripheralTest only instead of MemoryTest
##    because it is a peripheral, not a memory
## @END_CHANGELOG
##
## @BEGIN_CHANGELOG EDK_I
##
##  - Do not generate test function in TCL procedure, instead call driver example function.
##    
## @END_CHANGELOG

# Uses $XILINX_EDK/bin/lib/xillib_sw.tcl

# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines from common driver dir
#   1 : PeripheralTest - Calls any existing polled_example and/or selftest
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# Global variables
# Each global string defined here is a C test function definition.
# Each function defined here must have a unique function prototype.
# Ie. ALL functions defined in this file must be capable of 
#     co-existing in the same C file!

# -----------------------------------------------------------------
# TCL Procedures:
# -----------------------------------------------------------------

proc gen_include_files {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }
    if {$swproj == 1} {
        set gpio_intr [is_ip_interrupting_current_processor $mhsinst]
        set all_inputs [get_property CONFIG.C_ALL_INPUTS $mhsinst]
        
        if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
            set inc_file_lines {xgpio.h gpio_header.h gpio_intr_header.h}
        } else {
            set inc_file_lines {xgpio.h gpio_header.h}
        }
        
        
        
        return $inc_file_lines
    }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }
    if {$swproj == 1} {
        set gpio_intr [is_ip_interrupting_current_processor $mhsinst]
        set all_inputs [get_property CONFIG.C_ALL_INPUTS $mhsinst]
        
        if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
            set inc_file_lines {examples/xgpio_tapp_example.c examples/xgpio_intr_tapp_example.c data/gpio_header.h data/gpio_intr_header.h}
        } else {
            set inc_file_lines {examples/xgpio_tapp_example.c data/gpio_header.h}
        }
        return $inc_file_lines

    }
}

proc gen_testfunc_def {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }
    if {$swproj == 1} {
        set gpio_isdual [get_property CONFIG.C_IS_DUAL $mhsinst]
        set gpio_intr [is_ip_interrupting_current_processor $mhsinst]
        set all_inputs [get_property CONFIG.C_ALL_INPUTS $mhsinst]
        
        if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
            if { ${gpio_isdual} == 1 } {
                append decl "
#define GPIO_CHANNEL1 1
#define GPIO_CHANNEL2 2"	
            } else {
                append decl "
#define GPIO_CHANNEL1 1"
            }
            
            set inc_file_lines $decl
            return $inc_file_lines
        } else {
            return ""
        }
    }
}

proc gen_init_code {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }
    if {$swproj == 1} {
        set gpio_intr [is_ip_interrupting_current_processor $mhsinst]
        set all_inputs [get_property CONFIG.C_ALL_INPUTS $mhsinst]
        set ipname [get_property NAME $mhsinst]
        
        if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
            set decl "   static XGpio ${ipname}_Gpio;"
            set inc_file_lines $decl
            return $inc_file_lines
        } else {
            return ""
        }
    }
}

proc gen_testfunc_call {swproj mhsinst} {
    
    if {$swproj == 0} {
        return ""
    }
    
    set ipname [get_property NAME $mhsinst]
    set deviceid [xget_name $mhsinst "DEVICE_ID"]
    set gpio_width [get_property CONFIG.C_GPIO_WIDTH $mhsinst]
    set gpio_intr [is_ip_interrupting_current_processor $mhsinst]
    set gpio_isdual [get_property CONFIG.C_IS_DUAL $mhsinst]
    set all_inputs [get_property CONFIG.C_ALL_INPUTS $mhsinst]
    set stdout [get_property CONFIG.STDOUT [get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
    if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
       set intr_pin_name [get_pins -of_objects [get_cells $ipname]  -filter "TYPE==INTERRUPT"]
       set intcname [get_connected_interrupt_controller $ipname  $intr_pin_name]
       set intcvar intc
       set proc [get_property IP_NAME [get_cells [get_sw_processor]]]
    }
    set testfunc_call ""
    
    if {${hasStdout} == 0} {
        
        switch ${all_inputs} {
            0       { 
                append testfunc_call "

   {
      int status;
      
      status = GpioOutputExample(${deviceid},${gpio_width});
   }"
            }
            1       { 
                append testfunc_call "

   {
      int status;
	 
      u32 DataRead;
      
      status = GpioInputExample(${deviceid}, &DataRead);
   }"
            }
            default { return "" }
        }
        
        
        if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
	    if {
		 $proc == "microblaze"
	    } then {
		   set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	    } else {
		set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	    } 
            if { ${gpio_isdual} == 1 } {
                set intr_mask "GPIO_CHANNEL1 | GPIO_CHANNEL2"
            } else {
                set intr_mask "GPIO_CHANNEL1"
            }
            set intr_id [string toupper $intr_id]
            set intr_mask [string toupper $intr_mask]
            append testfunc_call "
    {
       int Status;
	
       u32 DataRead;
   
       Status = GpioIntrExample(&${intcvar}, &${ipname}_Gpio, \\
                                ${deviceid}, \\
                                ${intr_id}, \\
                                ${intr_mask}, &DataRead);
    }"  
        }
        
    }
    if {${hasStdout} == 1} {
        
        switch ${all_inputs} {
            0       { 
                append testfunc_call "

   {
      u32 status;
      
      print(\"\\r\\nRunning GpioOutputExample() for ${ipname}...\\r\\n\");

      status = GpioOutputExample(${deviceid},${gpio_width});
      
      if (status == 0) {
         print(\"GpioOutputExample PASSED.\\r\\n\");
      }
      else {
         print(\"GpioOutputExample FAILED.\\r\\n\");
      }
   }"
            }
            1       { 
                append testfunc_call "

   {
      u32 status;
      
      print(\"\\r\\nRunning GpioInputExample() for ${ipname}...\\r\\n\");

      u32 DataRead;
      
      status = GpioInputExample(${deviceid}, &DataRead);
      
      if (status == 0) {
         xil_printf(\"GpioInputExample PASSED. Read data:0x%X\\r\\n\", DataRead);
      }
      else {
         print(\"GpioInputExample FAILED.\\r\\n\");
      }
   }"
            }
            default { return "" }
        }
        
        
        if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
            if {
		 $proc == "microblaze"
	    } then {
		   set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	    } else {
		   set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	    } 
            if { ${gpio_isdual} == 1 } {
                set intr_mask "GPIO_CHANNEL1 | GPIO_CHANNEL2"
            } else {
                set intr_mask "GPIO_CHANNEL1"
            }
            set intr_id [string toupper $intr_id]
            set intr_mask [string toupper $intr_mask]
            append testfunc_call "
   {
      
      int Status;
        
      u32 DataRead;
      
      print(\" Press button to Generate Interrupt\\r\\n\");
      
      Status = GpioIntrExample(&${intcvar}, &${ipname}_Gpio, \\
                               ${deviceid}, \\
                               ${intr_id}, \\
                               ${intr_mask}, &DataRead);
	
      if (Status == 0 ){
             if(DataRead == 0)
                print(\"No button pressed. \\r\\n\");
             else
                print(\"Gpio Interrupt Test PASSED. \\r\\n\"); 
      } 
      else {
         print(\"Gpio Interrupt Test FAILED.\\r\\n\");
      }
	
   }"  
        }
    }
    
    return $testfunc_call
}

