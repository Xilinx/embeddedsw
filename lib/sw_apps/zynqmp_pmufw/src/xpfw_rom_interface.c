/*
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#include "xpfw_rom_interface.h"

__attribute__((used, section(".xpbr_serv_ext_tbl")))
	XpbrServExtHndlr_t XpbrServExtTbl[XPBR_SERV_EXT_TBL_MAX];
