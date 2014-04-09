proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    set clk_freq [get_ip_param_value $ip C_SDIO_CLK_FREQ_HZ]
    set_property CONFIG.clock-frequency "$clk_freq" $drv_handle
}


