/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilskey_bbram_ultrascale_input.h
*
* This file contains macros which needs to configured by user based on the
* options selected by user operations will be performed.
*
* @note
*
*  		User configurable parameters for Ultrascale BBRAM
*  	----------------------------------------------------------------------
*	#define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0		FALSE
*	TRUE will program BBRAM with OBFUSCATED key provided in
*	XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0. While programming obfuscated key DPA
*	configurations cannot be done due to silicon bug, and values provided
*	in DPA configuration macros will be ignored.
*	FALSE will program the BBRAM with key provided in XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_0
*	and DPA configurations (protect, count and mode) can be configured
*
*       #define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1		FALSE
*       TRUE will program BBRAM with OBFUSCATED key provided in
*	XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1. While programming obfuscated key DPA
*	configurations cannot be done due to silicon bug, and values provided
*	in DPA configuration macros will be ignored.
*	FALSE will program the BBRAM with key provided in XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_1
*	and DPA configurations (protect, count and mode) can be configured
*
*       #define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2		FALSE
*       TRUE will program BBRAM with OBFUSCATED key provided in
*	XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2. While programming obfuscated key DPA
*	configurations cannot be done due to silicon bug, and values provided
*	in DPA configuration macros will be ignored.
*	FALSE will program the BBRAM with key provided in XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_2
*	and DPA configurations (protect, count and mode) can be configured
*
*       #define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3		FALSE
*	TRUE will program BBRAM with OBFUSCATED key provided in
*	XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3. While programming obfuscated key DPA
*	configurations cannot be done due to silicon bug, and values provided
*	in DPA configuration macros will be ignored.
*	FALSE will program the BBRAM with key provided in XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_3
*	and DPA configurations (protect, count and mode) can be configured
*
*	#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0
*	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
*	The value mentioned in this will be converted to hex buffer and the
*	key is programmed into BBRAM, when program API used. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not program BBRAM. Note that,
*	for writing the OBFUSCATED Key, XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0 should
*	have TRUE value.
*
*       #define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1
*       "b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
*       The value mentioned in this will be converted to hex buffer and the
*	key is programmed into BBRAM, when program API used. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not program BBRAM. Note that,
*	for writing the OBFUSCATED Key, XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1 should
*	have TRUE value.
*
*	#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2
*	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
*	The value mentioned in this will be converted to hex buffer and the
*	key is programmed into BBRAM, when program API used. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not program BBRAM. Note that,
*	for writing the OBFUSCATED Key, XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2 should
*	have TRUE value.
*
*	#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3
*	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
*	The value mentioned in this will be converted to hex buffer and the
*	key is programmed into BBRAM, when program API used. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not program BBRAM. Note that,
*	for writing the OBFUSCATED Key, XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3 should
*	have TRUE value.
*
*	#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_0			FALSE
*	#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_1			FALSE
*	#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_2			FALSE
*	#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_3			FALSE
*       TRUE will program BBRAM with AES Key provided in XSK_BBRAM_AES_KEY for
*       particular SLR.
*
* 	#define 	XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_0
*	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"
*	#define 	XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_1
*	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"
*	#define 	XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_2
*	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"
*	#define 	XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_3
*	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"
*	The value mentioned in this will be converted to hex buffer and the
*	key is programmed into BBRAM for particular SLR(1/2/3/4), when
*	program API used. It should be 64 characters long, valid characters
*	are 0-9,a-f,A-F. Any other character is considered as invalid string
*	and will not program BBRAM. Note that, for writing the BBRAM Key, XSK_
*	BBRAM_PGM_OBFUSCATED_KEY for particular SLR(1/2/3/4)should have FALSE value.
*
* 	#define	XSK_BBRAM_AES_KEY_SIZE_IN_BITS	256
*
*	#define XSK_BBRAM_DPA_PROTECT_ENABLE	FALSE
*	TRUE will enable the DPA protection for BBRAM key, to enable
*	DPA protection XSK_BBRAM_PGM_OBFUSCATED_KEY should be FALSE,
*	as DPA protection is not supported for Obfuscated key due to silicon
*	bug, XSK_BBRAM_DPA_COUNT should be in the range of 1-255 and
*	XSK_BBRAM_DPA_MODE can be XSK_BBRAM_INVALID_CONFIGURATIONS or
*	XSK_BBRAM_ALL_CONFIGURATIONS.
*	FALSE will not enable DPA protection, XSK_BBRAM_DPA_COUNT and
*	XSK_BBRAM_DPA_MODE both will be taken default values 0 and
*	XSK_BBRAM_INVALID_CONFIGURATIONS respectively
*
*	#define XSK_BBRAM_DPA_COUNT	0
*	Default value will be zero,
*	when XSK_BBRAM_DPA_PROTECT_ENABLE is TRUE this should be in range
*	of 1-255 (should be greater that zero).
*
*	#define XSK_BBRAM_DPA_MODE	XSK_BBRAM_INVALID_CONFIGURATIONS
*	Default value is XSK_BBRAM_INVALID_CONFIGURATIONS
*	When XSK_BBRAM_DPA_PROTECT_ENABLE is TRUE it can be
*	XSK_BBRAM_INVALID_CONFIGURATIONS or XSK_BBRAM_ALL_CONFIGURATIONS
*	If XSK_BBRAM_DPA_PROTECT_ENABLE is FALSE, it should be
*	XSK_BBRAM_INVALID_CONFIGURATIONS.
*
*	#define XSK_BBRAM_AXI_GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID
*	Default value is XPAR_AXI_GPIO_0_DEVICE_ID
*	This macro is for providing exact GPIO device ID, based on the
*	design configuration this macro should be modified to provide
*	GPIO device ID which is used for connecting MASTER JTAG pins.
*
*	In Ultrascale GPIO pins are used for connecting MASTER_JTAG pins to
*	access BBRAM.
*	Following are the GPIO pins and user can change these pins
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TDO	(0)
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TDI	(0)
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TMS	(1)
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TCK	(2)
*
*	#define XSK_BBRAM_GPIO_INPUT_CH		(2)
*	This macro is for providing channel number of ALL INPUTS connected(TDO)
*	#define XSK_BBRAM_GPIO_OUTPUT_CH	(1)
*	This macro is for providing channel number of ALL OUTPUTS connected
*	(TDI, TCK, TMS)
*
*	NOTE: All inputs and outputs of GPIO can be configured in single
*	channel also
*	i.e XSK_BBRAM_GPIO_INPUT_CH = XSK_BBRAM_GPIO_OUTPUT_CH = 1 or 2.
*	Among (TDI, TCK, TMS) Outputs of GPIO cannot be connected to different
*	GPIO channels all the 3 signals should be in same channel.
*	TDO can be a other channel of (TDI, TCK, TMS) or the same.
*	DPA protection can be enabled only when programming non-obfuscated key.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ----    -------- ------------------------------------------------------
* 5.0   vns     09/01/16 First Release.
* 6.0   vns     07/28/16 Added counting configuration
*                        feature(DPA count), protection and to
*                        program Obfuscated key.
* 6.7   psl     03/20/19 Added BBRAM key write support for SSIT devices.
*       psl     03/29/19 Added Support for user configurable GPIO for
*                        jtag control.
* 7.5   ng      07/13/23 added SDT support
*       ng      09/02/23 fixed gpio macro in SDT flow
* </pre>
*
******************************************************************************/

