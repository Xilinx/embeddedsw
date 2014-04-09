proc generate {drv_handle} {
    set ip [get_cells $drv_handle]

    #disabling non-secure dma 
    if { [string match -nocase $ip "ps7_dma_ns"] } {
        set_property NAME none $drv_handle
    }
}
