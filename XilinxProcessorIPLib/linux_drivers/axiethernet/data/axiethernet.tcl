
proc gen_mdio_node {drv_handle} {
    set mdio_child_name "mdio"
    set mdio [hsm::utils::add_new_child_node $drv_handle $mdio_child_name]
    hsm::utils::add_new_property $mdio "#address-cells" int 1 
    hsm::utils::add_new_property $mdio "#size-cells" int 0 
    return $mdio
}
proc generate {drv_handle} {
    gen_mdio_node $drv_handle

    #adding stream connectivity
    set eth_ip [get_cells $drv_handle]
    set intf [get_intf_pins -of_objects $eth_ip "AXI_STR_RXD"]
    if { [llength $intf] } {
        set intf_net [get_intf_nets -of_objects $intf ]
        if { [llength $intf_net]  } {
            set target_intf [lindex [get_intf_pins -of_objects $intf_net -filter "TYPE==TARGET" ] 0]
            if { [llength $target_intf] } {
                set connected_ip [get_cells -of_objects $target_intf]
                set_property axistream-connected "$connected_ip" $drv_handle
                set_property axistream-control-connected "$connected_ip" $drv_handle
            }
        } 
    }

    #adding clock frequency
    set clk [get_pins -of_objects $eth_ip "S_AXI_ACLK"]
    if {[llength $clk] } {
        set freq [get_property CLK_FREQ $clk]
        set_property clock-frequency "$freq" $drv_handle
    }
}

