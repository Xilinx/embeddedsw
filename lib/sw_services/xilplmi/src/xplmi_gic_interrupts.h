/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
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
#include "pmc_global.h"
#include "xplmi_ipi.h"

/************************** Constant Definitions *****************************/
#define XPLMI_GICP_IRQ_STATUS				0xF11400A0
#define XPLMI_GICP0_IRQ_STATUS				0xF1140000
#define XPLMI_GICP0_IRQ_MASK				0xF1140004
#define XPLMI_GICP_SOURCE_COUNT				0x5
#define XPLMI_NO_OF_BITS_IN_REG				32U

#define XPLMI_GICP_MASK			(0xFF00U)
#define XPLMI_GICPX_MASK		(0xFF0000U)

/**
 * PMC GIC interrupts
 */
enum {
	XPLMI_PMC_GIC_IRQ_GICP0 = 0,
	XPLMI_PMC_GIC_IRQ_GICP1,
	XPLMI_PMC_GIC_IRQ_GICP2,
	XPLMI_PMC_GIC_IRQ_GICP3,
	XPLMI_PMC_GIC_IRQ_GICP4,
};

/**
 * PMC GICP0 interrupts
 */
enum {
	XPLMI_GICP0_SRC13 = 13, /**< GPIO Interrupt */
	XPLMI_GICP0_SRC14 = 14, /**< I2C_0 Interrupt */
	XPLMI_GICP0_SRC15 = 15, /**< I2C_1 Interrupt */
	XPLMI_GICP0_SRC16 = 16, /**< SPI_0 Interrupt */
	XPLMI_GICP0_SRC17 = 17, /**< SPI_1 Interrupt */
	XPLMI_GICP0_SRC18 = 18, /**< UART_0 Interrupt */
	XPLMI_GICP0_SRC19 = 19, /**< UART_1 Interrupt */
	XPLMI_GICP0_SRC20 = 20, /**< CAN_0 Interrupt */
	XPLMI_GICP0_SRC21 = 21, /**< CAN_1 Interrupt */
	XPLMI_GICP0_SRC22 = 22, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC23 = 23, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC24 = 24, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC25 = 25, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC26 = 26, /**< USB_0 Interrupt */
	XPLMI_GICP0_SRC27 = 27, /**< IPI Interrupt */
};

/**
 * PMC GICP1 interrupts
 */
enum {
	XPLMI_GICP1_SRC5 = 5, /**< TTC_0 Interrupt */
	XPLMI_GICP1_SRC6 = 6, /**< TTC_0 Interrupt */
	XPLMI_GICP1_SRC7 = 7, /**< TTC_0 Interrupt */
	XPLMI_GICP1_SRC8 = 8, /**< TTC_1 Interrupt */
	XPLMI_GICP1_SRC9 = 9, /**< TTC_1 Interrupt */
	XPLMI_GICP1_SRC10 = 10, /**< TTC_1 Interrupt */
	XPLMI_GICP1_SRC11 = 11, /**< TTC_2 Interrupt */
	XPLMI_GICP1_SRC12 = 12, /**< TTC_2 Interrupt */
	XPLMI_GICP1_SRC13 = 13, /**< TTC_2 Interrupt */
	XPLMI_GICP1_SRC14 = 14, /**< TTC_3 Interrupt */
	XPLMI_GICP1_SRC15 = 15, /**< TTC_3 Interrupt */
	XPLMI_GICP1_SRC16 = 16, /**< TTC_3 Interrupt */
	XPLMI_GICP1_SRC24 = 24, /**< GEM_0 Interrupt */
	XPLMI_GICP1_SRC25 = 25, /**< GEM_0 Interrupt */
	XPLMI_GICP1_SRC26 = 26, /**< GEM_1 Interrupt */
	XPLMI_GICP1_SRC27 = 27, /**< GEM_1 Interrupt */
	XPLMI_GICP1_SRC28 = 28, /**< ADMA_0 Interrupt */
	XPLMI_GICP1_SRC29 = 29, /**< ADMA_1 Interrupt */
	XPLMI_GICP1_SRC30 = 30, /**< ADMA_2 Interrupt */
	XPLMI_GICP1_SRC31 = 31, /**< ADMA_3 Interrupt */
};

/**
 * PMC GICP2 interrupts
 */
enum {
	XPLMI_GICP2_SRC0 = 0, /**< ADMA_4 Interrupt */
	XPLMI_GICP2_SRC1 = 1, /**< ADMA_5 Interrupt */
	XPLMI_GICP2_SRC2 = 2, /**< ADMA_6 Interrupt */
	XPLMI_GICP2_SRC3 = 3, /**< ADMA_7 Interrupt */
	XPLMI_GICP2_SRC10 = 10, /**< USB_0 Interrupt */
};

/**
 * PMC GICP3 interrupts
 */
enum {
	XPLMI_GICP3_SRC30 = 30, /**< SD_0 Interrupt */
	XPLMI_GICP3_SRC31 = 31, /**< SD_0 Interrupt */
};

/**
 * PMC GICP4 interrupts
 */
enum {
	XPLMI_GICP4_SRC0 = 0, /**< SD_1 Interrupt */
	XPLMI_GICP4_SRC1 = 1, /**< SD_1 Interrupt */
	XPLMI_GICP4_SRC8 = 8, /**< SBI interrupt */
	XPLMI_GICP4_SRC14 = 14, /**< RTC interrupt */
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
int XPlmi_DispatchWakeHandler(void *DeviceIdx);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_GIC_INTERRUPTS_H */
