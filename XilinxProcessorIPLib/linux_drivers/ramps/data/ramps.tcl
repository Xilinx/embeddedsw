proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    if { [string match -nocase $ip "ps7_ram_1"] } {
        set_property NAME none $drv_handle
    }
}
