# ==============================================================
# Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2020.1 (64-bit)
# Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# ==============================================================
proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_warp_filter" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
	"MAX_COLS" \
	"MAX_ROWS" \
	"AXI_MM_READ_DATA_WIDTH" \
	"PERFORMANCE_LEVEL" \
	"BITS_PER_COMPONENT"

    xdefine_config_file $drv_handle "xv_warp_filter_g.c" "XV_warp_filter" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
	"MAX_COLS" \
	"MAX_ROWS" \
	"AXI_MM_READ_DATA_WIDTH" \
	"PERFORMANCE_LEVEL" \
	"BITS_PER_COMPONENT"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_warp_filter" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
	"MAX_COLS" \
	"MAX_ROWS" \
	"AXI_MM_READ_DATA_WIDTH" \
	"PERFORMANCE_LEVEL" \
	"BITS_PER_COMPONENT"
}
