/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_MMIO_ACCESS_H
#define PM_MMIO_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_MEM_RANGE

#ifdef ENABLE_MEM_RANGE_PM_SET_CONFIG
#define BASE_CONFIG_OBJ_START_ADDR	0xFFFD6740U
#define BASE_CONFIG_OBJ_LEN		0x1DBU

#define OVERLAY_CONFIG_OBJ_START_ADDR	0x812CB7C
#define OVERLAY_CONFIG_OBJ_LEN		0x20U
#endif

#ifdef ENABLE_MEM_RANGE_PM_SELF_SUSPEND
#define SELF_SUSPEND_DDR_START_ADDR	0x0U
#define SELF_SUSPEND_OCM_START_ADDR	0xFFFEA144U
#define SELF_SUSPEND_DDR_OCM_ADDR_LEN	0x4U
#endif

#ifdef ENABLE_MEM_RANGE_PM_REQUEST_WAKEUP
#define REQUEST_WAKEUP_OCM_START_ADDR	0xFFFEA144U
#define REQUEST_WAKEUP_OCM_ADDR_LEN	0x4U
#endif

/**
 * @enum XPm_MemRangeAccessType
 * @brief Defines the types of access permissions for a memory range.
 */
typedef enum {
        MEM_RANGE_READ_ACCESS = 1, /**< Accessible for read operations only. */
        MEM_RANGE_WRITE_ACCESS,	   /**< Accessible for write operations only. */
        MEM_RANGE_ANY_ACCESS,	   /**< Accessible for both read and write operations. */
}XPm_MemRangeAccessType;
#endif

/*********************************************************************
 * Function declarations
 ********************************************************************/
bool PmGetMmioAccessRead(const PmMaster *const master, const u32 address);
bool PmGetMmioAccessWrite(const PmMaster *const master, const u32 address);
#ifdef ENABLE_MEM_RANGE
u32 PmIsValidAddressRange(const PmMaster *const master, const UINTPTR address,
			  const u32 length, const u32 access);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_MMIO_ACCESS_H */
