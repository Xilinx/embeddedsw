/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_sinit.c
* @addtogroup vprocss Overview
* @{
* @brief
*
* This file contains the implementation of the Video Processing Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xvprocss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XVprocSs_Config XVprocSs_ConfigTable[];

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XVprocSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param  DeviceId is the unique device ID of the device being looked up
*
* @return A pointer to the configuration table entry corresponding to the
*         given device ID, or NULL if no match is found
*
*******************************************************************************/
#ifndef SDT
XVprocSs_Config* XVprocSs_LookupConfig(u32 DeviceId)
{
  XVprocSs_Config *CfgPtr = NULL;
  u32 index;

  for (index = 0U; index < (u32)XPAR_XVPROCSS_NUM_INSTANCES; index++)
  {
    if (XVprocSs_ConfigTable[index].DeviceId == DeviceId)
    {
      CfgPtr = &XVprocSs_ConfigTable[index];
      break;
    }
  }
  return (CfgPtr);
}
#else
/**
 * This function searches for and returns a pointer to the XVprocSs_Config
 * structure that matches the specified BaseAddress. If BaseAddress is zero,
 * it returns the first entry in the XVprocSs_ConfigTable. If no matching
 * configuration is found, it returns NULL.
 *
 * @param	BaseAddress is the base address of the device to look up.
 *
 * @return	A pointer to the matching XVprocSs_Config structure if found,
 *          otherwise NULL.
 */

XVprocSs_Config* XVprocSs_LookupConfig(UINTPTR BaseAddress)
{
  XVprocSs_Config *CfgPtr = NULL;
  u32 Index;

  for (Index = 0U; XVprocSs_ConfigTable[Index].Name != NULL; Index++) {
    if ((XVprocSs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
      CfgPtr = &XVprocSs_ConfigTable[Index];
      break;
    }
  }
  return (CfgPtr);
}

/**
 * Retrieves the index of the driver configuration in the XVprocSs_ConfigTable
 * that matches the specified BaseAddress.
 *
 * This function iterates through the XVprocSs_ConfigTable array and compares
 * each entry's BaseAddress with the provided BaseAddress. If a match is found,
 * the corresponding index is returned. If no match is found, the function
 * returns the index at which the Name field is NULL (end of the table).
 *
 * @param	BaseAddress	The base address to search for in the configuration table.
 *
 * @return	The index of the matching configuration entry in XVprocSs_ConfigTable.
 *		If no match is found, returns the index at the end of the table.
 */


u32 XVprocSs_GetDrvIndex(UINTPTR BaseAddress)
{
 u32 Index = 0;

 for (Index = 0U; XVprocSs_ConfigTable[Index].Name != NULL; Index++) {
   if ((XVprocSs_ConfigTable[Index].BaseAddress == BaseAddress)) {
	break;
   }
 }
 return Index;
}

#endif
/** @} */
