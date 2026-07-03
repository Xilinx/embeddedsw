// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
#include <stdio.h>
#include "sleep.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "mbox_cmd.h"
#include "sensor_cmd.h"

//#include "sensor_cfgs.h"
#include "vlog.h"
//#include "vvdevice.h"

#define LOGTAG "VISP_COMMON"

/* Send the ISP buffer addressing mode to the RPU.
 *   high_mem : high 32 bits of the buffer-pool physical address
 *              (0x0 = 32-bit / low memory, 0x8 = DDR_LOW_1, 0x500 = DDR_CH_1)
 *   is_64bit : 0 for 32-bit addressing, 1 for 64-bit / high memory
 * Both are derived by the caller from where the linker placed the pools, so
 * the same binary works for low / 32-bit and any high-memory region. */
int SetATM(uint32_t high_mem, uint32_t is_64bit);

int ATM_ENABLE = 0;
