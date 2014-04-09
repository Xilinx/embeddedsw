proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    set count [get_ip_param_value $ip "C_NUM_BANKS_MEM"]
    if { [llength $count] == 0 } {
        set count 1
    }
    for {set x 0} { $x < $count} {incr x} {
        set datawidth [get_ip_param_value $ip [format "C_MEM%d_WIDTH" $x]]
        set_property bank-width "[expr ($datawidth/8)]" $drv_handle
    }
}
