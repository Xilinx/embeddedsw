
# For calling from top level BSP
proc bsp_drc {os_handle} {
}

# If standalone purpose
proc device_tree_drc {os_handle} {
	bsp_drc $os_handle
    hsm::utils::add_new_child_node $os_handle "global_params"
}

proc generate {lib_handle} {
}

proc post_generate {os_handle} {
    add_chosen $os_handle 
    clean_os $os_handle
    add_ps7_pmu $os_handle
}

proc clean_os { os_handle } {
    #deleting unwanted child nodes of OS for dumping into dts file
    set node [get_child_nodes -of_objects $os_handle "global_params"]
    if { [llength $node] } {
        delete_objs $node
    }
}

proc add_chosen { os_handle } {
    set system_node [hsm::utils::get_or_create_child_node $os_handle "dtg.system"]
    set chosen_node [hsm::utils::get_or_create_child_node $system_node "chosen"]

    #getting boot arguments 
    set bootargs [get_property CONFIG.bootargs $os_handle]
    if { [llength $bootargs] == 0 } {
        set console [hsm::utils::get_os_parameter_value "console"]
        if { [llength $console] } {
            set bootargs "console=$console"
        }
    }
    if { [llength $bootargs]  } {
        hsm::utils::add_new_property $chosen_node "bootargs" string $bootargs
    }
    set consoleip [get_property CONFIG.console_device $os_handle]
    hsm::utils::add_new_property $chosen_node "linux,stdout-path" aliasref $consoleip
}

#Hack to disable ps7_pmu from bus and add it explicitly parallel to cpu
proc add_ps7_pmu { os_handle } {
    set proc_name [get_property HW_INSTANCE [get_sw_processor]]
    set hwproc [get_cells -filter " NAME==$proc_name"]
    set proctype [get_property IP_NAME $hwproc]
    if { [string match -nocase $proctype "ps7_cortexa9"] } {


        #get PMU driver handler and disabling it 
        set all_drivers [get_drivers] 
        foreach driver $all_drivers {
            set hwinst [get_property HW_INSTANCE $driver]
            set ip [get_cells $hwinst]
            set iptype [get_property IP_NAME $ip]
            if { [string match -nocase $iptype "ps7_pmu" ] } {
                set_property NAME "none" $driver
            }
        }

        #adding hardcoded pmu into system node
        set ps_node [hsm::utils::get_or_create_child_node $os_handle "dtg.ps"]
        set pmu_node [hsm::utils::get_or_create_child_node $ps_node "ps7_pmu"]
        hsm::utils::add_new_property $pmu_node "reg" hexintlist "0xf8891000 0x1000 0xf8893000 0x1000"
        hsm::utils::add_new_property $pmu_node "reg-names" stringlist "cpu0 cpu1"
        hsm::utils::add_new_property $pmu_node "compatible" stringlist "arm,cortex-a9-pmu"
    }
}

