#/******************************************************************************
#* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
#* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = standalone
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
 PARAMETER IMG_RCVRY_BSP = true
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = lwip213
 PARAMETER API_MODE = RAW_API
 PARAMETER ipv6_enable = false
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilffs
 PARAMETER fs_interface = 2
 PARAMETER ramfs_size = 1048576
 PARAMETER ramfs_start_addr = 0x10000000
END
