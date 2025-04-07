/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file si5324drv.h
*
* This file contains definitions for low-level driver functions for
* controlling the SiliconLabs Si5324 clock generator as mounted on the KC705
* demo board.
* The user should refer to the hardware device specification for more details
* of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- --- ----------   -----------------------------------------------
*           dd/mm/yyyy
* ----- --- ----------   -----------------------------------------------
* 1.00  gm  12/05/2018   Initial release
* </pre>
*
******************************************************************************/

#ifndef VIDEO_FMC_H_
#define VIDEO_FMC_H_

#include "xparameters.h"
#include "xil_types.h"
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
#include "xiicps.h"
#else
#include "xiic.h"
#endif
#include "sleep.h"
#include "xgpio.h"
#include "idt_8t49n24x.h"
#include "ti_lmk03318.h"
#include "onsemi_nb7nq621m.h"
#include "si5344drv.h"
#if defined (XPS_BOARD_VEK280_ES) || \
	defined (XPS_BOARD_VEK280_ES_REVB)
#define XPS_BOARD_VEK280
#endif
#if defined (XPS_BOARD_VEK280)
#include "ti_tmds1204.h"
#include "rc21008adrv.h"
#elif defined (XPS_BOARD_VEK385)
#include "ti_tmds1204.h"
#endif
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == 6) /*GTYE4*/
#define XPS_BOARD_VCU118
#else
/* Place-holder for other boards in future */
#endif
#define VFMC_GPIO_TX_CH4_DATASRC_SEL_MASK	0x00000004

#define VFMC_GPIO_RX_CH4_DATASRC_SEL_MASK	0x00040000

typedef enum {
	VFMC_HPC0  				= 1,
	VFMC_HCP1   			= 2,
	VFMC_LOC_INVALID 		= 99,
} XVfmc_Location;

typedef enum {
	VFMC_MEZZ_HDMI_PASSIVE     = 0x70000001,
	VFMC_MEZZ_HDMI_ONSEMI_R0   = 0x70000100,	/* ONSEMI Pass 1 */
	VFMC_MEZZ_HDMI_ONSEMI_R1   = 0x70000101,	/* ONSEMI Pass 2 */
	VFMC_MEZZ_HDMI_ONSEMI_R2   = 0x70000102,	/* ONSEMI Pass 3 */
	VFMC_MEZZ_HDMI_ONSEMI_R3   = 0x70000103,	/* ONSEMI Pass 4 */
	VFMC_MEZZ_HDMI_TI_R0       = 0x70000200,	/* TI Dummy */
	VFMC_MEZZ_HDMI_TI_R1       = 0x70000201,	/* TI Rev1 */
	VFMC_MEZZ_HDMI_TI_R3       = 0x70000201,	/* TI Rev1 */
	VFMC_MEZZ_INVALID          = 0x70000999,
} XVfmc_MezzType;

typedef enum {
	VFMC_GPIO_TX_LED0 	    = 0x00000001,
	VFMC_GPIO_TX_LED1 	    = 0x00000002,
	VFMC_GPIO_RX_LED0 	    = 0x00010000,
	VFMC_GPIO_RX_LED1 	    = 0x00020000,
} XVfmc_Gpio_Led;

typedef enum {
	VFMC_GPIO_TX_CH4_As_DataAndClock,
	VFMC_GPIO_TX_CH4_As_ClockOut,
	VFMC_GPIO_RX_CH4_As_Data,
	VFMC_GPIO_RX_CH4_As_Clock,
} XVfmc_Gpio_Ch4_DataClkSel;

typedef enum {
	VFMC_MEZZ_RxRefclk_From_Si5344,
	VFMC_MEZZ_RxRefclk_From_Cable,
} XVfmc_Mezz_RxRefClkSel;

typedef enum {
	VFMC_MEZZ_TxRefclk_From_IDT,
	VFMC_MEZZ_TxRefclk_From_Si5344,
} XVfmc_Mezz_TxRefClkSel;
/**
 * This typedef defines the Vfmc structure
 */
typedef struct {
	void *IicPtr;        /**< Reference to IIC controller for vfmc. */
	XGpio Gpio;          /**< Reference to GPIO for vfmc */
	XVfmc_Location Loc;  /**< Location of vfmc on development board */
	XVfmc_MezzType TxMezzType; /**< Mezzanine Type */
	XVfmc_MezzType RxMezzType; /**< Mezzanine Type */
	u32 IsReady;		 /**< Is Ready */
	u8 isTxTi;
	u8 isRxTi;
} XVfmc;


int Vfmc_I2cMuxSelect(XVfmc *VfmcPtr);
#ifndef SDT
u32 Vfmc_HdmiInit(XVfmc *VfmcPtr, u16 GpioDeviceId, void *IicPtr,
#else
u32 Vfmc_HdmiInit(XVfmc *VfmcPtr, UINTPTR GpioBaseAddr, void *IicPtr,
#endif
		XVfmc_Location Loc);
int Vfmc_PowerDownTiLMK03318(XVfmc *VfmcPtr, u8 Powerdown);
void Vfmc_Gpio_Led_On(XVfmc *VfmcPtr, XVfmc_Gpio_Led Led, u8 On);
void Vfmc_Gpio_Ch4_DataClock_Sel(XVfmc *VfmcPtr,
		XVfmc_Gpio_Ch4_DataClkSel DataClkSel);
void Vfmc_Gpio_Mezz_HdmiTxDriver_Enable(XVfmc *VfmcPtr, u8 Enable);
void Vfmc_Gpio_Mezz_HdmiRxEqualizer_Enable(XVfmc *VfmcPtr, u8 Enable);
void Vfmc_Gpio_Mezz_HdmiTxDriver_Reconfig(XVfmc *VfmcPtr, u8 IsFRL,
		u64 LineRate, u8 Lanes);
void Vfmc_Gpio_Mezz_HdmiRxDriver_Reconfig(XVfmc *VfmcPtr, u8 IsFRL,
		u64 LineRate, u8 Lanes);
u32 Vfmc_Mezz_HdmiRxRefClock_Sel(XVfmc *VfmcPtr, XVfmc_Mezz_RxRefClkSel Sel);
u32 Vfmc_Mezz_HdmiTxRefClock_Sel(XVfmc *VfmcPtr, XVfmc_Mezz_TxRefClkSel Sel);
#endif /* VIDEO_FMC_H_ */
