#/******************************************************************************
#* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

 PARAMETER VERSION = 2.2.0

BEGIN OS
 PARAMETER OS_NAME = freertos10_xilinx
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
 PARAMETER SYSTMR_SPEC = true
 PARAMETER SYSTMR_DEV = *
 PARAMETER SYSINTC_SPEC = *
 PARAMETER XIL_INTERRUPT = true
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xiltimer
 PARAMETER en_interval_timer = true
END
