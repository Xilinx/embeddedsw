/*******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xddrpsv.h
 * @addtogroup ddrpsv_v1_3
 * @{
 * @details
 *
 * The Xilinx DdrPsv driver. This driver supports the Xilinx axi_noc
 * IP core.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0	 mus  03/16/19 First Release
 * 1.3   mus  05/08/20 Updated tcl to include detailed #defines for all
 *                     NOC IPs present in the design. Also, updated logic
 *                     to create canonical #defines.
 *
 *                     In case, if more than 1 NOC IP in design is connected to
 *                     same DDR segment through different address range,
 *                     generated #defines could be wrong, as existing logic
 *                     doesnt generate unique #define, so names would be
 *                     repeated and values would be wrong in certain scenarios.
 *
 *                     Now, new logic is adding 2 major enhancements. First
 *                     one is, it is creating detailed #defines for each
 *                     connection to the DDR segment, which includes NOC IP
 *                     instance name, DDR segment name and master
 *                     interface name, it would avoid repeatation in names.
 *                     So, xparameters.h would reflect each NOC connection
 *                     in HW design.
 *                     Second enhancement is, it would create canonicals
 *                     for each DDR segment, which would be consumed by
 *                     translation table/MPU in BSP. Base address canonical
 *                     would point to lowest base address value for that
 *                     specific DDR segemnt in HW design, and high address
 *                     canonical would point to highest high address for
 *                     that DDR segment in given HW design.
 *
 *                     Limitation which still exist: In case, if
 *                     different NOC instances are connected to same DDR
 *                     region through different address ranges, we are
 *                     assuming that there is no hole in that address
 *                     ranges. If HW design contains holes in those
 *                     address ranges, canonical #defines would point to
 *                     base address and high address with holes.
 *                     For example, specific HW design is having 2 NOC
 *                     instances NOC1 and NOC2. NoC1 is connected to DDR_LOW_0
 *                     with base address 0x0 and high address as 0x3FFF.
 *                     NOC2 is connected to DDR_LOW_0 with base address as
 *                     0x4000_0000 and high address as 0x4FFF_FFFF. Now, as per
 *                     logic, generated base address canonical for DDR_LOW_0
 *                     would point to  0x0 and high address canonical
 *                     points to 0x4FFF_FFFF. It is incorrect as, there is
 *                     hole in address range starting from 0x4000 to
 *                     0x3FFF_FFFF.
 *
 * </pre>
 *
*******************************************************************************/

#ifndef XDDRPSV_H_
/* Prevent circular inclusions by using protection macros. */
#define XDDRPSV_H_

/******************************* Include Files ********************************/


#endif /* XDDRPSV_H_ */
/** @} */
