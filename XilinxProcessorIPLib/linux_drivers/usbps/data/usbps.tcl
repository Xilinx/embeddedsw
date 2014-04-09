proc generate {drv_handle} {
    set ip [get_cells $drv_handle]
    set value [get_property CONFIG.C_USB_RESET $ip]
    if { [llength $value] } {
        regsub -all "MIO" $value "" value
        if { $value != "-1" && [llength $value] !=0  } {
            set_property CONFIG.usb-reset "ps7_gpio_0 $value 0" $drv_handle
        }
    }  
}

