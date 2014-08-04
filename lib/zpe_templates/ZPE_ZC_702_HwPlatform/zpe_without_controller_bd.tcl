
################################################################
# This is a generated script based on design: zpe_without_controller
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2014.3
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   puts "ERROR: This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source zpe_without_controller_script.tcl

# If you do not already have a project created,
# you can create a project using the following command:
#    create_project project_1 myproj -part xc7z020clg484-1


# CHANGE DESIGN NAME HERE
set design_name zpe_without_controller

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# CHECKING IF PROJECT EXISTS
if { [get_projects -quiet] eq "" } {
   puts "ERROR: Please open or create a project!"
   return 1
}


# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} ne "" && ${cur_design} eq ${design_name} } {

   # Checks if design is empty or not
   if { $list_cells ne "" } {
      set errMsg "ERROR: Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
      set nRet 1
   } else {
      puts "INFO: Constructing design in IPI design <$design_name>..."
   }
} elseif { ${cur_design} ne "" && ${cur_design} ne ${design_name} } {

   if { $list_cells eq "" } {
      puts "INFO: You have an empty design <${cur_design}>. Will go ahead and create design..."
   } else {
      set errMsg "ERROR: Design <${cur_design}> is not empty! Please do not source this script on non-empty designs."
      set nRet 1
   }
} else {

   if { [get_files -quiet ${design_name}.bd] eq "" } {
      puts "INFO: Currently there is no design <$design_name> in project, so creating one..."

      create_bd_design $design_name

      puts "INFO: Making design <$design_name> as current_bd_design."
      current_bd_design $design_name

   } else {
      set errMsg "ERROR: Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
      set nRet 3
   }

}

