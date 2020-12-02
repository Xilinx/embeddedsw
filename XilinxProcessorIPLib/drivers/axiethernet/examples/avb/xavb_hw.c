/******************************************************************************
* Copyright (C) 2008 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xavb_hw.c
*
* The xavb_hw driver. Functions in this file are the minimum required functions
* for this driver. See xavb_hw.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a mbr  09/19/08 First release
* 1.01a mbr  06/24/09 PTP frame format updates for IEEE802.1 AS draft 5-0
* 2_02a mbr  09/16/09 Updates for programmable PTP timers
* 2_04a kag  07/23/10 PTP frame format updates for IEEE802.1 AS draft 6-7
* 3_01a kag  08/29/11 Added new APIs to update the RX Filter Control Reg.
*		      Fix for CR:572539. Updated bit map for Rx Filter
*		      control reg.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"
#include "xavb_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/****************************************************************************/
/**
*
* This function reads the given Ethernet Statistic Register
*
* @param  BaseAddress is the base address of the device
* @param  CounterID is the Statistic Counter to be read
* @param  Value a pointer to the read value of the 64-bit value of the counter
*         and it is updated by this function
*
* @return None.
*
* @note   The CounterID's defined here are the same as the Statistic Counter
*         Addresses as used with the TEMAC core.  Since each counter is 8-bytes
*         wide, we must multiply these Counter ID's by 8 (or a left shift of 3)
*         when mapping this into the PLB memory map.  The 64-bit counter value
*         is read as two separate 32-bit accesses.
*
*****************************************************************************/
void XAvbMac_ReadStats(u32 BaseAddress,
                        u32 CounterId,
                        XAvb_Uint64* Value)
{
  Value->Upper = Xil_In32(BaseAddress + ((CounterId & 0x00ff) << 3) + 0x4);
  Value->Lower = Xil_In32(BaseAddress + ((CounterId & 0x00ff) << 3));
}


/****************************************************************************/
/**
*
* This function reads the current Real Time Counter (RTC) value
*
* @param  BaseAddress is the base address of the device
* @param  RtcValue is a pointer to a struct in which to store the value read
*         from the RTC (The RTC 48-bit seconds field and the 32-bit ns field
*         of this struct are updated).
*
* @return None
*
* @note   This is provided as a basic function since the ns field MUST be read
*         before the seconds/epoch registers (reading the ns samples the entire
*         RTC in hardware).
*
*****************************************************************************/
void XAvb_ReadRtc(u32 BaseAddress,
                   XAvb_RtcFormat* RtcValue)
{
  RtcValue->NanoSeconds  =
      Xil_In32(BaseAddress + XAVB_RTC_NANOSEC_VALUE_OFFSET)
      & XAVB_RTC_NS_MASK;

  RtcValue->SecondsLower =
      Xil_In32(BaseAddress + XAVB_RTC_SEC_LOWER_VALUE_OFFSET)
      & XAVB_RTC_SEC_LOWER_MASK;

  RtcValue->SecondsUpper =
      Xil_In32(BaseAddress + XAVB_RTC_SEC_UPPER_VALUE_OFFSET)
      & XAVB_RTC_SEC_UPPER_MASK;
}


/****************************************************************************/
/**
*
* This function writes to the Real Time Counter (RTC) Offset Registers
*
* @param  BaseAddress is the base address of the device
* @param  RtcValue is the nanoseconds and seconds offset values that should
*         be written to the RTC.
*
* @return None
*
* @note   This is provided as a basic function since the ns field MUST be
*         written after the seconds/epoch offset registers (writing to the ns
*         offset registers samples the entire RTC offset in hardware).
*
*****************************************************************************/
void XAvb_WriteRtcOffset(u32 BaseAddress, XAvb_RtcFormat* RtcValue)
{
  Xil_Out32((BaseAddress + XAVB_RTC_SEC_UPPER_OFFSET),
            (RtcValue->SecondsUpper & XAVB_RTC_SEC_UPPER_MASK));

  Xil_Out32((BaseAddress + XAVB_RTC_SEC_LOWER_OFFSET),
            (RtcValue->SecondsLower & XAVB_RTC_SEC_LOWER_MASK));

  Xil_Out32((BaseAddress + XAVB_RTC_NANOSEC_OFFSET),
            (RtcValue->NanoSeconds  & XAVB_RTC_NS_MASK));
}
