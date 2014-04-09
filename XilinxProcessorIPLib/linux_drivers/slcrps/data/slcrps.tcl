
proc generate {drv_handle} {
    gen_clocks_node $drv_handle
}

proc gen_clocks_node { drv_handle} {
    set clocks_child_name "clocks"
    set clocks [hsm::utils::add_new_child_node $drv_handle $clocks_child_name]
    hsm::utils::add_new_property $clocks "#address-cells" int 1
    hsm::utils::add_new_property $clocks "#size-cells" int 0
    set clkc [hsm::utils::add_new_child_node $clocks "clkc:clkc"]
    hsm::utils::add_new_property $clkc "#clock-cells" int 1
    hsm::utils::add_new_property $clkc clock-output-names stringlist  { armpll ddrpll iopll cpu_6or4x \
									cpu_3or2x cpu_2x cpu_1x ddr2x ddr3x \
									dci lqspi smc pcap gem0 gem1 \
									fclk0 fclk1 fclk2 fclk3 can0 can1 \
									sdio0 sdio1 uart0 uart1 spi0 spi1 \
									dma usb0_aper usb1_aper gem0_aper \
									gem1_aper sdio0_aper sdio1_aper \
									spi0_aper spi1_aper can0_aper can1_aper \
									i2c0_aper i2c1_aper uart0_aper uart1_aper \
									gpio_aper lqspi_aper smc_aper swdt \
									dbg_trc } 
    hsm::utils::add_new_property $clkc compatible stringlist "xlnx,ps7-clkc"
    hsm::utils::add_new_property $clkc fclk-enable int 0xf
    hsm::utils::add_new_property $clkc ps-clk-frequency int 33333333
    return $clocks
}