puts "INFO: Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   puts $errMsg
   return $nRet
}

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     puts "ERROR: Unable to find parent cell <$parentCell>!"
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     puts "ERROR: Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set DDR [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR ]
  set FIXED_IO [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO ]

  # Create ports

  # Create instance: atg_acp, and set properties
  set atg_acp [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_traffic_gen:2.0 atg_acp ]
  set_property -dict [ list CONFIG.ATG_OPTIONS {Custom} CONFIG.C_ATG_MODE {AXI4} CONFIG.C_ATG_MODE_L2 {Advanced} CONFIG.C_M_AXI_DATA_WIDTH {64} CONFIG.C_S_AXI_DATA_WIDTH {32} CONFIG.DATA_SIZE_AVG {16} CONFIG.TRAFFIC_PROFILE {Video}  ] $atg_acp

  # Create instance: atg_hp0, and set properties
  set atg_hp0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_traffic_gen:2.0 atg_hp0 ]
  set_property -dict [ list CONFIG.C_M_AXI_DATA_WIDTH {64} CONFIG.DATA_SIZE_AVG {16}  ] $atg_hp0

  # Create instance: atg_hp1, and set properties
  set atg_hp1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_traffic_gen:2.0 atg_hp1 ]
  set_property -dict [ list CONFIG.C_M_AXI_DATA_WIDTH {64} CONFIG.DATA_SIZE_AVG {16}  ] $atg_hp1

  # Create instance: atg_hp2, and set properties
  set atg_hp2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_traffic_gen:2.0 atg_hp2 ]
  set_property -dict [ list CONFIG.C_M_AXI_DATA_WIDTH {64} CONFIG.DATA_SIZE_AVG {16}  ] $atg_hp2

  # Create instance: atg_hp3, and set properties
  set atg_hp3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_traffic_gen:2.0 atg_hp3 ]
  set_property -dict [ list CONFIG.C_M_AXI_DATA_WIDTH {64} CONFIG.DATA_SIZE_AVG {16}  ] $atg_hp3

  # Create instance: axi_gpio_0, and set properties
  set axi_gpio_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0 ]
  set_property -dict [ list CONFIG.C_ALL_OUTPUTS {1} CONFIG.C_DOUT_DEFAULT {0x00000000} CONFIG.C_GPIO_WIDTH {1}  ] $axi_gpio_0

  # Create instance: axi_mem_intercon, and set properties
  set axi_mem_intercon [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon ]
  set_property -dict [ list CONFIG.Component_Name {design_2_axi_mem_intercon_0} CONFIG.ENABLE_ADVANCED_OPTIONS {1} CONFIG.M00_HAS_REGSLICE {1} CONFIG.M01_HAS_REGSLICE {1} CONFIG.M02_HAS_REGSLICE {1} CONFIG.M03_HAS_REGSLICE {1} CONFIG.M04_HAS_REGSLICE {1} CONFIG.M05_HAS_REGSLICE {1} CONFIG.M06_HAS_REGSLICE {1} CONFIG.M07_HAS_REGSLICE {1} CONFIG.M08_HAS_REGSLICE {1} CONFIG.M09_HAS_REGSLICE {1} CONFIG.NUM_MI {10} CONFIG.NUM_SI {1} CONFIG.S00_HAS_REGSLICE {1} CONFIG.S01_HAS_REGSLICE {1} CONFIG.S02_HAS_REGSLICE {1} CONFIG.STRATEGY {1}  ] $axi_mem_intercon

  # Create instance: axi_mem_intercon_1, and set properties
  set axi_mem_intercon_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon_1 ]
  set_property -dict [ list CONFIG.Component_Name {design_1_axi_mem_intercon_1_1} CONFIG.NUM_MI {1}  ] $axi_mem_intercon_1

  # Create instance: axi_mem_intercon_2, and set properties
  set axi_mem_intercon_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon_2 ]
  set_property -dict [ list CONFIG.Component_Name {design_1_axi_mem_intercon_2_2} CONFIG.NUM_MI {1}  ] $axi_mem_intercon_2

  # Create instance: axi_mem_intercon_3, and set properties
  set axi_mem_intercon_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon_3 ]
  set_property -dict [ list CONFIG.Component_Name {design_1_axi_mem_intercon_3_3} CONFIG.NUM_MI {1}  ] $axi_mem_intercon_3

  # Create instance: axi_mem_intercon_4, and set properties
  set axi_mem_intercon_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon_4 ]
  set_property -dict [ list CONFIG.Component_Name {design_1_axi_mem_intercon_4_4} CONFIG.NUM_MI {1}  ] $axi_mem_intercon_4

  # Create instance: axi_mem_intercon_5, and set properties
  set axi_mem_intercon_5 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_mem_intercon_5 ]
  set_property -dict [ list CONFIG.Component_Name {design_1_axi_mem_intercon_5_5} CONFIG.NUM_MI {1}  ] $axi_mem_intercon_5

  # Create instance: proc_sys_reset, and set properties
  set proc_sys_reset [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset ]
  set_property -dict [ list CONFIG.C_AUX_RESET_HIGH {0}  ] $proc_sys_reset

  # Create instance: processing_system7_1, and set properties
  set processing_system7_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.4 processing_system7_1 ]
  set_property -dict [ list CONFIG.PCW_ACT_CAN_PERIPHERAL_FREQMHZ {23.809523} CONFIG.PCW_ACT_ENET0_PERIPHERAL_FREQMHZ {25.000000} CONFIG.PCW_ACT_ENET1_PERIPHERAL_FREQMHZ {10.000000} CONFIG.PCW_ACT_FPGA0_PERIPHERAL_FREQMHZ {100.000000} CONFIG.PCW_ACT_FPGA1_PERIPHERAL_FREQMHZ {50.000000} CONFIG.PCW_ACT_FPGA2_PERIPHERAL_FREQMHZ {50.000000} CONFIG.PCW_ACT_FPGA3_PERIPHERAL_FREQMHZ {50.000000} CONFIG.PCW_ACT_PCAP_PERIPHERAL_FREQMHZ {200.000000} CONFIG.PCW_ACT_QSPI_PERIPHERAL_FREQMHZ {190.476196} CONFIG.PCW_ACT_SDIO_PERIPHERAL_FREQMHZ {50.000000} CONFIG.PCW_ACT_SMC_PERIPHERAL_FREQMHZ {10.000000} CONFIG.PCW_ACT_SPI_PERIPHERAL_FREQMHZ {10.000000} CONFIG.PCW_ACT_TPIU_PERIPHERAL_FREQMHZ {10.000000} CONFIG.PCW_ACT_UART_PERIPHERAL_FREQMHZ {50.000000} CONFIG.PCW_APU_CLK_RATIO_ENABLE {6:2:1} CONFIG.PCW_APU_PERIPHERAL_FREQMHZ {666.666666} CONFIG.PCW_CAN0_CAN0_IO {MIO 46 .. 47} CONFIG.PCW_CAN0_GRP_CLK_ENABLE {0} CONFIG.PCW_CAN0_PERIPHERAL_CLKSRC {External} CONFIG.PCW_CAN0_PERIPHERAL_ENABLE {1} CONFIG.PCW_CAN1_PERIPHERAL_CLKSRC {External} CONFIG.PCW_CAN1_PERIPHERAL_ENABLE {0} CONFIG.PCW_CAN_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_CAN_PERIPHERAL_FREQMHZ {23.8095} CONFIG.PCW_CAN_PERIPHERAL_VALID {1} CONFIG.PCW_CLK0_FREQ {100000000} CONFIG.PCW_CPU_CPU_6X4X_MAX_RANGE {667} CONFIG.PCW_CPU_PERIPHERAL_CLKSRC {ARM PLL} CONFIG.PCW_CRYSTAL_PERIPHERAL_FREQMHZ {33.333333} CONFIG.PCW_DCI_PERIPHERAL_CLKSRC {DDR PLL} CONFIG.PCW_DCI_PERIPHERAL_FREQMHZ {10.159} CONFIG.PCW_DDR_PERIPHERAL_CLKSRC {DDR PLL} CONFIG.PCW_DDR_RAM_HIGHADDR {0x3FFFFFFF} CONFIG.PCW_ENET0_ENET0_IO {MIO 16 .. 27} CONFIG.PCW_ENET0_GRP_MDIO_ENABLE {1} CONFIG.PCW_ENET0_GRP_MDIO_IO {MIO 52 .. 53} CONFIG.PCW_ENET0_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {1} CONFIG.PCW_ENET0_PERIPHERAL_FREQMHZ {100 Mbps} CONFIG.PCW_ENET0_RESET_ENABLE {1} CONFIG.PCW_ENET0_RESET_IO {MIO 11} CONFIG.PCW_ENET1_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_ENET1_PERIPHERAL_ENABLE {0} CONFIG.PCW_ENET1_RESET_ENABLE {0} CONFIG.PCW_ENET1_RESET_IO {<Select>} CONFIG.PCW_ENET_RESET_POLARITY {Active Low} CONFIG.PCW_EN_4K_TIMER {0} CONFIG.PCW_EN_CAN0 {1} CONFIG.PCW_EN_CLK0_PORT {1} CONFIG.PCW_EN_ENET0 {1} CONFIG.PCW_EN_GPIO {1} CONFIG.PCW_EN_I2C0 {1} CONFIG.PCW_EN_QSPI {1} CONFIG.PCW_EN_SDIO0 {1} CONFIG.PCW_EN_UART1 {1} CONFIG.PCW_EN_USB0 {1} CONFIG.PCW_FCLK0_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_FCLK1_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_FCLK2_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_FCLK3_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {100} CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {50} CONFIG.PCW_FPGA2_PERIPHERAL_FREQMHZ {50} CONFIG.PCW_FPGA3_PERIPHERAL_FREQMHZ {50} CONFIG.PCW_GPIO_EMIO_GPIO_ENABLE {0} CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {1} CONFIG.PCW_GPIO_MIO_GPIO_IO {MIO} CONFIG.PCW_GPIO_PERIPHERAL_ENABLE {0} CONFIG.PCW_I2C0_GRP_INT_ENABLE {0} CONFIG.PCW_I2C0_I2C0_IO {MIO 50 .. 51} CONFIG.PCW_I2C0_PERIPHERAL_ENABLE {1} CONFIG.PCW_I2C0_RESET_ENABLE {1} CONFIG.PCW_I2C0_RESET_IO {MIO 13} CONFIG.PCW_I2C1_PERIPHERAL_ENABLE {0} CONFIG.PCW_I2C1_RESET_ENABLE {0} CONFIG.PCW_I2C1_RESET_IO {<Select>} CONFIG.PCW_I2C_PERIPHERAL_FREQMHZ {111.111115} CONFIG.PCW_I2C_RESET_POLARITY {Active Low} CONFIG.PCW_MIO_0_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_0_PULLUP {enabled} CONFIG.PCW_MIO_0_SLEW {fast} CONFIG.PCW_MIO_10_DIRECTION {inout} CONFIG.PCW_MIO_10_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_10_PULLUP {enabled} CONFIG.PCW_MIO_10_SLEW {slow} CONFIG.PCW_MIO_11_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_11_PULLUP {enabled} CONFIG.PCW_MIO_11_SLEW {slow} CONFIG.PCW_MIO_12_DIRECTION {inout} CONFIG.PCW_MIO_12_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_12_PULLUP {enabled} CONFIG.PCW_MIO_12_SLEW {slow} CONFIG.PCW_MIO_13_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_13_PULLUP {enabled} CONFIG.PCW_MIO_13_SLEW {slow} CONFIG.PCW_MIO_14_DIRECTION {inout} CONFIG.PCW_MIO_14_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_14_PULLUP {enabled} CONFIG.PCW_MIO_14_SLEW {slow} CONFIG.PCW_MIO_15_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_15_PULLUP {enabled} CONFIG.PCW_MIO_15_SLEW {fast} CONFIG.PCW_MIO_16_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_16_PULLUP {enabled} CONFIG.PCW_MIO_16_SLEW {fast} CONFIG.PCW_MIO_17_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_17_PULLUP {enabled} CONFIG.PCW_MIO_17_SLEW {fast} CONFIG.PCW_MIO_18_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_18_PULLUP {enabled} CONFIG.PCW_MIO_18_SLEW {fast} CONFIG.PCW_MIO_19_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_19_PULLUP {enabled} CONFIG.PCW_MIO_19_SLEW {fast} CONFIG.PCW_MIO_1_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_1_PULLUP {enabled} CONFIG.PCW_MIO_1_SLEW {fast} CONFIG.PCW_MIO_20_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_20_PULLUP {enabled} CONFIG.PCW_MIO_20_SLEW {fast} CONFIG.PCW_MIO_21_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_21_PULLUP {enabled} CONFIG.PCW_MIO_21_SLEW {fast} CONFIG.PCW_MIO_22_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_22_PULLUP {enabled} CONFIG.PCW_MIO_22_SLEW {fast} CONFIG.PCW_MIO_23_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_23_PULLUP {enabled} CONFIG.PCW_MIO_23_SLEW {fast} CONFIG.PCW_MIO_24_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_24_PULLUP {enabled} CONFIG.PCW_MIO_24_SLEW {fast} CONFIG.PCW_MIO_25_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_25_PULLUP {enabled} CONFIG.PCW_MIO_25_SLEW {fast} CONFIG.PCW_MIO_26_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_26_PULLUP {enabled} CONFIG.PCW_MIO_26_SLEW {fast} CONFIG.PCW_MIO_27_IOTYPE {HSTL 1.8V} CONFIG.PCW_MIO_27_PULLUP {enabled} CONFIG.PCW_MIO_27_SLEW {fast} CONFIG.PCW_MIO_28_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_28_PULLUP {enabled} CONFIG.PCW_MIO_28_SLEW {fast} CONFIG.PCW_MIO_29_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_29_PULLUP {enabled} CONFIG.PCW_MIO_29_SLEW {fast} CONFIG.PCW_MIO_2_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_2_SLEW {fast} CONFIG.PCW_MIO_30_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_30_PULLUP {enabled} CONFIG.PCW_MIO_30_SLEW {fast} CONFIG.PCW_MIO_31_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_31_PULLUP {enabled} CONFIG.PCW_MIO_31_SLEW {fast} CONFIG.PCW_MIO_32_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_32_PULLUP {enabled} CONFIG.PCW_MIO_32_SLEW {fast} CONFIG.PCW_MIO_33_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_33_PULLUP {enabled} CONFIG.PCW_MIO_33_SLEW {fast} CONFIG.PCW_MIO_34_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_34_PULLUP {enabled} CONFIG.PCW_MIO_34_SLEW {fast} CONFIG.PCW_MIO_35_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_35_PULLUP {enabled} CONFIG.PCW_MIO_35_SLEW {fast} CONFIG.PCW_MIO_36_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_36_PULLUP {enabled} CONFIG.PCW_MIO_36_SLEW {fast} CONFIG.PCW_MIO_37_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_37_PULLUP {enabled} CONFIG.PCW_MIO_37_SLEW {fast} CONFIG.PCW_MIO_38_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_38_PULLUP {enabled} CONFIG.PCW_MIO_38_SLEW {fast} CONFIG.PCW_MIO_39_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_39_PULLUP {enabled} CONFIG.PCW_MIO_39_SLEW {fast} CONFIG.PCW_MIO_3_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_3_SLEW {fast} CONFIG.PCW_MIO_40_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_40_PULLUP {enabled} CONFIG.PCW_MIO_40_SLEW {fast} CONFIG.PCW_MIO_41_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_41_PULLUP {enabled} CONFIG.PCW_MIO_41_SLEW {fast} CONFIG.PCW_MIO_42_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_42_PULLUP {enabled} CONFIG.PCW_MIO_42_SLEW {fast} CONFIG.PCW_MIO_43_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_43_PULLUP {enabled} CONFIG.PCW_MIO_43_SLEW {fast} CONFIG.PCW_MIO_44_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_44_PULLUP {enabled} CONFIG.PCW_MIO_44_SLEW {fast} CONFIG.PCW_MIO_45_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_45_PULLUP {enabled} CONFIG.PCW_MIO_45_SLEW {fast} CONFIG.PCW_MIO_46_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_46_PULLUP {enabled} CONFIG.PCW_MIO_46_SLEW {slow} CONFIG.PCW_MIO_47_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_47_PULLUP {enabled} CONFIG.PCW_MIO_47_SLEW {slow} CONFIG.PCW_MIO_48_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_48_PULLUP {enabled} CONFIG.PCW_MIO_48_SLEW {slow} CONFIG.PCW_MIO_49_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_49_PULLUP {enabled} CONFIG.PCW_MIO_49_SLEW {slow} CONFIG.PCW_MIO_4_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_4_SLEW {fast} CONFIG.PCW_MIO_50_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_50_PULLUP {enabled} CONFIG.PCW_MIO_50_SLEW {slow} CONFIG.PCW_MIO_51_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_51_PULLUP {enabled} CONFIG.PCW_MIO_51_SLEW {slow} CONFIG.PCW_MIO_52_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_52_PULLUP {enabled} CONFIG.PCW_MIO_52_SLEW {slow} CONFIG.PCW_MIO_53_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_53_PULLUP {enabled} CONFIG.PCW_MIO_53_SLEW {slow} CONFIG.PCW_MIO_5_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_5_SLEW {fast} CONFIG.PCW_MIO_6_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_6_SLEW {fast} CONFIG.PCW_MIO_7_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_7_SLEW {slow} CONFIG.PCW_MIO_8_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_8_SLEW {slow} CONFIG.PCW_MIO_9_DIRECTION {inout} CONFIG.PCW_MIO_9_IOTYPE {LVCMOS 1.8V} CONFIG.PCW_MIO_9_PULLUP {enabled} CONFIG.PCW_MIO_9_SLEW {slow} CONFIG.PCW_MIO_TREE_PERIPHERALS {SD 0#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#USB Reset#Quad SPI Flash#GPIO#GPIO#ENET Reset#GPIO#I2C Reset#GPIO#SD 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#SD 0#SD 0#SD 0#SD 0#SD 0#SD 0#CAN 0#CAN 0#UART 1#UART 1#I2C 0#I2C 0#Enet 0#Enet 0} CONFIG.PCW_MIO_TREE_SIGNALS {cd#qspi0_ss_b#qspi0_io[0]#qspi0_io[1]#qspi0_io[2]#qspi0_io[3]#qspi0_sclk#reset#qspi_fbclk#gpio[9]#gpio[10]#reset#gpio[12]#reset#gpio[14]#wp#tx_clk#txd[0]#txd[1]#txd[2]#txd[3]#tx_ctl#rx_clk#rxd[0]#rxd[1]#rxd[2]#rxd[3]#rx_ctl#data[4]#dir#stp#nxt#data[0]#data[1]#data[2]#data[3]#clk#data[5]#data[6]#data[7]#clk#cmd#data[0]#data[1]#data[2]#data[3]#rx#tx#tx#rx#scl#sda#mdc#mdio} CONFIG.PCW_M_AXI_GP0_FREQMHZ {100} CONFIG.PCW_NAND_CYCLES_T_AR {0} CONFIG.PCW_NAND_CYCLES_T_CLR {0} CONFIG.PCW_NAND_CYCLES_T_RC {2} CONFIG.PCW_NAND_CYCLES_T_REA {1} CONFIG.PCW_NAND_CYCLES_T_RR {0} CONFIG.PCW_NAND_CYCLES_T_WC {2} CONFIG.PCW_NAND_CYCLES_T_WP {1} CONFIG.PCW_NOR_CS0_T_CEOE {1} CONFIG.PCW_NOR_CS0_T_PC {1} CONFIG.PCW_NOR_CS0_T_RC {2} CONFIG.PCW_NOR_CS0_T_TR {1} CONFIG.PCW_NOR_CS0_T_WC {2} CONFIG.PCW_NOR_CS0_T_WP {1} CONFIG.PCW_NOR_CS0_WE_TIME {2} CONFIG.PCW_NOR_CS1_T_CEOE {1} CONFIG.PCW_NOR_CS1_T_PC {1} CONFIG.PCW_NOR_CS1_T_RC {2} CONFIG.PCW_NOR_CS1_T_TR {1} CONFIG.PCW_NOR_CS1_T_WC {2} CONFIG.PCW_NOR_CS1_T_WP {1} CONFIG.PCW_NOR_CS1_WE_TIME {2} CONFIG.PCW_NOR_SRAM_CS0_T_CEOE {1} CONFIG.PCW_NOR_SRAM_CS0_T_PC {1} CONFIG.PCW_NOR_SRAM_CS0_T_RC {2} CONFIG.PCW_NOR_SRAM_CS0_T_TR {1} CONFIG.PCW_NOR_SRAM_CS0_T_WC {2} CONFIG.PCW_NOR_SRAM_CS0_T_WP {1} CONFIG.PCW_NOR_SRAM_CS0_WE_TIME {2} CONFIG.PCW_NOR_SRAM_CS1_T_CEOE {1} CONFIG.PCW_NOR_SRAM_CS1_T_PC {1} CONFIG.PCW_NOR_SRAM_CS1_T_RC {2} CONFIG.PCW_NOR_SRAM_CS1_T_TR {1} CONFIG.PCW_NOR_SRAM_CS1_T_WC {2} CONFIG.PCW_NOR_SRAM_CS1_T_WP {1} CONFIG.PCW_NOR_SRAM_CS1_WE_TIME {2} CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY0 {0.010} CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY1 {0.010} CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY2 {0.010} CONFIG.PCW_PACKAGE_DDR_BOARD_DELAY3 {0.013} CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_0 {-0.001} CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_1 {-0.002} CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_2 {-0.001} CONFIG.PCW_PACKAGE_DDR_DQS_TO_CLK_DELAY_3 {-0.008} CONFIG.PCW_PCAP_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_PCAP_PERIPHERAL_FREQMHZ {200} CONFIG.PCW_PERIPHERAL_BOARD_PRESET {zc702} CONFIG.PCW_PJTAG_PERIPHERAL_ENABLE {0} CONFIG.PCW_PRESET_BANK0_VOLTAGE {LVCMOS 1.8V} CONFIG.PCW_PRESET_BANK1_VOLTAGE {LVCMOS 1.8V} CONFIG.PCW_QSPI_GRP_FBCLK_ENABLE {1} CONFIG.PCW_QSPI_GRP_FBCLK_IO {MIO 8} CONFIG.PCW_QSPI_GRP_IO1_ENABLE {0} CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {1} CONFIG.PCW_QSPI_GRP_SINGLE_SS_IO {MIO 1 .. 6} CONFIG.PCW_QSPI_GRP_SS1_ENABLE {0} CONFIG.PCW_QSPI_PERIPHERAL_CLKSRC {ARM PLL} CONFIG.PCW_QSPI_PERIPHERAL_ENABLE {1} CONFIG.PCW_QSPI_PERIPHERAL_FREQMHZ {200} CONFIG.PCW_QSPI_QSPI_IO {MIO 1 .. 6} CONFIG.PCW_SD0_GRP_CD_ENABLE {1} CONFIG.PCW_SD0_GRP_CD_IO {MIO 0} CONFIG.PCW_SD0_GRP_POW_ENABLE {0} CONFIG.PCW_SD0_GRP_WP_ENABLE {1} CONFIG.PCW_SD0_GRP_WP_IO {MIO 15} CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} CONFIG.PCW_SD0_SD0_IO {MIO 40 .. 45} CONFIG.PCW_SD1_PERIPHERAL_ENABLE {0} CONFIG.PCW_SDIO_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_SDIO_PERIPHERAL_FREQMHZ {50} CONFIG.PCW_SDIO_PERIPHERAL_VALID {1} CONFIG.PCW_SMC_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_SPI0_PERIPHERAL_ENABLE {0} CONFIG.PCW_SPI1_PERIPHERAL_ENABLE {0} CONFIG.PCW_SPI_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_S_AXI_ACP_FREQMHZ {100} CONFIG.PCW_S_AXI_HP0_DATA_WIDTH {64} CONFIG.PCW_S_AXI_HP0_FREQMHZ {100} CONFIG.PCW_S_AXI_HP1_DATA_WIDTH {64} CONFIG.PCW_S_AXI_HP1_FREQMHZ {100} CONFIG.PCW_S_AXI_HP2_DATA_WIDTH {64} CONFIG.PCW_S_AXI_HP2_FREQMHZ {100} CONFIG.PCW_S_AXI_HP3_DATA_WIDTH {64} CONFIG.PCW_S_AXI_HP3_FREQMHZ {100} CONFIG.PCW_TPIU_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_TRACE_PERIPHERAL_ENABLE {0} CONFIG.PCW_TTC0_CLK0_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_TTC0_CLK0_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_TTC0_CLK1_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_TTC0_CLK1_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_TTC0_CLK2_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_TTC0_CLK2_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_TTC0_PERIPHERAL_ENABLE {0} CONFIG.PCW_TTC1_CLK0_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_TTC1_CLK0_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_TTC1_CLK1_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_TTC1_CLK1_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_TTC1_CLK2_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_TTC1_CLK2_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_TTC1_PERIPHERAL_ENABLE {0} CONFIG.PCW_UART0_PERIPHERAL_ENABLE {0} CONFIG.PCW_UART1_BAUD_RATE {115200} CONFIG.PCW_UART1_GRP_FULL_ENABLE {0} CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} CONFIG.PCW_UART1_UART1_IO {MIO 48 .. 49} CONFIG.PCW_UART_PERIPHERAL_CLKSRC {IO PLL} CONFIG.PCW_UART_PERIPHERAL_FREQMHZ {50} CONFIG.PCW_UIPARAM_DDR_ADV_ENABLE {0} CONFIG.PCW_UIPARAM_DDR_AL {0} CONFIG.PCW_UIPARAM_DDR_BL {8} CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY0 {0.537} CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY1 {0.442} CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY2 {0.464} CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY3 {0.521} CONFIG.PCW_UIPARAM_DDR_BUS_WIDTH {32 Bit} CONFIG.PCW_UIPARAM_DDR_CLOCK_0_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_CLOCK_0_PACKAGE_LENGTH {61.0905} CONFIG.PCW_UIPARAM_DDR_CLOCK_0_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_CLOCK_1_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_CLOCK_1_PACKAGE_LENGTH {61.0905} CONFIG.PCW_UIPARAM_DDR_CLOCK_1_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_CLOCK_2_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_CLOCK_2_PACKAGE_LENGTH {61.0905} CONFIG.PCW_UIPARAM_DDR_CLOCK_2_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_CLOCK_3_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_CLOCK_3_PACKAGE_LENGTH {61.0905} CONFIG.PCW_UIPARAM_DDR_CLOCK_3_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_CLOCK_STOP_EN {0} CONFIG.PCW_UIPARAM_DDR_DQS_0_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQS_0_PACKAGE_LENGTH {68.4725} CONFIG.PCW_UIPARAM_DDR_DQS_0_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQS_1_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQS_1_PACKAGE_LENGTH {71.086} CONFIG.PCW_UIPARAM_DDR_DQS_1_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQS_2_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQS_2_PACKAGE_LENGTH {66.794} CONFIG.PCW_UIPARAM_DDR_DQS_2_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQS_3_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQS_3_PACKAGE_LENGTH {108.7385} CONFIG.PCW_UIPARAM_DDR_DQS_3_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_0 {0.217} CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_1 {0.133} CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_2 {0.089} CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_3 {0.248} CONFIG.PCW_UIPARAM_DDR_DQ_0_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQ_0_PACKAGE_LENGTH {64.1705} CONFIG.PCW_UIPARAM_DDR_DQ_0_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQ_1_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQ_1_PACKAGE_LENGTH {63.686} CONFIG.PCW_UIPARAM_DDR_DQ_1_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQ_2_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQ_2_PACKAGE_LENGTH {68.46} CONFIG.PCW_UIPARAM_DDR_DQ_2_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_DQ_3_LENGTH_MM {0} CONFIG.PCW_UIPARAM_DDR_DQ_3_PACKAGE_LENGTH {105.4895} CONFIG.PCW_UIPARAM_DDR_DQ_3_PROPOGATION_DELAY {160} CONFIG.PCW_UIPARAM_DDR_ENABLE {1} CONFIG.PCW_UIPARAM_DDR_FREQ_MHZ {533.333333} CONFIG.PCW_UIPARAM_DDR_HIGH_TEMP {Normal (0-85)} CONFIG.PCW_UIPARAM_DDR_MEMORY_TYPE {DDR 3} CONFIG.PCW_UIPARAM_DDR_PARTNO {MT41J256M8 HX-15E} CONFIG.PCW_UIPARAM_DDR_TRAIN_DATA_EYE {1} CONFIG.PCW_UIPARAM_DDR_TRAIN_READ_GATE {1} CONFIG.PCW_UIPARAM_DDR_TRAIN_WRITE_LEVEL {1} CONFIG.PCW_UIPARAM_DDR_USE_INTERNAL_VREF {1} CONFIG.PCW_USB0_PERIPHERAL_ENABLE {1} CONFIG.PCW_USB0_RESET_ENABLE {1} CONFIG.PCW_USB0_RESET_IO {MIO 7} CONFIG.PCW_USB0_USB0_IO {MIO 28 .. 39} CONFIG.PCW_USB1_PERIPHERAL_ENABLE {0} CONFIG.PCW_USB1_RESET_ENABLE {0} CONFIG.PCW_USB1_RESET_IO {<Select>} CONFIG.PCW_USB_RESET_POLARITY {Active Low} CONFIG.PCW_USE_CROSS_TRIGGER {0} CONFIG.PCW_USE_S_AXI_ACP {1} CONFIG.PCW_USE_S_AXI_HP0 {1} CONFIG.PCW_USE_S_AXI_HP1 {1} CONFIG.PCW_USE_S_AXI_HP2 {1} CONFIG.PCW_USE_S_AXI_HP3 {1} CONFIG.PCW_WDT_PERIPHERAL_CLKSRC {CPU_1X} CONFIG.PCW_WDT_PERIPHERAL_DIVISOR0 {1} CONFIG.PCW_WDT_PERIPHERAL_ENABLE {0}  ] $processing_system7_1

  # Create instance: xilmonitor_apm, and set properties
  set xilmonitor_apm [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_perf_mon:5.0 xilmonitor_apm ]
  set_property -dict [ list CONFIG.C_ENABLE_PROFILE {1} CONFIG.C_ENABLE_TRACE {1} CONFIG.C_EN_EXT_EVENTS_FLAG {0} CONFIG.C_EN_FIRST_READ_FLAG {0} CONFIG.C_EN_FIRST_WRITE_FLAG {0} CONFIG.C_EN_LAST_READ_FLAG {1} CONFIG.C_EN_LAST_WRITE_FLAG {1} CONFIG.C_EN_RD_ADD_FLAG {1} CONFIG.C_EN_RESPONSE_FLAG {0} CONFIG.C_EN_SW_REG_WR_FLAG {1} CONFIG.C_EN_WR_ADD_FLAG {1} CONFIG.C_FIFO_AXIS_DEPTH {16} CONFIG.C_HAVE_SAMPLED_METRIC_CNT {1} CONFIG.C_NUM_MONITOR_SLOTS {5} CONFIG.C_SHOW_AXI_IDS {0} CONFIG.C_SHOW_AXI_LEN {0} CONFIG.ENABLE_EXT_EVENTS {1} CONFIG.ENABLE_EXT_TRIGGERS {1}  ] $xilmonitor_apm

  # Create instance: xilmonitor_broadcast, and set properties
  set xilmonitor_broadcast [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_broadcaster:1.1 xilmonitor_broadcast ]
  set_property -dict [ list CONFIG.M00_TDATA_REMAP {24'b000000000000000000000000,tdata[71:0]} CONFIG.M01_TDATA_REMAP {24'b000000000000000000000000,tdata[71:0]} CONFIG.M02_TDATA_REMAP {24'b000000000000000000000000,tdata[71:0]} CONFIG.M_TDATA_NUM_BYTES {12} CONFIG.NUM_MI {3} CONFIG.S_TDATA_NUM_BYTES {9} CONFIG.TID_WIDTH {1}  ] $xilmonitor_broadcast

  # Create instance: xilmonitor_fifo0, and set properties
  set xilmonitor_fifo0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_fifo_mm_s:4.1 xilmonitor_fifo0 ]
  set_property -dict [ list CONFIG.C_BASEADDR {0x49000000} CONFIG.C_DATA_INTERFACE_TYPE {0} CONFIG.C_HIGHADDR {0x497FFFFF} CONFIG.C_RX_FIFO_DEPTH {4096} CONFIG.C_USE_RX_CUT_THROUGH {true}  ] $xilmonitor_fifo0

  # Create instance: xilmonitor_fifo1, and set properties
  set xilmonitor_fifo1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_fifo_mm_s:4.1 xilmonitor_fifo1 ]
  set_property -dict [ list CONFIG.C_BASEADDR {0x4A000000} CONFIG.C_DATA_INTERFACE_TYPE {0} CONFIG.C_HIGHADDR {0x4A7FFFFF} CONFIG.C_RX_FIFO_DEPTH {4096} CONFIG.C_USE_RX_CUT_THROUGH {true}  ] $xilmonitor_fifo1

  # Create instance: xilmonitor_fifo2, and set properties
  set xilmonitor_fifo2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_fifo_mm_s:4.1 xilmonitor_fifo2 ]
  set_property -dict [ list CONFIG.C_BASEADDR {0x4B000000} CONFIG.C_DATA_INTERFACE_TYPE {0} CONFIG.C_HIGHADDR {0x4B7FFFFF} CONFIG.C_RX_FIFO_DEPTH {4096} CONFIG.C_USE_RX_CUT_THROUGH {true}  ] $xilmonitor_fifo2

  # Create instance: xilmonitor_subset0, and set properties
  set xilmonitor_subset0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_subset_converter:1.1 xilmonitor_subset0 ]
  set_property -dict [ list CONFIG.M_TDATA_NUM_BYTES {4} CONFIG.M_TID_WIDTH {1} CONFIG.S_TDATA_NUM_BYTES {12} CONFIG.S_TID_WIDTH {1} CONFIG.TDATA_REMAP {tdata[31:0]}  ] $xilmonitor_subset0

  # Create instance: xilmonitor_subset1, and set properties
  set xilmonitor_subset1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_subset_converter:1.1 xilmonitor_subset1 ]
  set_property -dict [ list CONFIG.M_TDATA_NUM_BYTES {4} CONFIG.M_TID_WIDTH {1} CONFIG.S_TDATA_NUM_BYTES {12} CONFIG.S_TID_WIDTH {1} CONFIG.TDATA_REMAP {tdata[63:32]}  ] $xilmonitor_subset1

  # Create instance: xilmonitor_subset2, and set properties
  set xilmonitor_subset2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_subset_converter:1.1 xilmonitor_subset2 ]
  set_property -dict [ list CONFIG.M_TDATA_NUM_BYTES {4} CONFIG.M_TID_WIDTH {1} CONFIG.S_TDATA_NUM_BYTES {12} CONFIG.S_TID_WIDTH {1} CONFIG.TDATA_REMAP {tdata[95:64]}  ] $xilmonitor_subset2

  # Create interface connections
  connect_bd_intf_net -intf_net atg_hp0_m_axi [get_bd_intf_pins atg_hp0/M_AXI] [get_bd_intf_pins axi_mem_intercon_2/S00_AXI]
