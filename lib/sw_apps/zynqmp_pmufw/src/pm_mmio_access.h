/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_MMIO_ACCESS_H
#define PM_MMIO_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Function declarations
 ********************************************************************/
bool PmGetMmioAccessRead(const PmMaster *const master, const u32 address);
bool PmGetMmioAccessWrite(const PmMaster *const master, const u32 address);

#ifdef __cplusplus
}
#endif

#endif /* PM_MMIO_ACCESS_H */
