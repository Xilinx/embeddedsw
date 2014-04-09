proc generate {drv_handle} {
    set i2c_count [hsm::utils::get_os_parameter_value "i2c_count"]
    if { [llength $i2c_count] == 0 } {
        set i2c_count 0
    }
    set_property CONFIG.bus-id "$i2c_count" $drv_handle
    incr i2c_count
    hsm::utils::set_os_parameter_value "i2c_count" $i2c_count
    set i2c_count [hsm::utils::get_os_parameter_value "i2c_count"]
}