connect_bd_intf_net -intf_net atg_hp0_m_axi [get_bd_intf_pins atg_hp0/M_AXI] [get_bd_intf_pins xilmonitor_apm/SLOT_0_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_1_m00_axi [get_bd_intf_pins axi_mem_intercon_1/M00_AXI] [get_bd_intf_pins processing_system7_1/S_AXI_ACP]
  connect_bd_intf_net -intf_net axi_mem_intercon_2_m00_axi [get_bd_intf_pins axi_mem_intercon_2/M00_AXI] [get_bd_intf_pins processing_system7_1/S_AXI_HP0]
  connect_bd_intf_net -intf_net axi_mem_intercon_3_m00_axi [get_bd_intf_pins axi_mem_intercon_3/M00_AXI] [get_bd_intf_pins processing_system7_1/S_AXI_HP1]
  connect_bd_intf_net -intf_net axi_mem_intercon_4_m00_axi [get_bd_intf_pins axi_mem_intercon_4/M00_AXI] [get_bd_intf_pins processing_system7_1/S_AXI_HP2]
  connect_bd_intf_net -intf_net axi_mem_intercon_5_m00_axi [get_bd_intf_pins axi_mem_intercon_5/M00_AXI] [get_bd_intf_pins processing_system7_1/S_AXI_HP3]
  connect_bd_intf_net -intf_net axi_mem_intercon_M05_AXI [get_bd_intf_pins axi_mem_intercon/M05_AXI] [get_bd_intf_pins xilmonitor_apm/S_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M06_AXI [get_bd_intf_pins axi_mem_intercon/M06_AXI] [get_bd_intf_pins xilmonitor_fifo0/S_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M07_AXI [get_bd_intf_pins axi_mem_intercon/M07_AXI] [get_bd_intf_pins xilmonitor_fifo1/S_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M08_AXI [get_bd_intf_pins axi_mem_intercon/M08_AXI] [get_bd_intf_pins xilmonitor_fifo2/S_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_M09_AXI [get_bd_intf_pins axi_gpio_0/S_AXI] [get_bd_intf_pins axi_mem_intercon/M09_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_m00_axi [get_bd_intf_pins atg_hp0/S_AXI] [get_bd_intf_pins axi_mem_intercon/M00_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_m01_axi [get_bd_intf_pins atg_hp1/S_AXI] [get_bd_intf_pins axi_mem_intercon/M01_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_m02_axi [get_bd_intf_pins atg_hp2/S_AXI] [get_bd_intf_pins axi_mem_intercon/M02_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_m03_axi [get_bd_intf_pins atg_hp3/S_AXI] [get_bd_intf_pins axi_mem_intercon/M03_AXI]
  connect_bd_intf_net -intf_net axi_mem_intercon_m04_axi [get_bd_intf_pins atg_acp/S_AXI] [get_bd_intf_pins axi_mem_intercon/M04_AXI]
  connect_bd_intf_net -intf_net axi_traffic_gen_1_axi_master [get_bd_intf_pins atg_acp/M_AXI] [get_bd_intf_pins axi_mem_intercon_1/S00_AXI]
connect_bd_intf_net -intf_net axi_traffic_gen_1_axi_master [get_bd_intf_pins atg_acp/M_AXI] [get_bd_intf_pins xilmonitor_apm/SLOT_4_AXI]
  connect_bd_intf_net -intf_net axi_traffic_gen_3_axi_master [get_bd_intf_pins atg_hp1/M_AXI] [get_bd_intf_pins axi_mem_intercon_3/S00_AXI]
connect_bd_intf_net -intf_net axi_traffic_gen_3_axi_master [get_bd_intf_pins atg_hp1/M_AXI] [get_bd_intf_pins xilmonitor_apm/SLOT_1_AXI]
  connect_bd_intf_net -intf_net axi_traffic_gen_4_axi_master [get_bd_intf_pins atg_hp2/M_AXI] [get_bd_intf_pins axi_mem_intercon_4/S00_AXI]
connect_bd_intf_net -intf_net axi_traffic_gen_4_axi_master [get_bd_intf_pins atg_hp2/M_AXI] [get_bd_intf_pins xilmonitor_apm/SLOT_2_AXI]
  connect_bd_intf_net -intf_net axi_traffic_gen_5_axi_master [get_bd_intf_pins atg_hp3/M_AXI] [get_bd_intf_pins axi_mem_intercon_5/S00_AXI]
connect_bd_intf_net -intf_net axi_traffic_gen_5_axi_master [get_bd_intf_pins atg_hp3/M_AXI] [get_bd_intf_pins xilmonitor_apm/SLOT_3_AXI]
  connect_bd_intf_net -intf_net processing_system7_1_M_AXI_GP0 [get_bd_intf_pins axi_mem_intercon/S00_AXI] [get_bd_intf_pins processing_system7_1/M_AXI_GP0]
  connect_bd_intf_net -intf_net processing_system7_1_ddr [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7_1/DDR]
  connect_bd_intf_net -intf_net processing_system7_1_fixed_io [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7_1/FIXED_IO]
  connect_bd_intf_net -intf_net xilmonitor_apm_M_AXIS [get_bd_intf_pins xilmonitor_apm/M_AXIS] [get_bd_intf_pins xilmonitor_broadcast/S_AXIS]
  connect_bd_intf_net -intf_net xilmonitor_broadcast_M00_AXIS [get_bd_intf_pins xilmonitor_broadcast/M00_AXIS] [get_bd_intf_pins xilmonitor_subset0/S_AXIS]
  connect_bd_intf_net -intf_net xilmonitor_broadcast_M01_AXIS [get_bd_intf_pins xilmonitor_broadcast/M01_AXIS] [get_bd_intf_pins xilmonitor_subset1/S_AXIS]
  connect_bd_intf_net -intf_net xilmonitor_broadcast_M02_AXIS [get_bd_intf_pins xilmonitor_broadcast/M02_AXIS] [get_bd_intf_pins xilmonitor_subset2/S_AXIS]
  connect_bd_intf_net -intf_net xilmonitor_subset0_M_AXIS [get_bd_intf_pins xilmonitor_fifo0/AXI_STR_RXD] [get_bd_intf_pins xilmonitor_subset0/M_AXIS]
  connect_bd_intf_net -intf_net xilmonitor_subset1_M_AXIS [get_bd_intf_pins xilmonitor_fifo1/AXI_STR_RXD] [get_bd_intf_pins xilmonitor_subset1/M_AXIS]
  connect_bd_intf_net -intf_net xilmonitor_subset2_M_AXIS [get_bd_intf_pins xilmonitor_fifo2/AXI_STR_RXD] [get_bd_intf_pins xilmonitor_subset2/M_AXIS]

  # Create port connections
  connect_bd_net -net axi_gpio_0_gpio_io_o [get_bd_pins atg_acp/core_ext_start] [get_bd_pins atg_hp0/core_ext_start] [get_bd_pins atg_hp1/core_ext_start] [get_bd_pins atg_hp2/core_ext_start] [get_bd_pins atg_hp3/core_ext_start] [get_bd_pins axi_gpio_0/gpio_io_o]
  connect_bd_net -net clk [get_bd_pins atg_acp/s_axi_aclk] [get_bd_pins atg_hp0/s_axi_aclk] [get_bd_pins atg_hp1/s_axi_aclk] [get_bd_pins atg_hp2/s_axi_aclk] [get_bd_pins atg_hp3/s_axi_aclk] [get_bd_pins axi_gpio_0/s_axi_aclk] [get_bd_pins axi_mem_intercon/ACLK] [get_bd_pins axi_mem_intercon/M00_ACLK] [get_bd_pins axi_mem_intercon/M01_ACLK] [get_bd_pins axi_mem_intercon/M02_ACLK] [get_bd_pins axi_mem_intercon/M03_ACLK] [get_bd_pins axi_mem_intercon/M04_ACLK] [get_bd_pins axi_mem_intercon/M05_ACLK] [get_bd_pins axi_mem_intercon/M06_ACLK] [get_bd_pins axi_mem_intercon/M07_ACLK] [get_bd_pins axi_mem_intercon/M08_ACLK] [get_bd_pins axi_mem_intercon/M09_ACLK] [get_bd_pins axi_mem_intercon/S00_ACLK] [get_bd_pins axi_mem_intercon_1/ACLK] [get_bd_pins axi_mem_intercon_1/M00_ACLK] [get_bd_pins axi_mem_intercon_1/S00_ACLK] [get_bd_pins axi_mem_intercon_2/ACLK] [get_bd_pins axi_mem_intercon_2/M00_ACLK] [get_bd_pins axi_mem_intercon_2/S00_ACLK] [get_bd_pins axi_mem_intercon_3/ACLK] [get_bd_pins axi_mem_intercon_3/M00_ACLK] [get_bd_pins axi_mem_intercon_3/S00_ACLK] [get_bd_pins axi_mem_intercon_4/ACLK] [get_bd_pins axi_mem_intercon_4/M00_ACLK] [get_bd_pins axi_mem_intercon_4/S00_ACLK] [get_bd_pins axi_mem_intercon_5/ACLK] [get_bd_pins axi_mem_intercon_5/M00_ACLK] [get_bd_pins axi_mem_intercon_5/S00_ACLK] [get_bd_pins proc_sys_reset/slowest_sync_clk] [get_bd_pins processing_system7_1/FCLK_CLK0] [get_bd_pins processing_system7_1/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_1/S_AXI_ACP_ACLK] [get_bd_pins processing_system7_1/S_AXI_HP0_ACLK] [get_bd_pins processing_system7_1/S_AXI_HP1_ACLK] [get_bd_pins processing_system7_1/S_AXI_HP2_ACLK] [get_bd_pins processing_system7_1/S_AXI_HP3_ACLK] [get_bd_pins xilmonitor_apm/core_aclk] [get_bd_pins xilmonitor_apm/m_axis_aclk] [get_bd_pins xilmonitor_apm/s_axi_aclk] [get_bd_pins xilmonitor_apm/slot_0_axi_aclk] [get_bd_pins xilmonitor_apm/slot_1_axi_aclk] [get_bd_pins xilmonitor_apm/slot_2_axi_aclk] [get_bd_pins xilmonitor_apm/slot_3_axi_aclk] [get_bd_pins xilmonitor_apm/slot_4_axi_aclk] [get_bd_pins xilmonitor_broadcast/aclk] [get_bd_pins xilmonitor_fifo0/s_axi_aclk] [get_bd_pins xilmonitor_fifo1/s_axi_aclk] [get_bd_pins xilmonitor_fifo2/s_axi_aclk] [get_bd_pins xilmonitor_subset0/aclk] [get_bd_pins xilmonitor_subset1/aclk] [get_bd_pins xilmonitor_subset2/aclk]
  connect_bd_net -net interconnect_resetn [get_bd_pins axi_mem_intercon/ARESETN] [get_bd_pins axi_mem_intercon_1/ARESETN] [get_bd_pins axi_mem_intercon_2/ARESETN] [get_bd_pins axi_mem_intercon_3/ARESETN] [get_bd_pins axi_mem_intercon_4/ARESETN] [get_bd_pins axi_mem_intercon_5/ARESETN] [get_bd_pins proc_sys_reset/interconnect_aresetn]
  connect_bd_net -net peripheral_resetn [get_bd_pins atg_acp/s_axi_aresetn] [get_bd_pins atg_hp0/s_axi_aresetn] [get_bd_pins atg_hp1/s_axi_aresetn] [get_bd_pins atg_hp2/s_axi_aresetn] [get_bd_pins atg_hp3/s_axi_aresetn] [get_bd_pins axi_gpio_0/s_axi_aresetn] [get_bd_pins axi_mem_intercon/M00_ARESETN] [get_bd_pins axi_mem_intercon/M01_ARESETN] [get_bd_pins axi_mem_intercon/M02_ARESETN] [get_bd_pins axi_mem_intercon/M03_ARESETN] [get_bd_pins axi_mem_intercon/M04_ARESETN] [get_bd_pins axi_mem_intercon/M05_ARESETN] [get_bd_pins axi_mem_intercon/M06_ARESETN] [get_bd_pins axi_mem_intercon/M07_ARESETN] [get_bd_pins axi_mem_intercon/M08_ARESETN] [get_bd_pins axi_mem_intercon/M09_ARESETN] [get_bd_pins axi_mem_intercon/S00_ARESETN] [get_bd_pins axi_mem_intercon_1/M00_ARESETN] [get_bd_pins axi_mem_intercon_1/S00_ARESETN] [get_bd_pins axi_mem_intercon_2/M00_ARESETN] [get_bd_pins axi_mem_intercon_2/S00_ARESETN] [get_bd_pins axi_mem_intercon_3/M00_ARESETN] [get_bd_pins axi_mem_intercon_3/S00_ARESETN] [get_bd_pins axi_mem_intercon_4/M00_ARESETN] [get_bd_pins axi_mem_intercon_4/S00_ARESETN] [get_bd_pins axi_mem_intercon_5/M00_ARESETN] [get_bd_pins axi_mem_intercon_5/S00_ARESETN] [get_bd_pins proc_sys_reset/peripheral_aresetn] [get_bd_pins xilmonitor_apm/core_aresetn] [get_bd_pins xilmonitor_apm/m_axis_aresetn] [get_bd_pins xilmonitor_apm/s_axi_aresetn] [get_bd_pins xilmonitor_apm/slot_0_axi_aresetn] [get_bd_pins xilmonitor_apm/slot_1_axi_aresetn] [get_bd_pins xilmonitor_apm/slot_2_axi_aresetn] [get_bd_pins xilmonitor_apm/slot_3_axi_aresetn] [get_bd_pins xilmonitor_apm/slot_4_axi_aresetn] [get_bd_pins xilmonitor_broadcast/aresetn] [get_bd_pins xilmonitor_fifo0/s_axi_aresetn] [get_bd_pins xilmonitor_fifo1/s_axi_aresetn] [get_bd_pins xilmonitor_fifo2/s_axi_aresetn] [get_bd_pins xilmonitor_subset0/aresetn] [get_bd_pins xilmonitor_subset1/aresetn] [get_bd_pins xilmonitor_subset2/aresetn]
  connect_bd_net -net processing_system7_1_FCLK_RESET0_N [get_bd_pins proc_sys_reset/ext_reset_in] [get_bd_pins processing_system7_1/FCLK_RESET0_N]

  # Create address segments
  create_bd_addr_seg -range 0x40000000 -offset 0x0 [get_bd_addr_spaces atg_acp/Data] [get_bd_addr_segs processing_system7_1/S_AXI_ACP/ACP_DDR_LOWOCM] SEG_processing_system7_1_ACP_DDR_LOWOCM
  create_bd_addr_seg -range 0x400000 -offset 0xE0000000 [get_bd_addr_spaces atg_acp/Data] [get_bd_addr_segs processing_system7_1/S_AXI_ACP/ACP_IOP] SEG_processing_system7_1_ACP_IOP
  create_bd_addr_seg -range 0x40000000 -offset 0x40000000 [get_bd_addr_spaces atg_acp/Data] [get_bd_addr_segs processing_system7_1/S_AXI_ACP/ACP_M_AXI_GP0] SEG_processing_system7_1_ACP_M_AXI_GP0
  create_bd_addr_seg -range 0x1000000 -offset 0xFC000000 [get_bd_addr_spaces atg_acp/Data] [get_bd_addr_segs processing_system7_1/S_AXI_ACP/ACP_QSPI_LINEAR] SEG_processing_system7_1_ACP_QSPI_LINEAR
  create_bd_addr_seg -range 0x40000000 -offset 0x0 [get_bd_addr_spaces atg_hp0/Data] [get_bd_addr_segs processing_system7_1/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_1_HP0_DDR_LOWOCM
  create_bd_addr_seg -range 0x40000000 -offset 0x0 [get_bd_addr_spaces atg_hp1/Data] [get_bd_addr_segs processing_system7_1/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_1_HP1_DDR_LOWOCM
  create_bd_addr_seg -range 0x40000000 -offset 0x0 [get_bd_addr_spaces atg_hp2/Data] [get_bd_addr_segs processing_system7_1/S_AXI_HP2/HP2_DDR_LOWOCM] SEG_processing_system7_1_HP2_DDR_LOWOCM
  create_bd_addr_seg -range 0x40000000 -offset 0x0 [get_bd_addr_spaces atg_hp3/Data] [get_bd_addr_segs processing_system7_1/S_AXI_HP3/HP3_DDR_LOWOCM] SEG_processing_system7_1_HP3_DDR_LOWOCM
  create_bd_addr_seg -range 0x800000 -offset 0x45000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs atg_acp/S_AXI/Reg0] SEG_atg_acp_Reg0
  create_bd_addr_seg -range 0x800000 -offset 0x41000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs atg_hp0/S_AXI/Reg0] SEG_atg_hp0_Reg0
  create_bd_addr_seg -range 0x800000 -offset 0x42000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs atg_hp1/S_AXI/Reg0] SEG_atg_hp1_Reg0
  create_bd_addr_seg -range 0x800000 -offset 0x43000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs atg_hp2/S_AXI/Reg0] SEG_atg_hp2_Reg0
  create_bd_addr_seg -range 0x800000 -offset 0x44000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs atg_hp3/S_AXI/Reg0] SEG_atg_hp3_Reg0
  create_bd_addr_seg -range 0x10000 -offset 0x46000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] SEG_axi_gpio_0_Reg
  create_bd_addr_seg -range 0x800000 -offset 0x48000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs xilmonitor_apm/S_AXI/Reg] SEG_xilmonitor_apm_Reg
  create_bd_addr_seg -range 0x800000 -offset 0x49000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs xilmonitor_fifo0/S_AXI/Mem0] SEG_xilmonitor_fifo0_Mem0
  create_bd_addr_seg -range 0x800000 -offset 0x4A000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs xilmonitor_fifo1/S_AXI/Mem0] SEG_xilmonitor_fifo1_Mem0
  create_bd_addr_seg -range 0x800000 -offset 0x4B000000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs xilmonitor_fifo2/S_AXI/Mem0] SEG_xilmonitor_fifo2_Mem0
  

  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


