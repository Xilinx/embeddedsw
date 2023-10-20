/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager.h
* @addtogroup Overview
* @{
* @details
*
* This component contains the implementation of the XTMR_Manager component which is
* the driver for the Xilinx TMR Manager device. Most features are configurable
* at run time by software, but someare only configurable when the hardware device
* is built.
*
* The driver defaults to no interrupts at initialization such that interrupts
* must be enabled if desired. An interrupt is generated when a SEM event that
* has not been masked occurs.
*
* <b>Initialization & Configuration</b>
*
* The XTMR_Manager_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in one
* of the following ways:
*
*   - XTMR_Manager_Initialize(InstancePtr, DeviceId) - The driver looks up its own
*     configuration structure created by the tool-chain based on an ID provided
*     by the tool-chain.
*
*   - XTMR_Manager_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*     configuration structure provided by the caller. If running in a system
*     with address translation, the provided virtual memory base address
*     replaces the physical address present in the configuration structure.
*
* <b>RTOS Independence</b>
*
* This driver is intended to be RTOS and processor independent.  It works
* with physical addresses only.  Any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control must
* be satisfied by the layer above this driver.
*
* @note
*
* The driver is partitioned such that a minimal implementation may be used.
* More features require additional files to be linked in.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 1.2   mus  08/31/20 Updated makefile to support parallel make and
*                     incremental builds. It would help to reduce
*                     compilation time.
* 1.3   adk  01/07/22 Fixed Assert check in the XTMR_Manager_BreakHandler API,
* 		      Corrected the break handler handoff offset in the
* 		      xtmr_manager_recover_l.S file.
*       adk  01/31/22 Updated the interrupt examples independent of SEM IP
*       	      hardware configuration.
*       adk  02/23/22 Added new API XTMR_Manager_Configure_BrkDelay()
*       	      for configuring break delay.
* 1.4   adk  27/12/22 Updated addtogroup tag.
* 1.5   adk  03/07/23 Added support for system device-tree flow.
* 1.6   adk  20/10/23 The function XTMR_Manager_InjectionTest was erroneously
* 		      declared. It does not exist. Remove the
* 		      XTMR_Manager_InjectionTest API declaration.
* </pre>
*
*****************************************************************************/

#ifndef XTMR_MANAGER_H /* prevent circular inclusions */
#define XTMR_MANAGER_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/**
 * Callback function.  The first argument is a callback reference passed in by
 * the upper layer when setting the callback functions, and passed back to the
 * upper layer when the callback is invoked.
 */
typedef void (*XTMR_Manager_Handler)(void *CallBackRef);

/**
 * Statistics for the XTMR_Manager driver
 */
typedef struct {
	u32 InterruptCount;		/**< Number of SEM interrupts */
	u32 RecoveryCount;		/**< Number of recoveries performed */
} XTMR_Manager_Stats;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR RegBaseAddr;	/**< Register base address */
	u32 BrkDelayRstValue;	/**< Value of parameter C_BRK_DLEAY_RST_VALUE */
	u64 MaskRstValue;	/**< Value of parameter C_MASK_RST_VALUE */
	u8  Magic1;		/**< Value of parameter C_MAGIC1 */
	u8  Magic2;		/**< Value of parameter C_MAGIC2 */
	u8  UEIsFatal;		/**< Value of parameter C_UE_IS_FATAL */
	u8  UEWidth;		/**< Value of parameter C_UE_WIDTH */
	u8  NoOfComparators;	/**< Value of parameter C_NO_OF_COMPARATORS */
	u8  ComparatorsMask;	/**< Value of parameter C_COMPARATORS_MASK */
	u8  Watchdog;		/**< Value of parameter C_WATCHDOG */
	u8  WatchdogWidth;	/**< Value of parameter C_WATCHDOG_WIDTH */
	u8  SemInterface;	/**< Value of parameter C_SEM_INTERFACE */
	u8  SemWatchdog;	/**< Value of parameter
				   C_SEM_HEARTBEAT_WATCHDOG */
	u8  SemWatchdogWidth;	/**< Value of parameter
				   C_SEM_HEARTBEAT_WATCHDOG_WIDTH */
	u8  BrkDelayWidth;	/**< Value of parameter C_BRK_DELAY_WIDTH */
	u8  Tmr;		/**< Value of parameter C_TMR */
	u8  TestComparator;	/**< Value of parameter C_TEST_COMPARATOR */
	u8  StrictMiscompare;	/**< Value of parameter C_STRICT_MISCOMPARE */
	u8  UseDebugDisable;	/**< Value of parameter C_USE_DEBUG_DISABLE */
	u8  UseTmrDisable;	/**< Value of parameter C_USE_TMR_DISABLE */
} XTMR_Manager_Config;

