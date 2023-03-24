#/******************************************************************************
#* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Peripheral Tests";
}
 
proc swapp_get_description {} {
    return "Simple test routines for all peripherals in the hardware.";
}

proc swapp_is_supported_sw {} {
    set proc_inst [hsi::get_sw_processor];
    if { $proc_inst == "" } {
        error "Processor Instance $proc_inst doesn't exist in Xml/HwDb"
        return 0;
    }
    set vecMemRanges [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier -filter {IP_TYPE == "PROCESSOR"} $proc_inst -hierarchical] -filter {MEM_TYPE=="MEMORY" && IS_DATA!=0 || IS_INSTRUCTION!=0} ]
    set typical_testapp_size 8192;

    foreach vec $vecMemRanges {
        set base_value [common::get_property BASE_VALUE $vec]
        set high_value [common::get_property HIGH_VALUE $vec]
        set size [expr $high_value-$base_value+1]
        if { [expr $size > $typical_testapp_size] } {
             return 1;
        }
    }
    error "Peripheral tests require atleast $typical_testapp_size bytes of memory."
    return 0;
}

proc swapp_is_supported_hw {} {
    return 1;
}

proc generate_app_sources {} {
    set proc_instance [hsi::get_sw_processor]

    # from the list of drivers, collect the required test 
    # application code for each hw_instance of each driver into std::string vectors
    set driver_list [hsi::get_drivers [lindex [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier -filter {IP_TYPE=="PROCESSOR"} $proc_instance -hierarchical]]]]
    
    # removing duplicates 
    variable u_driver_list [list]
    for {set i 0} {$i < [llength $driver_list]} {incr i} {
        for {set j 0} {$j < [llength $u_driver_list]} {incr j} {
            if {[lindex $driver_list $i] == [lindex $u_driver_list $j]} {
                break
            }        
        }
        if { $j == [llength $u_driver_list]} {
            lappend u_driver_list [lindex $driver_list $i]        
        }
    } 
    
    set u_driver_list [lsort $u_driver_list]
    
    set t_instnames [common::get_property HW_INSTANCE $u_driver_list]
    set t_peripheral_names [common::get_property NAME $u_driver_list]
    set t_peripheral_versions [common::get_property VERSION $u_driver_list]
    
    variable peri_dirs[list]
    variable instnames_objs[list]
    variable include_files [list]
    variable src_files [list]
    variable src_files_base [list]
    variable func_defs [list]
    variable func_calls [list]
    variable func_init_codes [list]

    for {set i 0} {$i < [llength $t_peripheral_names]} {incr i} {
        set sw_core [hsi::get_sw_cores [lindex $t_peripheral_names $i]]
        if { $sw_core == "" } {
            set t_periver [string map {. _} [lindex $t_peripheral_versions $i]]
            set peri_dir [format {%s_v%s} [lindex $t_peripheral_names $i] $t_periver]
            set peri_dir_path [common::get_property DATA_FILE [hsi::get_sw_cores $peri_dir]]
            set peri_dir_path [string map {.mdd _tapp.tcl} $peri_dir_path]
            set t_src_files_base [common::get_property REPOSITORY [hsi::get_sw_cores $peri_dir]]        
        } else {
            set peri_dir [lindex $t_peripheral_names $i]
            set peri_dir_path [common::get_property DATA_FILE [hsi::get_sw_cores [lindex $t_peripheral_names $i]]]
            set peri_dir_path [string map {.mdd _tapp.tcl} $peri_dir_path]
            set t_src_files_base [common::get_property REPOSITORY [hsi::get_sw_cores [lindex $t_peripheral_names $i]]] 
        }

        set fexist [file exist $peri_dir_path]
        if { $fexist == 1 } {
            source $peri_dir_path  
            
            set t_instnames_obj [hsi::get_cells [lindex $t_instnames $i] -hierarchical]
            set t_include_files [gen_include_files 1 $t_instnames_obj]
            set t_src_files [gen_src_files 1 $t_instnames_obj]
            set t_func_defs [gen_testfunc_def 1 $t_instnames_obj]
            set t_func_calls [gen_testfunc_call 1 $t_instnames_obj]
            set t_func_init_codes [gen_init_code 1 $t_instnames_obj]         
            
            lappend peri_dirs $peri_dir                        
            lappend instnames_objs $t_instnames_obj
            lappend include_files $t_include_files
            lappend src_files $t_src_files
            lappend src_files_base $t_src_files_base
            lappend func_defs $t_func_defs
            lappend func_calls $t_func_calls
            lappend func_init_codes $t_func_init_codes
        }
    }    
    
    # copy files to the output directory
    set od [pwd]
    copy_files $src_files_base $src_files $od
    
    # if we have a interrupt controller, then move things around so that it is listed first 
    # note that we don't have to swap if it is at index 0 
     
    set intc_count 0
    set intc_index 0
    set intc_type ""
    
    for {set i 0} {$i < [llength $instnames_objs]} {incr i} {
        if { [common::get_property IP_TYPE [lindex $instnames_objs $i]] == "INTERRUPT_CNTLR" } {
            set intc_type [common::get_property INTC_TYPE [hsi::get_sw_cores [lindex $peri_dirs $i]]]
            
            if { $intc_type == "" } {
                set intc_type "XIntc"
                if { [common::get_property IP_NAME [lindex $instnames_objs $i]] == "ps7_scugic" } {
                    set intc_type "XScuGic"
                }
            }

            set pos $intc_count
            incr intc_count
            set intc_index $i
            # If intc_type connected to CortexA9 is not ps7_scugic, search rest of the list
            # until ps7_scugic is found
            # We should parse the connections and figure out if an INTC is connected to INTR port
            # of the processor, rather than relying on processor/INTC types            
            if { $proc_instance == "ps7_cortexa9" && [common::get_property IP_NAME [lindex $instnames_objs $i]] != "ps7_scugic" } {
                continue
            }            
           
           if { $intc_count && $intc_index > $pos } {
                set temp [lindex $peri_dirs $i]
                set peri_dirs [lreplace $peri_dirs $i $i]
                set peri_dirs [linsert $peri_dirs $pos $temp]

                set temp [lindex $include_files $i]
                set include_files [lreplace $include_files $i $i]
                set include_files [linsert $include_files $pos $temp]
                
                set temp [lindex $src_files $i]
                set src_files [lreplace $src_files $i $i]
                set src_files [linsert $src_files $pos $temp]
                
                set temp [lindex $src_files_base $i]
                set src_files_base [lreplace $src_files_base $i $i]
                set src_files_base [linsert $src_files_base $pos $temp]                
                
                set temp [lindex $func_defs $i]
                set func_defs [lreplace $func_defs $i $i]
                set func_defs [linsert $func_defs $pos $temp]
                
                set temp [lindex $func_calls $i]
                set func_calls [lreplace $func_calls $i $i]
                set func_calls [linsert $func_calls $pos $temp]
                
                set temp [lindex $func_init_codes $i]
                set func_init_codes [lreplace $func_init_codes $i $i]
                set func_init_codes [linsert $func_init_codes $pos $temp]           
                
                incr pos
            }            
        }    
    }
    
    # flattening the list of lists into a single list
    set include_files [join $include_files]
    
    # Removing duclicate includes
    variable u_include_files [list]
    
    for {set i 0} {$i < [llength $include_files]} {incr i} {
        for {set j 0} {$j < [llength $u_include_files]} {incr j} {
            if {[lindex $include_files $i] == [lindex $u_include_files $j]} {
                break
            }        
        }
        if { $j == [llength $u_include_files]} {
            lappend u_include_files [lindex $include_files $i]        
        }
    } 
    
    # generate main testperiph.c in output folder
    set fid [open "testperiph.c" "w+"];
    
    variable l_script_data [list]
    
    fill_static_info $fid
    
    # write include statements 
    lappend l_script_data "#include <stdio.h>\n"; 
    lappend l_script_data "#include \"xparameters.h\"\n"; 
    lappend l_script_data "#include \"xil_cache.h\"\n"; 
    
    foreach step $l_script_data {
        puts -nonewline $fid $step
    }
    
    set l_script_data []
    
   
    for {set i 0} {$i < [llength $u_include_files]} {incr i} {
        if { [lindex $u_include_files $i] != ""} {
            set u_include_file [format {#include "%s"} [lindex $u_include_files $i]]
            lappend l_script_data $u_include_file        
        } 
    }
    
    # declare functions
    for {set i 0} {$i < [llength $func_defs]} {incr i} {
        if { [lindex $func_defs $i] != ""} {
            lappend l_script_data [lindex $func_defs $i]
        } 
    }
    
    foreach step $l_script_data {
        puts $fid $step
    }
    set l_script_data []
    
    # now start main
    lappend l_script_data "int main () \n"; 
    lappend l_script_data "\{\n"; 
    
    # intc is a special variable that we need to write out
    if { $intc_count } {
        lappend l_script_data "   static $intc_type intc;\n"
    }    
   
    # define variables
    for {set i 0} {$i < [llength $func_init_codes]} {incr i} {
        if { [lindex $func_init_codes $i] != "" } {
        lappend l_script_data [lindex $func_init_codes $i]
        lappend l_script_data "\n"
        }
    }
    
    lappend l_script_data "   Xil_ICacheEnable();\n"; 
    lappend l_script_data "   Xil_DCacheEnable();\n"; 
    
    foreach step $l_script_data {
        puts -nonewline $fid $step
    }
    
    set l_script_data []
    
    lappend l_script_data "   print(\"---Entering main---\\n\\r\");\n"
    
    foreach step $l_script_data {
        puts -nonewline $fid $step
    }    
    set l_script_data []
    
    # call functions 
    # we need to make sure that we first call the intc functions     
    for {set i 0} {$i < [llength $func_calls]} {incr i} {
        if { [lindex $func_calls $i] != "" } {
        lappend l_script_data [lindex $func_calls $i]
        }
    }
    
    foreach step $l_script_data {
        puts -nonewline $fid $step
        puts  $fid "\n"
    }    
    set l_script_data []
    
    # For testing purposes, print out Exiting main()
    lappend l_script_data "\n   print(\"---Exiting main---\\n\\r\");\n"
   
    foreach step $l_script_data {
        puts -nonewline $fid $step
    }    
    set l_script_data []
    
    lappend l_script_data "   Xil_DCacheDisable();\n"; 
    lappend l_script_data "   Xil_ICacheDisable();\n"; 
    lappend l_script_data "   return 0;\n\}\n"
    
    
    foreach step $l_script_data {
        puts -nonewline $fid $step
    }    
    
    set l_script_data []

    close $fid;

}

proc swapp_generate {} {
    set is_sw [swapp_is_supported_sw]
    if {$is_sw == 0} {
        puts "Application is not supported for the Sw Design."
        return 0;
    }
    set gen_app [generate_app_sources]
    if {$gen_app == 0} {
        return 0;
    }
    return 1;
}

proc swapp_get_linker_constraints {} {
    return "";
}

proc fill_static_info { fid } {

    # Initial Comments
    lappend l_script_data "/*\n"; 
    lappend l_script_data " *\n"; 
    lappend l_script_data " * Xilinx, Inc.\n"; 
    lappend l_script_data " * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION \"AS IS\" AS A \n"; 
    lappend l_script_data " * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS\n"; 
    lappend l_script_data " * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR \n"; 
    lappend l_script_data " * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION \n"; 
    lappend l_script_data " * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE \n"; 
    lappend l_script_data " * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION\n"; 
    lappend l_script_data " * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO \n"; 
    lappend l_script_data " * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO \n"; 
    lappend l_script_data " * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE \n"; 
    lappend l_script_data " * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY \n"; 
    lappend l_script_data " * AND FITNESS FOR A PARTICULAR PURPOSE.\n"; 
    lappend l_script_data " */\n"; 
    lappend l_script_data "\n"; 
        
    lappend l_script_data "/*\n"; 
    lappend l_script_data " * \n"; 
    lappend l_script_data " *\n"; 
    lappend l_script_data " * This file is a generated sample test application.\n"; 
    lappend l_script_data " *\n"; 
    lappend l_script_data " * This application is intended to test and/or illustrate some \n"; 
    lappend l_script_data " * functionality of your system.  The contents of this file may\n"; 
    lappend l_script_data " * vary depending on the IP in your system and may use existing\n"; 
    lappend l_script_data " * IP driver functions.  These drivers will be generated in your\n"; 
    lappend l_script_data " * SDK application project when you run the \"Generate Libraries\" menu item.\n"; 
    lappend l_script_data " *\n"; 
    lappend l_script_data " */\n"; 
    lappend l_script_data "\n";

    foreach step $l_script_data {
        puts -nonewline $fid $step
    }    
    set l_script_data []
}


proc copy_file {fname od} {
    set ss [split $fname "/"] 
    set last_name [lindex $ss end] 
    append od "/" $last_name
    set fout [open $od "w+"]
    puts $fout "#define TESTAPP_GEN"
    puts $fout ""
    set fin [open $fname "r"]

    while { [gets $fin line]>=0 } {
    	puts $fout $line
    }
    close $fout
    return "";
}

proc copy_files {src_files_base src_files od} {
    for { set i 0 } { $i < [llength $src_files_base] } { incr i} { 
        
        set files_base [lindex $src_files_base $i]
        set files_base [string map {\\ /} $files_base]
        set files [lindex $src_files $i]
        for { set j 0 } { $j < [llength $files] } { incr j } {
            set s [string trim [lindex $files $j]]
            if { $s!="" } {
                set pf $::env(RDI_PLATFORM)    
                if {$pf == "win64" || $pf == "win" || $pf == "win32"} {
                    append files_base "\\" $s
                } else {
                    append files_base "/" $s
                }               
                copy_file $files_base $od
                set files_base [lindex $src_files_base $i]
            }
            set files_base [lindex $src_files_base $i]
        }
    }

}

proc swapp_get_supported_processors {} {
    return "ps7_cortexa9 psu_cortexa53 psu_cortexr5 microblaze psv_cortexa72 psv_cortexr5 psxl_cortexa78 psxl_cortexr52 psx_cortexa78 psx_cortexr52 microblaze_riscv";
}

proc swapp_get_supported_os {} {
    return "standalone xilkernel";
}

