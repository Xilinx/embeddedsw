proc get_baseaddr { slave_ip } {
    set mem_range [lindex [xget_ip_mem_ranges $slave_ip] 0]
    return [get_property BASE_VALUE $mem_range]
}
proc get_highaddr { slave_ip } {
    set mem_range [lindex [xget_ip_mem_ranges $slave_ip] 0]
    return [get_property HIGH_VALUE $mem_range]
}

proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    set baseaddr [get_baseaddr $ip]
    set highaddr [get_highaddr $ip]
    set main_memory [get_property CONFIG.main_memory [get_os]]
    if { [string match -nocase "$main_memory" $ip] }  {
        set baseaddr "0x0"
    }
    set size [expr $highaddr -$baseaddr + 1]
    set value [format "0x%x 0x%x" $baseaddr $size]
    set_property CONFIG.reg "$value" $drv_handle
}
