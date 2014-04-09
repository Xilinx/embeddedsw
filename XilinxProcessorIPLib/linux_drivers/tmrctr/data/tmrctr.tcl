proc generate {drv_handle} {
    #adding clock frequency
    set ip [get_cells $drv_handle]
    set clk [get_pins -of_objects $ip "S_AXI_ACLK"]
    if {[llength $clk] } {
        set freq [get_property CLK_FREQ $clk]
        set_property clock-frequency "$freq" $drv_handle
    }
}
