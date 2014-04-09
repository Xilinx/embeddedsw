
# For calling from top level BSP
proc bsp_drc {os_handle} {
}

# If standalone purpose
proc device_tree_drc {os_handle} {
	bsp_drc $os_handle
    hsm::utils::add_new_child_node $os_handle "chosen"
    hsm::utils::add_new_child_node $os_handle "global_params"
}

proc generate {lib_handle} {
}

proc post_generate {os_handle} {
    add_chosen $os_handle
    clean_os $os_handle
}

proc clean_os { os_handle } {
    #deleting unwanted child nodes of OS for dumping into dts file
    set node [get_child_nodes -of_objects $os_handle "global_params"]
    if { [llength $node] } {
        delete_objs $node
    }
}

proc add_chosen { os_handle } {
    set bootargs [get_property CONFIG.bootargs $os_handle]
    if { [llength $bootargs] == 0 } {
        set console [hsm::utils::get_os_parameter_value "console"]
        if { [llength $console] } {
            set bootargs "console=$console"
        }
    }
    if { [llength $bootargs]  } {
        set chosen_node [get_child_nodes -of_objects $os_handle "chosen"]
        if { [llength $chosen_node] == 0  } {
            set chosen_node [hsm::utils::add_new_child_node $os_handle "chosen"]
        }
        hsm::utils::add_new_property $chosen_node "bootargs" string $bootargs
    }
    set consoleip [get_property CONFIG.console_device $os_handle]
    hsm::utils::add_new_property $chosen_node "linux,stdout-path" aliasref $consoleip
}
