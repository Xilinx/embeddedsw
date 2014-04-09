proc generate {drv_handle} {
    set dma_ip [get_cells $drv_handle]
    set intf  [get_intf_pins -of_objects $dma_ip "M_AXIS_MM2S"]
    if { [llength $intf] } {
        set intf_net [get_intf_nets -of_objects $intf]
        if { [llength $intf_net] } {
            set target_intf [lindex [get_intf_pins -of_objects $intf_net -filter "TYPE==TARGET"] 0]
            if { [llength $target_intf] } {
                set connected_ip [get_cells -of_objects $target_intf]
                set_property axistream-connected "$connected_ip" $drv_handle
                set_property axistream-control-connected "$connected_ip" $drv_handle
            }
        }
    }
}
