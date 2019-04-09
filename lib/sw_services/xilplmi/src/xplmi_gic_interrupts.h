/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_gic_interrupts.h
*
* This is the header file for xplm_gic_interrupts.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  mg   10/08/2018 Initial release
* 1.01  mg   04/02/2020 Remove defines which are already part of pmc_global.h
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_GIC_INTERRUPTS_H
#define XPLMI_GIC_INTERRUPTS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_ipi.h"

/************************** Constant Definitions *****************************/
#define XPLMI_GICP_SOURCE_COUNT				(0x5U)
#define XPLMI_NO_OF_BITS_IN_REG				(32U)

#define XPLMI_GICP_MASK			(0xFF00U)
#define XPLMI_GICPX_MASK		(0xFF0000U)
#define XPLMI_GICPX_LEN			(0x14U)

/*
 * PMC GIC interrupts
 */
enum {
	XPLMI_PMC_GIC_IRQ_GICP0 = 0U,
	XPLMI_PMC_GIC_IRQ_GICP1, /**< 1U */
	XPLMI_PMC_GIC_IRQ_GICP2, /**< 2U */
	XPLMI_PMC_GIC_IRQ_GICP3, /**< 3U */
	XPLMI_PMC_GIC_IRQ_GICP4, /**< 4U */
};

/*
 * PMC GICP0 interrupts
 */
enum {
	XPLMI_GICP0_SRC13 = 13U, /**< GPIO Interrupt */
	XPLMI_GICP0_SRC14 = 14U, /**< I2C_0 Interrupt */
	XPLMI_GICP0_SRC15 = 15U, /**< I2C_1 Interrupt */
	XPLMI_GICP0_SRC16 = 16U, /**< SPI_0 Interrupt */
	XPLMI_GICP0_SRC17 = 17U, /**< SPI_1 Interrupt */
	XPLMI_GICP0_SRC18 = 18U, /**< UART_0 Interrupt */
	XPLMI_GICP0_SRC19 = 19U, /**< UART_1 Interrupt */
	XPLMI_GICP0_SRC20 = 20U, /**< CAN_0 Interrupt */
	XPLMI_GICP0_SRC21 = 21U, /**< CAN_1 Interrupt */
	XPLMI_GICP0_SRC22 = 22U, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC23 = 23U, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC24 = 24U, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC25 = 25U, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC26 = 26U, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC27 = 27U, /**< IPI Interrupt */
};

/*
 * PMC GICP1 interrupts
 */
enum {
	XPLMI_GICP1_SRC5 = 5U, /**< TTC_0 Interrupt */
	XPLMI_GICP1_SRC6 = 6U, /**< TTC_0 Interrupt */
	XPLMI_GICP1_SRC7 = 7U, /**< TTC_0 Interrupt */
	XPLMI_GICP1_SRC8 = 8U, /**< TTC_1 Interrupt */
	XPLMI_GICP1_SRC9 = 9U, /**< TTC_1 Interrupt */
	XPLMI_GICP1_SRC10 = 10U, /**< TTC_1 Interrupt */
	XPLMI_GICP1_SRC11 = 11U, /**< TTC_2 Interrupt */
	XPLMI_GICP1_SRC12 = 12U, /**< TTC_2 Interrupt */
	XPLMI_GICP1_SRC13 = 13U, /**< TTC_2 Interrupt */
	XPLMI_GICP1_SRC14 = 14U, /**< TTC_3 Interrupt */
	XPLMI_GICP1_SRC15 = 15U, /**< TTC_3 Interrupt */
	XPLMI_GICP1_SRC16 = 16U, /**< TTC_3 Interrupt */
	XPLMI_GICP1_SRC24 = 24U, /**< GEM_0 Interrupt */
	XPLMI_GICP1_SRC25 = 25U, /**< GEM_0 Interrupt */
	XPLMI_GICP1_SRC26 = 26U, /**< GEM_1 Interrupt */
	XPLMI_GICP1_SRC27 = 27U, /**< GEM_1 Interrupt */
	XPLMI_GICP1_SRC28 = 28U, /**< ADMA_0 Interrupt */
	XPLMI_GICP1_SRC29 = 29U, /**< ADMA_1 Interrupt */
	XPLMI_GICP1_SRC30 = 30U, /**< ADMA_2 Interrupt */
	XPLMI_GICP1_SRC31 = 31U, /**< ADMA_3 Interrupt */
};

/*
 * PMC GICP2 interrupts
 */
enum {
	XPLMI_GICP2_SRC0 = 0U, /**< ADMA_4 Interrupt */
	XPLMI_GICP2_SRC1 = 1U, /**< ADMA_5 Interrupt */
	XPLMI_GICP2_SRC2 = 2U, /**< ADMA_6 Interrupt */
	XPLMI_GICP2_SRC3 = 3U, /**< ADMA_7 Interrupt */
	XPLMI_GICP2_SRC10 = 10U, /**< USB_0 Interrupt */
};

/*
 * PMC GICP3 interrupts
 */
enum {
	XPLMI_GICP3_SRC30 = 30U, /**< SD_0 Interrupt */
	XPLMI_GICP3_SRC31 = 31U, /**< SD_0 Interrupt */
};

/*
 * PMC GICP4 interrupts
 */
enum {
	XPLMI_GICP4_SRC0 = 0U, /**< SD_1 Interrupt */
	XPLMI_GICP4_SRC1 = 1U, /**< SD_1 Interrupt */
	XPLMI_GICP4_SRC8 = 8U, /**< SBI interrupt */
	XPLMI_GICP4_SRC14 = 14U, /**< RTC interrupt */
};

/**************************** Type Definitions *******************************/
/* Handler Table Structure */
typedef int (*Function_t)(void *Data);
struct GicIntrHandlerTable {
	void *Data;
	Function_t GicHandler;
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* Functions defined in xplm_main.c */
void XPlmi_GicIntrHandler(void *CallbackRef);
void XPlmi_GicRegisterHandler(u32 PlmIntrId, Function_t Handler, void *Data);
void XPlmi_GicIntrEnable(u32 PlmIntrId);
void XPlmi_GicIntrDisable(u32 PlmIntrId);
void XPlmi_GicIntrClearStatus(u32 PlmIntrId);
int XPlmi_DispatchWakeHandler(void *DeviceIdx);
void XPlmi_GicIntrAddTask(u32 Index);
int XPlmi_GicTaskHandler(void *Arg);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_GIC_INTERRUPTS_H */
