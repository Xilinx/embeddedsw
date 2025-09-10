/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpm_init.c
* @addtogroup xpm_init xpm APIs
*
* This file contains the xpm node data and API's.
* @{
* @details
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
*  1.0  gm      14/06/23 Initial release.
*  9.4  ml      10/09/25 Added SDT support
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "bspconfig.h"
#if defined  (XPM_SUPPORT)
#include "xil_assert.h"
#include "xparameters.h"
#include "pm_api_sys.h"
#include "xpm_nodeid.h"
#include "xstatus.h"
#include "xpm_init.h"

/************************** Constant Definitions *****************************/
#ifndef SDT
#define XPMU_IPI_CHANNEL_ID      XPAR_XIPIPSU_0_DEVICE_ID
#else
#define XPMU_IPI_CHANNEL_ID      XPAR_XIPIPSU_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

static XIpiPsu IpiInst;
/************************** Function Prototypes ******************************/

/************************** Global Variables ********************************/

#ifdef VERSAL_NET
XpmNodeInfo XpmNodeData[] = {
	/* { Base Address, Node ID, Reset ID } */
	{ 0xF1B00000U, PM_DEV_USB_0, PM_RST_USB_0 },		/* USB_0 */
	{ 0xF1C00000U, PM_DEV_USB_1, PM_RST_USB_1 },		/* USB_1 */
	{ 0xF19E0000U, PM_DEV_GEM_0, PM_RST_GEM_0 },		/* GEM_0 */
	{ 0xF19F0000U, PM_DEV_GEM_1, PM_RST_GEM_1 },		/* GEM_1 */
	{ 0xF1960000U, PM_DEV_SPI_0, PM_RST_SPI_0 },		/* SPI_0 */
	{ 0xF1970000U, PM_DEV_SPI_1, PM_RST_SPI_1 },		/* SPI_1 */
	{ 0xF1940000U, PM_DEV_I2C_0, PM_RST_I2C_0 },		/* I2C_0 */
	{ 0xF1950000U, PM_DEV_I2C_1, PM_RST_I2C_1 },		/* I2C_1 */
	{ 0xF1980000U, PM_DEV_CAN_FD_0, PM_RST_CAN_FD_0 },	/* CAN_FD_0 */
	{ 0xF1990000U, PM_DEV_CAN_FD_1, PM_RST_CAN_FD_1 },	/* CAN_FD_1 */
	{ 0xF1920000U, PM_DEV_UART_0, PM_RST_UART_0 },		/* UART_0 */
	{ 0xF1930000U, PM_DEV_UART_1, PM_RST_UART_1 },		/* UART_1 */
	{ 0xF19D0000U, PM_DEV_GPIO, PM_RST_GPIO_LPD },		/* GPIO */
	{ 0xF1DC0000U, PM_DEV_TTC_0, PM_RST_TTC_0 },		/* TTC_0 */
	{ 0xF1DD0000U, PM_DEV_TTC_1, PM_RST_TTC_1 },		/* TTC_1 */
	{ 0xF1DE0000U, PM_DEV_TTC_2, PM_RST_TTC_2 },		/* TTC_2 */
	{ 0xF1DF0000U, PM_DEV_TTC_3, PM_RST_TTC_3 },		/* TTC_3 */
	{ 0xEA420000U, PM_DEV_LPD_SWDT_0, PM_RST_SWDT_0 },	/* LPD_SWDT_0 */
	{ 0xEA430000U, PM_DEV_LPD_SWDT_1, PM_RST_SWDT_1 },	/* LPD_SWDT_1 */
	{ 0xECC10000U, PM_DEV_FPD_SWDT_0, PM_RST_FPD_SWDT_0 },	/* FPD_SWDT_0 */
	{ 0xECD10000U, PM_DEV_FPD_SWDT_1, PM_RST_FPD_SWDT_1 },	/* FPD_SWDT_1 */
	{ 0xECE10000U, PM_DEV_FPD_SWDT_2, PM_RST_FPD_SWDT_2 },	/* FPD_SWDT_2 */
	{ 0xECF10000U, PM_DEV_FPD_SWDT_3, PM_RST_FPD_SWDT_3 },	/* FPD_SWDT_3 */
	{ 0xF1010000U, PM_DEV_OSPI, PM_RST_OSPI },		/* OSPI */
	{ 0xF1030000U, PM_DEV_QSPI, PM_RST_QSPI },		/* QSPI */
	{ 0xF1020000U, PM_DEV_GPIO_PMC, PM_RST_GPIO_PMC },	/* GPIO_PMC */
	{ 0xF1000000U, PM_DEV_I2C_PMC, PM_RST_I2C_PMC },	/* I2C_PMC */
	{ 0xF1040000U, PM_DEV_SDIO_0, PM_RST_SDIO_0 },		/* SDIO_0 */
	{ 0xF1050000U, PM_DEV_SDIO_1, PM_RST_SDIO_1 },		/* EMMC */
	{ 0xEBD00000U, PM_DEV_ADMA_0, PM_RST_ADMA },		/* ADMA_0 */
	{ 0xEBD10000U, PM_DEV_ADMA_1, PM_RST_ADMA },		/* ADMA_1 */
	{ 0xEBD20000U, PM_DEV_ADMA_2, PM_RST_ADMA },		/* ADMA_2 */
	{ 0xEBD30000U, PM_DEV_ADMA_3, PM_RST_ADMA },		/* ADMA_3 */
	{ 0xEBD40000U, PM_DEV_ADMA_4, PM_RST_ADMA },		/* ADMA_4 */
	{ 0xEBD50000U, PM_DEV_ADMA_5, PM_RST_ADMA },		/* ADMA_5 */
	{ 0xEBD60000U, PM_DEV_ADMA_6, PM_RST_ADMA },		/* ADMA_6 */
	{ 0xEBD70000U, PM_DEV_ADMA_7, PM_RST_ADMA },		/* ADMA_7 */
	{ 0xEB300000U, PM_DEV_IPI_0, PM_RST_IPI },		/* IPI */
};
#elif defined(versal)
XpmNodeInfo XpmNodeData[] = {
	/* { Base Address, Node ID, Reset ID } */
	{ 0xFE200000U, PM_DEV_USB_0, PM_RST_USB_0 },		/* USB_0 */
	{ 0xFF0C0000U, PM_DEV_GEM_0, PM_RST_GEM_0 },		/* GEM_0 */
	{ 0xFF0D0000U, PM_DEV_GEM_1, PM_RST_GEM_1 },		/* GEM_1 */
	{ 0xFF040000U, PM_DEV_SPI_0, PM_RST_SPI_0 },		/* SPI_0 */
	{ 0xFF050000U, PM_DEV_SPI_1, PM_RST_SPI_1 },		/* SPI_1 */
	{ 0xFF020000U, PM_DEV_I2C_0, PM_RST_I2C_0 },		/* I2C_0 */
	{ 0xFF030000U, PM_DEV_I2C_1, PM_RST_I2C_1 },		/* I2C_1 */
	{ 0xFF060000U, PM_DEV_CAN_FD_0, PM_RST_CAN_FD_0 },	/* CAN_FD_0 */
	{ 0xFF070000U, PM_DEV_CAN_FD_1, PM_RST_CAN_FD_1 },	/* CAN_FD_1 */
	{ 0xFF000000U, PM_DEV_UART_0, PM_RST_UART_0 },		/* UART_0 */
	{ 0xFF010000U, PM_DEV_UART_1, PM_RST_UART_1 },		/* UART_1 */
	{ 0xFF0B0000U, PM_DEV_GPIO, PM_RST_GPIO_LPD },		/* GPIO */
	{ 0xFF0E0000U, PM_DEV_TTC_0, PM_RST_TTC_0 },		/* TTC_0 */
	{ 0xFF0F0000U, PM_DEV_TTC_1, PM_RST_TTC_1 },		/* TTC_1 */
	{ 0xFF100000U, PM_DEV_TTC_2, PM_RST_TTC_2 },		/* TTC_2 */
	{ 0xFF110000U, PM_DEV_TTC_3, PM_RST_TTC_3 },		/* TTC_3 */
	{ 0xFF120000U, PM_DEV_SWDT_LPD, PM_RST_SWDT_LPD },	/* SWDT_LPD */
	{ 0xFD4D0000U, PM_DEV_SWDT_FPD, PM_RST_SWDT_FPD },	/* SWDT_FPD */
	{ 0xF1010000U, PM_DEV_OSPI, PM_RST_OSPI },		/* OSPI */
	{ 0xF1030000U, PM_DEV_QSPI, PM_RST_QSPI },		/* QSPI */
	{ 0xF1020000U, PM_DEV_GPIO_PMC, PM_RST_GPIO_PMC },	/* GPIO_PMC */
	{ 0xF1000000U, PM_DEV_I2C_PMC, PM_RST_I2C_PMC },	/* I2C_PMC */
	{ 0xF1040000U, PM_DEV_SDIO_0, PM_RST_SDIO_0 },		/* SDIO_0 */
	{ 0xF1050000U, PM_DEV_SDIO_1, PM_RST_SDIO_1 },		/* SDIO_1 */
	{ 0xFFA80000U, PM_DEV_ADMA_0, PM_RST_ADMA },		/* ADMA_0 */
	{ 0xFFA90000U, PM_DEV_ADMA_1, PM_RST_ADMA },		/* ADMA_1 */
	{ 0xFFAA0000U, PM_DEV_ADMA_2, PM_RST_ADMA },		/* ADMA_2 */
	{ 0xFFAB0000U, PM_DEV_ADMA_3, PM_RST_ADMA },		/* ADMA_3 */
	{ 0xFFAC0000U, PM_DEV_ADMA_4, PM_RST_ADMA },		/* ADMA_4 */
	{ 0xFFAD0000U, PM_DEV_ADMA_5, PM_RST_ADMA },		/* ADMA_5 */
	{ 0xFFAE0000U, PM_DEV_ADMA_6, PM_RST_ADMA },		/* ADMA_6 */
	{ 0xFFAF0000U, PM_DEV_ADMA_7, PM_RST_ADMA },		/* ADMA_7 */
	{ 0xFF300000U, PM_DEV_IPI_0, PM_RST_IPI },		/* IPI */
};
#endif

/****************************************************************************/
/**
 * *
 * * @brief     This API is used to provide the node id.
 * *
 * * @return    Node ID if successful, otherwise XST_FAILURE.
 * *
 * * @note      none
 * *
 * *****************************************************************************/
UINTPTR XpmGetNodeId(UINTPTR BaseAddress)
{
	u32 id;

	for (id = 0; id < MAX_NODE_COUNT; id++) {
		if (BaseAddress == XpmNodeData[id].BaseAddress) {
			return XpmNodeData[id].NodeId;
		}
	}
	return (UINTPTR)XST_FAILURE;
}

/****************************************************************************/
/**
 * *
 * * @brief     This API is used to provide the reset id.
 * *
 * * @return    Reset ID if successful, otherwise XST_FAILURE.
 * *
 * * @note      none
 * *
 * *****************************************************************************/
UINTPTR XpmGetResetId(UINTPTR BaseAddress)
{
	u32 id;

	for (id = 0; id < MAX_NODE_COUNT; id++) {
		if (BaseAddress == XpmNodeData[id].BaseAddress) {
			return XpmNodeData[id].ResetId;
		}
	}
	return (UINTPTR)XST_FAILURE;
}

static XStatus XpmIpiConfig(XIpiPsu *const IpiInst)
{
	XStatus status;
	XIpiPsu_Config *IpiCfgPtr;

	Xil_AssertNonvoid(IpiInst != NULL);

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPMU_IPI_CHANNEL_ID);
	if (NULL == IpiCfgPtr) {
		status = XST_FAILURE;
		xil_printf("%s ERROR in getting CfgPtr\n", __func__);
		return status;
	}

	/* Init with the Cfg Data */
	status = XIpiPsu_CfgInitialize(IpiInst, IpiCfgPtr,
				       IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != status) {
		xil_printf("%s ERROR #%d in configuring IPI\n", __func__, status);
	}
	return status;
}

/****************************************************************************/
/**
 * *
 * * This API initializes the ipi and xilpm through constructor.
 * *
 * * @return           none
 * *
 * * @note             none
 * *
 * *****************************************************************************/
void __attribute__ ((constructor)) xpminit()
{
	XStatus status;

	status = XpmIpiConfig(&IpiInst);
	if (XST_SUCCESS != status) {
		xil_printf("IPI configuration failed.\n");
	}

	status = XPm_InitXilpm(&IpiInst);
	if (XST_SUCCESS != status) {
		xil_printf("Xilpm library initialization failed.\n");
	}
}
#endif
