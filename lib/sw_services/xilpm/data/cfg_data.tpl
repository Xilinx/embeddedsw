/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#include "xil_types.h"
#include "pm_defs.h"

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
	1,	/* Number of remaining words in the header */
	8,	/* Number of sections included in config object */
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
	PM_CONFIG_SET_CONFIG_SECTION_ID,	/* Section ID */
	0,					/* Permissions to set config */
	/**********************************************************************/
	/* SHUTDOWN SECTION */
	PM_CONFIG_SHUTDOWN_SECTION_ID,		/* Section ID */
	0,					/* Number of shutdown types */
	/**********************************************************************/
	/* GPO SECTION */
<<GPO_SECTION_DATA>>
};
#if defined (__ICCARM__)
#pragma language=restore
#endif
