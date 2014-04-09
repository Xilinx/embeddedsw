proc generate {drv_handle} {

    set ip [get_cells $drv_handle]
    set clk ""
    set clkhandle [get_pins -of_objects $ip "CLK"]
    if { [string compare -nocase $clkhandle ""] != 0 } {
        set clk [get_property CLK_FREQ $clkhandle]
    }
    if { [llength $ip]  } {
        set_property CONFIG.clock-frequency    "$clk" $drv_handle
        set_property CONFIG.timebase-frequency "$clk" $drv_handle
    }

    set icache_size [get_ip_param_value $ip "C_CACHE_BYTE_SIZE"]
	set icache_base [get_ip_param_value $ip "C_ICACHE_BASEADDR"]
	set icache_high [get_ip_param_value $ip "C_ICACHE_HIGHADDR"]
	set dcache_size [get_ip_param_value $ip "C_DCACHE_BYTE_SIZE"]
	set dcache_base [get_ip_param_value $ip "C_DCACHE_BASEADDR"]
	set dcache_high [get_ip_param_value $ip "C_DCACHE_HIGHADDR"]
    set icache_line_size [expr 4*[get_ip_param_value $ip "C_ICACHE_LINE_LEN"]]
	set dcache_line_size [expr 4*[get_ip_param_value $ip "C_DCACHE_LINE_LEN"]]


    if { [llength $icache_size] != 0 } {
        set_property CONFIG.i-cache-baseaddr  "$icache_base"      $drv_handle
        set_property CONFIG.i-cache-highaddr  "$icache_high"      $drv_handle
        set_property CONFIG.i-cache-size      "$icache_size"      $drv_handle
        set_property CONFIG.i-cache-line-size "$icache_line_size" $drv_handle
    }
    if { [llength $dcache_size] != 0 } {
        set_property CONFIG.d-cache-baseaddr  "$dcache_base"      $drv_handle
        set_property CONFIG.d-cache-highaddr  "$dcache_high"      $drv_handle
        set_property CONFIG.d-cache-size      "$dcache_size"      $drv_handle
        set_property CONFIG.d-cache-line-size "$dcache_line_size" $drv_handle
    } 

    set model "[get_property IP_NAME $ip],[get_ip_version $ip]"
    set_property CONFIG.model $model $drv_handle
}
