#/******************************************************************************
#* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = standalone
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
 PARAMETER ZYNQMP_FSBL_BSP = true
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilffs
 PARAMETER READ_ONLY = true
 PARAMETER USE_MKFS = false
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilsecure
 PARAMETER TPM_SUPPORT = true
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilpm
END