/**
 * The XTMR_Manager driver instance data. The user is required to allocate a
 * variable of this type for every TMR Manager device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XTMR_Manager_Stats Stats;	/* Component Statistics */
#ifdef SDT
	XTMR_Manager_Config Config;    /* Config structure pointer */
#endif
	UINTPTR RegBaseAddress;		/* Base address of registers */
	u32 IsReady;			/* Device is initialized and ready */

	u32 Cr;				/* Control register value (CR) */
	u32 Cmr0;			/* Comparison mask 0 (CMR0) */
	u32 Cmr1;			/* Comparison mask 0 (CMR0) */
	u32 Bdir;			/* Break delay init (BDIR) */
	u32 SemImr;			/* SEM interrupt mask (SEMIMR) */
	u32 Cfir;			/* Comparator fault inject (CFIR) */

	XTMR_Manager_Handler RecoveryHandler;
	void *RecoveryCallBackRef;	/* Callback ref for handler */
	XTMR_Manager_Handler PreResetHandler;
	void *PreResetCallBackRef;	/* Callback ref for handler */
	XTMR_Manager_Handler PostResetHandler;
	void *PostResetCallBackRef;	/* Callback ref for handler */

	XTMR_Manager_Handler Handler;
	void *CallBackRef;		/* Callback ref for handler */
} XTMR_Manager;


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

/*
 * Initialization functions in xtmr_manager_sinit.c
 */
#ifndef SDT
int XTMR_Manager_Initialize(XTMR_Manager *InstancePtr, u16 DeviceId);
XTMR_Manager_Config *XTMR_Manager_LookupConfig(u16 DeviceId);
#else
int XTMR_Manager_Initialize(XTMR_Manager *InstancePtr, UINTPTR BaseAddr);
XTMR_Manager_Config *XTMR_Manager_LookupConfig(UINTPTR BaseAddr);
#endif

/*
 * Required functions, in file xtmr_manager.c
 */
int XTMR_Manager_CfgInitialize(XTMR_Manager *InstancePtr,
				XTMR_Manager_Config *Config,
				UINTPTR EffectiveAddr);
void XTMR_Manager_Configure_BrkDelay(XTMR_Manager *InstancePtr,
				     u32 BrkDelay);

/*
 * Functions for recovery, in file xtmr_manager_recover.c
 */
void XTMR_Manager_SetRecoveryHandler(XTMR_Manager *InstancePtr,
				     XTMR_Manager_Handler FuncPtr,
				     void *CallBackRef);

void XTMR_Manager_SetPreResetHandler(XTMR_Manager *InstancePtr,
				     XTMR_Manager_Handler FuncPtr,
				     void *CallBackRef);

void XTMR_Manager_SetPostResetHandler(XTMR_Manager *InstancePtr,
				      XTMR_Manager_Handler FuncPtr,
				      void *CallBackRef);

/*
 * Functions for internal watchdog, in file xtmr_manager_wdog.c
 */
void XTMR_Manager_RestartWdt(XTMR_Manager *InstancePtr);
u32 XTMR_Manager_IsWdtExpired(XTMR_Manager *InstancePtr);

/*
 * Functions for statistics, in file xtmr_manager_stats.c
 */
void XTMR_Manager_GetStats(XTMR_Manager *InstancePtr, XTMR_Manager_Stats *StatsPtr);
void XTMR_Manager_ClearStats(XTMR_Manager *InstancePtr);

/*
 * Functions for SEM control and interrupts, in file xtmr_manager_sem.c
 */
void XTMR_Manager_SemCommand(XTMR_Manager *InstancePtr, char *command,
			    char *response);

void XTMR_Manager_EnableInterrupt(XTMR_Manager *InstancePtr, u32 mask);
void XTMR_Manager_DisableInterrupt(XTMR_Manager *InstancePtr);

void XTMR_Manager_SetHandler(XTMR_Manager *InstancePtr,
			     XTMR_Manager_Handler FuncPtr,
			     void *CallBackRef);

/*
 * Functions for self-test, in file xtmr_manager_selftest.c
 */
int XTMR_Manager_SelfTest(XTMR_Manager *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif			/* end of protection macro */

/** @} */
