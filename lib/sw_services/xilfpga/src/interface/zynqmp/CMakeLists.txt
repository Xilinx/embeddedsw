# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
if (${zynqmp_secure_env})
    collect (PROJECT_LIB_SOURCES xilfpga_ipi_pcap.c)
else()
    collect (PROJECT_LIB_SOURCES xilfpga_pcap.c)
    collect (PROJECT_LIB_HEADERS xilfpga_pcap.h)
endif()
collect (PROJECT_LIB_HEADERS xilfpga_pcap_common.h)
