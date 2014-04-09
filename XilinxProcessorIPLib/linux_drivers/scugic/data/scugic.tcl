proc get_baseaddr { slave_ip } {
    set mem_range [lindex [xget_ip_mem_ranges $slave_ip] 0]
    return [get_property BASE_VALUE $mem_range]
}
proc get_highaddr { slave_ip } {
    set mem_range [lindex [xget_ip_mem_ranges $slave_ip] 0]
    return [get_property HIGH_VALUE $mem_range]
}

proc generate {drv_handle} {
   # set baseaddr [get_baseaddr [get_cells $drv_handle]]
   # set highaddr [get_highaddr [get_cells $drv_handle]]
   # set size [expr $highaddr -$baseaddr + 1]
   # set_property CONFIG.reg "$baseaddr $size $baseaddr $size" $drv_handle
}
