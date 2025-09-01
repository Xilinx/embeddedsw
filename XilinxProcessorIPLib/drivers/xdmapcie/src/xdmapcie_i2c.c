/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "stdio.h"
#include "xil_printf.h"
#include "sleep.h"

#if defined(XPAR_XIICPS_NUM_INSTANCES)
#include "xiicps.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
#ifndef  XPAR_XIICPS_1_DEVICE_ID
#define  XDMAPCIE_IIC_DEVICE_ID         -1
#else
#define  XDMAPCIE_IIC_DEVICE_ID              XPAR_XIICPS_1_DEVICE_ID
#endif

#else
#ifndef  XPAR_XIICPS_1_BASEADDR
#define  XDMAPCIE_IIC_BASEADDR          -1
#else
#define  XDMAPCIE_IIC_BASEADDR               XPAR_XIICPS_1_BASEADDR
#endif
#endif

/***************** Macros (Inline Functions) Definitions ********************/
/* Defines for IIC1 arbitrator */
#define XDMAPCIE_IIC1_ARBITRATOR_ADDR    (0x72U)
#define XDMAPCIE_IIC1_CMD_CONTROL        (0x01U)  /* Channel 0 for bus 8 */

/* Control register values (Master 0 example) */
#define XDMAPCIE_CTL_ACQUIRE_M0          (0x05U)

/* IIC1 IO Expander Registers */
#define XDMAPCIE_IIC1_IO_EXPANDER        (0x20U)
#define GPIO_OUTPUT_REG                  (0x02U)
#define GPIO_CONFIG_REG                  (0x06U)
#define GPIO_EXP_CFG_PIN7_OUTPUT         (0x7FU)

#define XDMAPCIE_RESET_MASK              (0x80U)

#define XDMAPCIE_PVPERL_MS               (100U)
#define XDMAPCIE_IIC_SCLK_RATE           (100000U)
#define XDMAPCIE_MAX_DELAY               (10000000U)

#define XDMAPCIE_IIC_NOT_PRESENT         (-1U)

/************************** Function Prototypes *****************************/
int XDmaPcie_IicExpanderReset(void);
/************************** Variable Definitions ****************************/
static XIicPs IicInstance = {0U};

/****************************************************************************/
/**
* Configure GPIO expander and perform PHY reset sequence
* Configure pin 7 as output
* De-assert reset (link down)
* Assert reset (link up)
*
* @param    None
* @return   XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*****************************************************************************/
int XDmaPcie_IicExpanderReset(void)
{
    int Status = XST_FAILURE;
    u8 Buffer[2U];
    u32 TimeOutCount;

    /* Configure arbitrator to select channel 0 */
    Buffer[0] = XDMAPCIE_IIC1_CMD_CONTROL;
    Buffer[1] = XDMAPCIE_CTL_ACQUIRE_M0;

    Status = XIicPs_MasterSendPolled(&IicInstance, Buffer, sizeof(Buffer),
				     XDMAPCIE_IIC1_ARBITRATOR_ADDR);
    if (Status != XST_SUCCESS) {
            xil_printf("IIC1 arbitrator configuration failed\n");
	goto fail;
    }

    /* Wait for bus idle */
    TimeOutCount = XDMAPCIE_MAX_DELAY;
    while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
        TimeOutCount--;
    }
    if (TimeOutCount == 0U) {
        xil_printf("ERROR: Bus timeout after arbitrator configuration\n");
	goto fail;
    }

    /* Configure pin 7 as output */
    Buffer[0] = GPIO_CONFIG_REG;
    Buffer[1] = GPIO_EXP_CFG_PIN7_OUTPUT;

    Status = XIicPs_MasterSendPolled(&IicInstance, &Buffer[0U],
                                    sizeof(Buffer), XDMAPCIE_IIC1_IO_EXPANDER);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: Pin configuration failed\n");
	goto fail;
    }

    /* Wait for bus idle */
    TimeOutCount = XDMAPCIE_MAX_DELAY;
    while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
        TimeOutCount--;
    }
    if (TimeOutCount == 0U) {
        xil_printf("ERROR: Bus timeout after pin configuration\n");
        goto fail;
    }

    /* Assert the reset signal */
    Buffer[0] = GPIO_OUTPUT_REG;
    Buffer[1] = 0x0;

    Status = XIicPs_MasterSendPolled(&IicInstance, &Buffer[0U],
                                    sizeof(Buffer), XDMAPCIE_IIC1_IO_EXPANDER);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: GPIO assert failed\n");
        goto fail;
    }

    /* Final bus idle wait */
    TimeOutCount = XDMAPCIE_MAX_DELAY;
    while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
        TimeOutCount--;
    }
    if (TimeOutCount == 0U) {
        xil_printf("ERROR: Bus timeout after assert\n");
        goto fail;
    }

    msleep(XDMAPCIE_PVPERL_MS);

    /* Deassert the reset signal */
    Buffer[0] = GPIO_OUTPUT_REG;
    Buffer[1] = XDMAPCIE_RESET_MASK;

    Status = XIicPs_MasterSendPolled(&IicInstance, &Buffer[0U],
                                    sizeof(Buffer), XDMAPCIE_IIC1_IO_EXPANDER);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: GPIO deassert failed\n");
        goto fail;
    }

    /* Final bus idle wait */
    TimeOutCount = XDMAPCIE_MAX_DELAY;
    while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
        TimeOutCount--;
    }
    if (TimeOutCount == 0U) {
        xil_printf("ERROR: Bus timeout after deassert\n");
        goto fail;
    }

    msleep(XDMAPCIE_PVPERL_MS);

    return XST_SUCCESS;

