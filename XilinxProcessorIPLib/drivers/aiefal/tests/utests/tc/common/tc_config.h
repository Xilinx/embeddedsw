// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include <xaiengine.h>

#if AIE_GEN == 2

#define HW_GEN XAIE_DEV_GEN_AIE2
#define XAIE_NUM_ROWS            8
#define XAIE_NUM_COLS            50
#define XAIE_ADDR_ARRAY_OFF      0x800

#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 25
#define XAIE_ROW_SHIFT 20
#define XAIE_SHIM_ROW 0
#define XAIE_MEM_TILE_ROW_START 1
#define XAIE_MEM_TILE_NUM_ROWS 1
#define XAIE_AIE_TILE_ROW_START 2
#define XAIE_AIE_TILE_NUM_ROWS 8

#elif AIE_GEN == 1

#define HW_GEN XAIE_DEV_GEN_AIE
#define XAIE_NUM_ROWS            9
#define XAIE_NUM_COLS            50
#define XAIE_ADDR_ARRAY_OFF      0x800

#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 23
#define XAIE_ROW_SHIFT 18
#define XAIE_SHIM_ROW 0
#define XAIE_MEM_TILE_ROW_START 0
#define XAIE_MEM_TILE_NUM_ROWS 0
#define XAIE_AIE_TILE_ROW_START 1
#define XAIE_AIE_TILE_NUM_ROWS 8

#endif

#endif /* HW_CONFIG_H */
