proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    set consoleip [get_property CONFIG.console_device [get_os]]
    if { [string match -nocase $consoleip $ip] } {
        set ip_type [get_property IP_NAME $ip]
        if { [string match -nocase $ip_type] } {
            hsm::utils::set_os_parameter_value "console" "ttyUL0,115200"
        } else {
            hsm::utils::set_os_parameter_value "console" "ttyUL0,[get_ip_param_value $ip C_BAUDRATE]"
        }
    }
}