#ifndef XILSKEY_INPUT_H
#define XILSKEY_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xparameters.h"
/************************** Constant Definitions *****************************/

/* Constant definitions to specify the configurations in XSK_BBRAM_DPA_MODE*/
/* User should not change the below two constant definitions */
#define XSK_BBRAM_INVALID_CONFIGURATIONS	0
#define XSK_BBRAM_ALL_CONFIGURATIONS		1

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/**
 * Following defines should be given in decimal/hexa-decimal values.
 * These are to be defined for Ultrascale Microblaze
 * AXI GPIO pin numbers connected to MASTER JTAG primitive and corresponding
 * channel numbers for GPIO pins
 */
/* GPIO device ID */
#ifndef SDT
#define XSK_BBRAM_AXI_GPIO_DEVICE_ID	XPAR_AXI_GPIO_0_DEVICE_ID
#else
#define XSK_BBRAM_AXI_GPIO_DEVICE_ID	XPAR_XGPIO_0_BASEADDR
#endif

#define	XSK_BBRAM_AXI_GPIO_JTAG_TDO	(0)	/**< MASTER JTAG GPIO
						  *  pin for TDO */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TDI	(0)	/**< MASTER JTAG GPIO
						  *  pin for TDI */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TMS	(1)	/**< MASTER JTAG GPIO
						  *  pin for TMS */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TCK	(2)	/**< MASTER JTAG GPIO
						  *  pin for TCK */

