/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "pm_defs.h"
#include "pm_cfg_obj.h"

#define PM_CONFIG_MASTER_SECTION_ID	0x101U
#define PM_CONFIG_SLAVE_SECTION_ID	0x102U
#define PM_CONFIG_PREALLOC_SECTION_ID	0x103U
#define PM_CONFIG_POWER_SECTION_ID	0x104U
#define PM_CONFIG_RESET_SECTION_ID	0x105U
#define PM_CONFIG_SHUTDOWN_SECTION_ID	0x106U
#define PM_CONFIG_SET_CONFIG_SECTION_ID	0x107U
#define PM_CONFIG_GPO_SECTION_ID	0x108U

#define PM_SLAVE_FLAG_IS_SHAREABLE	0x1U
#define PM_MASTER_USING_SLAVE_MASK	0x2U

#define PM_CONFIG_GPO1_MIO_PIN_34_MAP	(1U << 10U)
#define PM_CONFIG_GPO1_MIO_PIN_35_MAP	(1U << 11U)
#define PM_CONFIG_GPO1_MIO_PIN_36_MAP	(1U << 12U)
#define PM_CONFIG_GPO1_MIO_PIN_37_MAP	(1U << 13U)

#define PM_CONFIG_GPO1_BIT_2_MASK	(1U << 2U)
#define PM_CONFIG_GPO1_BIT_3_MASK	(1U << 3U)
#define PM_CONFIG_GPO1_BIT_4_MASK	(1U << 4U)
#define PM_CONFIG_GPO1_BIT_5_MASK	(1U << 5U)

#define SUSPEND_TIMEOUT	0xFFFFFFFFU

#define PM_CONFIG_OBJECT_TYPE_BASE	0x1U

<<MASTER_IPI_MASK_DEF>>

#if defined (__ICCARM__)
#pragma language=save
#pragma language=extended
#endif
#if defined (__GNUC__)
    const u32 XPm_ConfigObject[] __attribute__((used, section(".sys_cfg_data"))) =
#elif defined (__ICCARM__)
#pragma location = ".sys_cfg_data"
__root const u32 XPm_ConfigObject[] =
#endif
{
	/**********************************************************************/
	/* HEADER */
	2,	/* Number of remaining words in the header */
	8,	/* Number of sections included in config object */
	PM_CONFIG_OBJECT_TYPE_BASE,	/* Type of config object as base */
	/**********************************************************************/
	/* MASTER SECTION */
<<MASTER_SECTION_DATA>>
	/**********************************************************************/
	/* SLAVE SECTION */
<<SLAVE_SECTION_DATA>>
	/**********************************************************************/
	/* PREALLOC SECTION */
<<PREALLOC_SECTION_DATA>>
	/**********************************************************************/
	/* POWER SECTION */
<<POWER_SECTION_DATA>>
	/**********************************************************************/
	/* RESET SECTION */
<<RESET_SECTION_DATA>>
	/**********************************************************************/
	/* SET CONFIG SECTION */
<<SET_CONFIG_SECTION_DATA>>
	/**********************************************************************/
	/* SHUTDOWN SECTION */
<<SHUTDOWN_SECTION_DATA>>
	/**********************************************************************/
	/* GPO SECTION */
<<GPO_SECTION_DATA>>
};
#if defined (__ICCARM__)
#pragma language=restore
#endif
