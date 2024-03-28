#/******************************************************************************
#* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

 PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = standalone
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = lwip220
 PARAMETER API_MODE = RAW_API
 PARAMETER lwip_dhcp_does_acd_check = true
 PARAMETER lwip_dhcp = true
 PARAMETER ipv6_enable = false
 PARAMETER pbuf_pool_size = 2048
END