#define	XSK_BBRAM_GPIO_INPUT_CH		(2)	/**< GPIO Channel of TDO
						  *  pin connected */
#define	XSK_BBRAM_GPIO_OUTPUT_CH	(1)	/**< GPIO Channel of TDI,
						  *  TMS and TCK pin
						  *  connected */

/*
 * If following is TRUE Obfuscated key will be programmed
 * and DPA macros will be ignored.
 * otherwise AES key will be programmed and DPA configurations will
 * be considered
 */
#define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0		FALSE
									/**< TRUE burns obfuscated key */
#define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1		FALSE
									/**< TRUE burns obfuscated key */
#define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2		FALSE
									/**< TRUE burns obfuscated key */
#define XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3		FALSE
									/**< TRUE burns obfuscated key */

/**
 * If the following is TRUE BBRAM Key in corresponding
 * XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_INDEX is programmed
 * Returns error if the target doesn't have enabled SLR
 */
#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_0	FALSE
									/**< TRUE burns BBRAM key */
#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_1	FALSE
									/**< TRUE burns BBRAM key */
#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_2	FALSE
									/**< TRUE burns BBRAM key */
#define XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_3	FALSE
									/**< TRUE burns BBRAM key */
/**
 *
 * This is the 256 bit key to be programmed into BBRAM.
 * This should entered by user in HEX.
 */
#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"
#define		XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3	"b1c276899d71fb4cdd4a0a7905ea46c2e11f9574d09c7ea23b70b67de713ccd1"

#define		XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_0			"0000000000000000524156a63950bcedafeadcdeabaadee34216615aaaabbaaa"
#define		XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_1			"0000000000000000524156a63950bcedafeadcdeabaadee34216615aaaabbaaa"
#define		XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_2			"0000000000000000524156a63950bcedafeadcdeabaadee34216615aaaabbaaa"
#define		XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_3			"0000000000000000524156a63950bcedafeadcdeabaadee34216615aaaabbaaa"

#define		XSK_BBRAM_AES_KEY_SIZE_IN_BITS	256

/*
* If XSK_BBRAM_PGM_OBFUSCATED_KEY FALSE user can set below macros.
* else this values will be ignored.
*/
#define XSK_BBRAM_DPA_PROTECT_ENABLE	FALSE

/* If DPA protect is enabled */
#if (XSK_BBRAM_DPA_PROTECT_ENABLE == TRUE)
#define XSK_BBRAM_DPA_COUNT		0
#define XSK_BBRAM_DPA_MODE		XSK_BBRAM_INVALID_CONFIGURATIONS
#endif

/*
 * End of definitions for BBRAM
 */

/************************** Function Prototypes *****************************/
/****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif	/*XILSKEY_INPUT_H*/
