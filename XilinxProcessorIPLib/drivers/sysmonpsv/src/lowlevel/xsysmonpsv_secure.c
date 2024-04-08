/******************************************************************************
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_secure.c
* @addtogroup sysmonpsv_api SYSMONPSV APIs
*
* The xsysmonpsv_secure.c file contains basic driver functions.
*
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 4.0   se	   11/10/22 Secure and Non-Secure mode integration
*
* </pre>
*
******************************************************************************/
#if defined(XSYSMONPSV_SECURE_MODE)
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsysmonpsv_secure.h"

/******************************************************************************/
/**
 * Functions as an Empty IPI Irq Handler.
 *
 * @param	Pointer Instance to XSysMonPsv.
 *
 * @return	None
 *
 * @note	XScuGic_Connect requires an irq handler.
 *
*******************************************************************************/
static void XSysMonPsv_IpiIrqHandler(XIpiPsu *InstancePtr)
{
	(void)InstancePtr;
}

/******************************************************************************/
/**
 * Configures IPI.
 *
 * @param	Pointer Instance to XScuGic.
 * @param	Pointer Instance to XIpiPsu.
 *
 * @return 	XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code.
 *
 *
*******************************************************************************/
static XStatus XSysMonPsv_IpiConfigure(XSysMonPsv *InstancePtr, XScuGic *const GicInst, XIpiPsu *const IpiInst)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	if (NULL == IpiInst) {
		goto done;
	}
	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(InstancePtr->IpiDeviceId);
	if (NULL == IpiCfgPtr) {
		Status = XST_FAILURE;
		xil_printf("%s ERROR in getting CfgPtr\n", __func__);
		goto done;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(IpiInst, IpiCfgPtr,
				       IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("%s ERROR #%d in configuring IPI\n", __func__,
			   Status);
		goto done;
	}

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(IpiInst, XIPIPSU_ALL_MASK);

	if (NULL == GicInst) {
		goto done;
	}
	Status = XScuGic_Connect(GicInst, InstancePtr->IpiIntrId,
				 (Xil_ExceptionHandler)XSysMonPsv_IpiIrqHandler, IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("%s ERROR #%d in GIC connect\n", __func__, Status);
		goto done;
	}
	/* Enable IPI interrupt at GIC */
	XScuGic_Enable(GicInst, InstancePtr->IpiIntrId);

done:

	return Status;
}

/******************************************************************************/
/**
 * Initializes the IPI.
 *
 * @param	Pointer Instance to XScuGic.
 * @param	Pointer Instance to XIpiPsu.
 *
 * @return 	XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code.
 *
 *
*******************************************************************************/
static XStatus XSysMonPsv_IpiInit(XSysMonPsv *InstancePtr, XScuGic *const GicInst, XIpiPsu *const IpiInst)
{
	int Status;

	Status = XSysMonPsv_IpiConfigure(InstancePtr, GicInst, IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("XSysMonPsv_IpiConfigure() failed with error: %d\r\n", Status);
		goto done;
	}

	/* FIXME: Enable reception of IPI from the SrcMask/CPU */
	XIpiPsu_InterruptEnable(IpiInst, XIPIPSU_ALL_MASK);

done:

	return Status;
}

/******************************************************************************/
/**
 * Initializes the XIL PM.
 *
 * @param	Pointer Instance to XScuGic.
 * @param	Pointer Instance to XIpiPsu.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code.
 *
 *
*******************************************************************************/
int XSysMonPsv_Xilpm_Init(XSysMonPsv *InstancePtr, XScuGic *const GicInst, XIpiPsu *const IpiInst)
{
	int Status;

	/* IPI Initialize */
	Status = XSysMonPsv_IpiInit(InstancePtr, GicInst, IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("XSysMonPsv_IpiInit() failed with error: %d\r\n", Status);
		goto done;
	}

	/* XilPM Initialize */
	Status = XPm_InitXilpm(IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("XPm_InitXilpm() failed with error: %d\r\n", Status);
		goto done;
	}

	/* Finalize Initialization */
	Status = XPm_InitFinalize();
	if (XST_SUCCESS != Status) {
		xil_printf("XPm_initfinalize() failed\r\n");
		goto done;
	}

done:
	return Status;
}

/******************************************************************************/
/**
 * Reads register value in secure mode.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	Offset Offset of the register.
 * @param   value Value to be read.
 *
 * @return	None
 *
 *
*******************************************************************************/
void XSysMonPsv_ReadReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 *Data)
{
	u32 Payload[3];
	(void)InstancePtr;

	Payload[0] = Offset;
	Payload[1] = XSYSMONPSV_SECURE_READ_DEFAULT;
	Payload[2] = XSYSMONPSV_SECURE_READ_DEFAULT;
	XPm_DevIoctl2(PM_DEV_AMS_ROOT, IOCTL_READ_REG, Payload,
			XSYSMONPSV_SECURE_DEFAULT_PAYLOAD_SIZE, Data, 1);
}

/******************************************************************************/
/**
 * Writes value to register in secure mode.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	Offset Offset of the register.
 * @param   value Value to be written in register.
 *
 * @return	None
 *
 *
*******************************************************************************/
void XSysMonPsv_WriteReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Data)
{

	u32 Payload[3];
	u32 Response[5];
	(void)InstancePtr;

	Payload[0] = Offset;
	Payload[1] = XSYSMONPSV_SECURE_WRITE_DEFAULT;
	Payload[2] = Data;
	XPm_DevIoctl2(PM_DEV_AMS_ROOT, IOCTL_MASK_WRITE_REG, Payload,
			XSYSMONPSV_SECURE_DEFAULT_PAYLOAD_SIZE, Response, 1);
}

/******************************************************************************/
/**
 * Updates register value in secure mode.
 *
 * @param	InstancePtr Pointer to the driver instance.
 * @param	Offset Offset addtress of the register.
 * @param	Mask Bits to be masked.
 * @param	Data value to be written.
 *
 * @return	None.
 *
*******************************************************************************/
void XSysMonPsv_UpdateReg32(XSysMonPsv *InstancePtr, u32 Offset, u32 Mask,
			   u32 Data)
{
	u32 Val;

	XSysMonPsv_ReadReg32(InstancePtr, Offset, &Val);
	XSysMonPsv_WriteReg32(InstancePtr, Offset,
			     (Val & ~Mask) | (Mask & Data));
}
#endif
