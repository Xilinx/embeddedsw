proc generate {drv_handle} {
    set dma_ip [get_cells $drv_handle]
    set dma_count [hsm::utils::get_os_parameter_value "dma_count"]
    if { [llength $dma_count] == 0 } {
        set dma_count 0
    }
    set axiethernetfound 0
    set connected_ip [hsm::utils::get_connected_stream_ip $dma_ip "M_AXIS_MM2S"]
    if { [llength $connected_ip] } {
        set connected_ip_type [get_property IP_NAME $connected_ip]
        if { [string match -nocase $connected_ip_type axi_ethernet ] 
            || [string match -nocase $connected_ip_type axi_ethernet_buffer ] } {
                set axiethernetfound 1
        }
    }
    if { $axiethernetfound != 1 } {
        set mem_range  [lindex [xget_ip_mem_ranges $dma_ip] 0 ]
        set baseaddr   [get_property BASE_VALUE $mem_range]
        set highaddr   [get_property HIGH_VALUE $mem_range]
        set tx_chan [get_ip_param_value $dma_ip C_INCLUDE_MM2S]
        if { $tx_chan == 1 } {
            set connected_ip [hsm::utils::get_connected_stream_ip $dma_ip "M_AXIS_MM2S"]
            set tx_chan [add_dma_channel $drv_handle "axi-dma" $baseaddr "MM2S" $dma_count ]
            set intr_info [::hsm::utils::get_dtg_interrupt_info $dma_ip "mm2s_introut" ]
            set intc [hsm::utils::get_interrupt_parent $dma_ip "mm2s_introut"]
            if { [llength $intr_info] } {
                hsm::utils::add_new_property $tx_chan "interrupts" int $intr_info
                hsm::utils::add_new_property $tx_chan "interrupt-parent" reference $intc
            }
            hsm::utils::add_new_property $tx_chan "axistream-connected" reference $connected_ip
            hsm::utils::add_new_property $tx_chan "axistream-control-connected" reference $connected_ip
        }
        set rx_chan [get_ip_param_value $dma_ip C_INCLUDE_S2MM]
        if { $rx_chan ==1 } {
            set connected_ip [hsm::utils::get_connected_stream_ip $dma_ip "S_AXIS_S2MM"]
            set rx_chan [add_dma_channel $drv_handle "axi-dma" [expr $baseaddr + 0x30] "S2MM" $dma_count]
            set intr_info [::hsm::utils::get_dtg_interrupt_info $dma_ip "s2mm_introut"]
            set intc [hsm::utils::get_interrupt_parent $dma_ip "s2mm_introut"]
            if { [llength $intr_info] } {
                hsm::utils::add_new_property $rx_chan "interrupts" int $intr_info
                hsm::utils::add_new_property $rx_chan "interrupt-parent" reference $intc
            }
            hsm::utils::add_new_property $rx_chan "axistream-connected-slave" reference $connected_ip
            hsm::utils::add_new_property $rx_chan "axistream-control-connected-slave" reference $connected_ip
        }
    } else {
        set_property axistream-connected "$connected_ip" $drv_handle
        set_property axistream-control-connected "$connected_ip" $drv_handle
    }
    incr dma_count
    hsm::utils::set_os_parameter_value "dma_count" $dma_count
}

proc add_dma_channel { drv_handle xdma addr mode devid} {
    set ip [get_cells $drv_handle]
    set modellow [string tolower $mode]
    set modeIndex [string index $mode 0]
    set node_name [format "dma-channel@%x" $addr]
    set dma_channel [hsm::utils::add_new_child_node $drv_handle $node_name]
    hsm::utils::add_new_property $dma_channel "compatible" stringlist [format "xlnx,%s-%s-channel" $xdma $modellow] 
    set tmp [get_ip_param_value $ip [format "C_INCLUDE_%s_DRE" $mode]]
    hsm::utils::add_new_property $dma_channel "xlnx,include-dre" hexint $tmp

    hsm::utils::add_new_property $dma_channel "xlnx,device-id" hexint $devid
    set tmp [get_ip_param_value $ip [format "C_%s_AXIS_%s_TDATA_WIDTH" $modeIndex $mode ] ]
    if { [llength $tmp] } {
        hsm::utils::add_new_property $dma_channel "xlnx,datawidth" hexint $tmp
    }
    set tmp [get_ip_param_value $ip [format "C_%s_AXIS_%s_DATA_WIDTH"  $modeIndex $mode ] ]
    if { [llength $tmp] } {
        hsm::utils::add_new_property $dma_channel "xlnx,datawidth" hexint $tmp
    }

    if { [string compare -nocase $xdma "axi-dma"] != 0 } {
        set tmp [get_ip_param_value $ip [format "C_%s_GENLOCK_MODE" $mode]]
        hsm::utils::add_new_property $dma_channel "xlnx,genlock-mode" hexint $tmp
    }


    return $dma_channel
}
