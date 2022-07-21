/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_config.h
*
* This file contains user configuration for PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_CONFIG_H_
#define XPSMFW_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_plat.h"

/*Check if UART is present in design */
#if defined (STDOUT_BASEADDRESS)
/*Check if MDM uart or PS Uart */
#if (STDOUT_BASEADDRESS == 0xF0310000U)
#define DEBUG_PMC_UART_MDM
#elif (STDOUT_BASEADDRESS == 0xFFCF0000U)
#define DEBUG_PSM_UART_MDM
#elif ((STDOUT_BASEADDRESS == UART0_BASEADDR) || (STDOUT_BASEADDRESS == UART1_BASEADDR))
#define DEBUG_UART_PS
#endif
#endif

/************* User Configurable Options ***************/
/* PSMFW print levels */
#if defined (DEBUG_UART_PS)
#define XPSMFW_PRINT_ALWAYS_VAL		(1U)
#else
#define XPSMFW_PRINT_ALWAYS_VAL		(0U)
#endif
#define XPSMFW_DEBUG_ERROR_VAL		(0U)
#define XPSMFW_DEBUG_DETAILED_VAL	(0U)

/**
 * PSMFW Debug options
 */

#if XPSMFW_PRINT_ALWAYS_VAL
#define XPSMFW_PRINT_ALWAYS
#endif

#if XPSMFW_DEBUG_ERROR_VAL
#define XPSMFW_DEBUG_ERROR
#endif

#if XPSMFW_DEBUG_DETAILED_VAL
#define XPSMFW_DEBUG_DETAILED
#endif

/*
 * PSM Firmware code include options
 *
 * PSM Firmware by default disables some functionality
 * Here is the list of all the build flags with the default options.
 * User can modify these flags to enable or disable any functionality
 */

/* TODO: List all PSM build flags here */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_CONFIG_H_ */