fail:
    return XST_FAILURE;
}
#endif
/****************************************************************************
* Initialize IIC controller and perform GPIO expander reset sequence
*
* @param    None
* @return   XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*****************************************************************************/
int XDmaPcie_IicInit(void)
{
    int Status = XST_FAILURE;

#if defined(XPAR_XIICPS_NUM_INSTANCES)
    XIicPs_Config *ConfigPtr;
    u32 TimeOutCount = XDMAPCIE_MAX_DELAY;

    /* Lookup IIC configuration */
#ifndef SDT
    if (XDMAPCIE_IIC_DEVICE_ID == XDMAPCIE_IIC_NOT_PRESENT)
        return XST_FAILURE;
    ConfigPtr = XIicPs_LookupConfig(XDMAPCIE_IIC_DEVICE_ID);
#else
    if (XDMAPCIE_IIC_BASEADDR == XDMAPCIE_IIC_NOT_PRESENT)
        return XST_FAILURE;
    ConfigPtr = XIicPs_LookupConfig(XDMAPCIE_IIC_BASEADDR);
#endif
    if (ConfigPtr == NULL) {
        xil_printf("ERROR: IIC config lookup failed\n");
        return XST_FAILURE;
    }

    /* Initialize IIC driver */
    Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr, ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: IIC driver initialization failed\n");
        return XST_FAILURE;
    }

    /* Set IIC clock rate FIRST (before any transactions) */
    Status = XIicPs_SetSClk(&IicInstance, XDMAPCIE_IIC_SCLK_RATE);
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: IIC clock setting failed\n");
        return XST_FAILURE;
    }

    /* Wait for bus to be idle after clock setting */
    while ((XIicPs_BusIsBusy(&IicInstance)) && (TimeOutCount > 0U)) {
        TimeOutCount--;
    }
    if (TimeOutCount == 0U) {
        xil_printf("ERROR: IIC bus busy timeout\n");
        return XST_FAILURE;
    }

    /* Perform IIC1 IO expander reset */
    Status = XDmaPcie_IicExpanderReset();
    if (Status != XST_SUCCESS) {
        xil_printf("ERROR: IIC1 IO expander reset failed\n");
        return XST_FAILURE;
    }
#endif

    return Status;
}
