# record the zynq basic tcl scripts.
#

set proj_name "zynq_hw"
set proj_bd_design "zynq_basic"

# WARNING:
# please modify your Zynq IC and BDF accordingly
# for example, if you are using Microzed 7Z010, then
# you should change part to xc7z010clg400-1 and bdf file to microzed_7010
#
create_project $proj_name "./" -part xc7z020clg400-1
set_property board_part em.avnet.com:microzed_7020:part0:1.1 [current_project]
set_property target_language VHDL [current_project]

# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# create BD
create_bd_design $proj_bd_design

# add zynq
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
endgroup

# zynq automation
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]

# config Zynq : PL clock 100MHz, enable ethernet-0, enable MGP0, enable interrupt.
# Just clicj the gray box on ethernet-0 if eth0 is not properly bring up.
startgroup
set_property -dict [list CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {100} CONFIG.PCW_USE_FABRIC_INTERRUPT {1} CONFIG.PCW_IRQ_F2P_INTR {1}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {1}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {1} CONFIG.PCW_ENET0_ENET0_IO {MIO 16 .. 27}] [get_bd_cells processing_system7_0]
endgroup

# add AXI Timer, Concat
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_0
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0
endgroup

startgroup
set_property -dict [list CONFIG.enable_timer2 {0}] [get_bd_cells axi_timer_0]
set_property -dict [list CONFIG.NUM_PORTS {1}] [get_bd_cells xlconcat_0]
endgroup

# connect automation
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" }  [get_bd_intf_pins axi_timer_0/S_AXI]
connect_bd_net [get_bd_pins axi_timer_0/interrupt] [get_bd_pins xlconcat_0/In0]
connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins processing_system7_0/IRQ_F2P]

# that's all for this basic demo.
#
regenerate_bd_layout
validate_bd_design
save_bd_design

# make wrappers and generate output products.
#
make_wrapper -files [get_files "$proj_dir/${proj_name}.srcs/sources_1/bd/$proj_bd_design/$proj_bd_design.bd"] -top
add_files -norecurse "$proj_dir/${proj_name}.srcs/sources_1/bd/$proj_bd_design/hdl/${proj_bd_design}_wrapper.vhd"
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
generate_target all [get_files  "$proj_dir/${proj_name}.srcs/sources_1/bd/$proj_bd_design/$proj_bd_design.bd"]

# if you do not need to modify this design, simply
# reset_run synth_1
# launch_runs impl_1 -to_step write_bitstream

