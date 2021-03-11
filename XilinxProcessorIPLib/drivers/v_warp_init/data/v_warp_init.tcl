# ==============================================================
# Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
# Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
# ==============================================================
proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_warp_init" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
	"MAX_COLS" \
	"MAX_ROWS" \
	"WARP_TYPE" \
	"AXI_MM_DATA_WIDTH" \
	"BITS_PER_COMPONENT" \
	"MAX_CTRL_PTS_R"

    xdefine_config_file $drv_handle "xv_warp_init_g.c" "XV_warp_init" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
	"MAX_COLS" \
	"MAX_ROWS" \
	"WARP_TYPE" \
	"AXI_MM_DATA_WIDTH" \
	"BITS_PER_COMPONENT" \
	"MAX_CTRL_PTS_R"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_warp_init" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
	"MAX_COLS" \
	"MAX_ROWS" \
	"WARP_TYPE" \
	"AXI_MM_DATA_WIDTH" \
	"BITS_PER_COMPONENT" \
	"MAX_CTRL_PTS_R"
}
