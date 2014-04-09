
proc generate {drv_handle} {
     set count 32
     set ip [get_cells $drv_handle]
     set_property CONFIG.emio-gpio-width "[get_ip_param_value $ip C_EMIO_GPIO_WIDTH]" $drv_handle
     set gpiomask [get_ip_param_value $ip "C_MIO_GPIO_MASK"]
     set mask [expr {$gpiomask & 0xffffffff}]
     set_property CONFIG.gpio-mask-low "$mask" $drv_handle
     set mask [expr {$gpiomask>>$count}]
     set mask [expr {$mask & 0xffffffff}]
     set_property CONFIG.gpio-mask-high "$mask" $drv_handle
}


