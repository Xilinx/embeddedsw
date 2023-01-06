#/******************************************************************************
#* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

 PARAMETER VERSION = 2.2.0

BEGIN OS
 PARAMETER OS_NAME = freertos10_xilinx
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
 PARAMETER total_heap_size = 262140
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = lwip213
 PARAMETER API_MODE = SOCKET_API
 PARAMETER dhcp_does_arp_check = true
 PARAMETER lwip_dhcp = true
 PARAMETER mem_size = 524288
 PARAMETER memp_n_pbuf = 1024
 PARAMETER memp_n_tcp_seg = 1024
 PARAMETER n_rx_descriptors = 512
 PARAMETER n_tx_descriptors = 512
 PARAMETER pbuf_pool_size = 16384
 PARAMETER tcp_snd_buf = 65535
 PARAMETER tcp_wnd = 65535
 PARAMETER ipv6_enable = false
END

