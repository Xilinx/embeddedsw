# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# DisplayPort live-output support sources for the VISP SS example.
# Datapath: ISP -> frmbuf_wr -> DDR -> frmbuf_rd -> VTC -> DC (live) -> DP TX
#
# Self-guarded: only compiled when the design provides the DP/DC/clk_wiz IPs
# (DP_DISPLAY_AVAILABLE is set in test.cmake from xparameters.h), so the same
# example also builds on ISP-only / HDMI designs that lack these drivers.
if(DP_DISPLAY_AVAILABLE)
    collect (PROJECT_LIB_HEADERS dp_display_bridge.h)
    collect (PROJECT_LIB_HEADERS mmi_dpdc_example.h)
    collect (PROJECT_LIB_HEADERS mmi_dpdc_platform.h)
    collect (PROJECT_LIB_HEADERS mmi_dc_nonlive_test.h)
    collect (PROJECT_LIB_HEADERS mmi_dp_init.h)
    collect (PROJECT_LIB_HEADERS mmi_dp_intr.h)

    collect (PROJECT_LIB_SOURCES dp_display_bridge.c)
    collect (PROJECT_LIB_SOURCES mmi_dp_init.c)
    collect (PROJECT_LIB_SOURCES mmi_dp_intr.c)
endif()
