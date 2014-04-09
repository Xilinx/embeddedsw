proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    set has_xin [get_ip_param_value $ip C_HAS_EXTERNAL_XIN]
    set clock_port "S_AXI_ACLK"
    if { [string match -nocase "$has_xin" "1"] } {
        set clock_port "xin"

    }
    set freq [xget_ip_clk_pin_freq $ip "$clock_port"]
    set_property clock-frequency $freq $drv_handle

    set consoleip [get_property CONFIG.console_device [get_os]]
    if { [string match -nocase $consoleip $ip] } {
        hsm::utils::set_os_parameter_value "console" "ttyS0,115200"
    }
}
