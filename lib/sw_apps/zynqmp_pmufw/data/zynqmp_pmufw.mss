#/******************************************************************************
#* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = standalone
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilfpga
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilsecure
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilskey
END

