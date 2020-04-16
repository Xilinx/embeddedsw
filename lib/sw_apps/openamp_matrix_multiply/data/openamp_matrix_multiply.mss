#/******************************************************************************
#* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = (standalone freertos10_xilinx)
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = openamp
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = libmetal
END
