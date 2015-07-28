###############################################################################
#
# Copyright (C) 2005 - 2014 Xilinx, Inc.  All rights reserved.
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
        set gpio_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set all_inputs [common::get_property CONFIG.C_ALL_INPUTS $mhsinst]
        
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
        set gpio_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set all_inputs [common::get_property CONFIG.C_ALL_INPUTS $mhsinst]
        
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
        set gpio_isdual [common::get_property CONFIG.C_IS_DUAL $mhsinst]
        set gpio_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set all_inputs [common::get_property CONFIG.C_ALL_INPUTS $mhsinst]
        
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
        set gpio_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set all_inputs [common::get_property CONFIG.C_ALL_INPUTS $mhsinst]
        set ipname [common::get_property NAME $mhsinst]
        
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
    
    set ipname [common::get_property NAME $mhsinst]
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set gpio_width [common::get_property CONFIG.C_GPIO_WIDTH $mhsinst]
    set gpio_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set gpio_isdual [common::get_property CONFIG.C_IS_DUAL $mhsinst]
    set all_inputs [common::get_property CONFIG.C_ALL_INPUTS $mhsinst]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
    if { ${gpio_intr} == 1 && ${all_inputs} == 1 } {
       set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
       set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
       set intcvar intc
       set proc [common::get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
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

