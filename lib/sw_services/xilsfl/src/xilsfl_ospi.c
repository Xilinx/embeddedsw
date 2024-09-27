/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_ospi.c
 * @addtogroup xilsfl overview
 * @{
 *
 * The xilsfl_ospi.c file implements the functions required to use the OSPIPS hardware to
 * perform a transfer. These are used by sfl library.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24 First release
 * 1.0   sb  9/25/24 Update XSfl_OspiTransfer api to support unaligned bytes read
 *                   and add support for non-blocking read
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"	/* SDK generated parameters */

#ifdef XPAR_XOSPIPSV_NUM_INSTANCES
#include "xospipsv.h"
#include "xilsfl.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XSfl_OspiRxTuning();
u32 XSfl_OspiDeviceRest(u8 Type);
u32 XSfl_OspiSelectFlash(u8 ChipSelNum);
u32 XSfl_OspiSetSdrDdr(u8 Mode, u8 DualByteOpCode);
u32 XSfl_OspiTransferDmaDone(u8 Index);
u32 XSfl_OspiNonBlockingTransfer(u8 Index, XSfl_Msg *SflMsg) ;

/************************** Variable Definitions *****************************/
u8 Index=0;
XOspiPsv OspiInstance;

/*****************************************************************************/
/**
 * @brief
 * Configures TX and RX DLL Delay. Based on the mode and reference clock
 * this API calculates the RX delay and configure them in PHY configuration
 * register.
 *
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if fails.
 *
 ******************************************************************************/
u32 XSfl_OspiRxTuning(){
	u32 Status;
	Status = XOspiPsv_SetDllDelay(&OspiInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * Used to select the specific flash device
 * This API should be called at least once in the
 * application. If desired, it can be called multiple times when switching
 * between communicating to different flash devices/using different configuration.
 *
 * @param	ChipSelect Flash Chip Select.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if fail to set.
 * @note
 * 		If this function is not called at least once in the application,
 *		the driver assumes there is a single flash connected to the
 *		lower bus and CS line.
 *
 ******************************************************************************/
u32 XSfl_OspiSelectFlash(u8 ChipSelNum){
	u32 Status;

	Status = XOspiPsv_SelectFlash(&OspiInstance, ChipSelNum);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * Configures the Ospi controller.
 *
 * @param	Mode Edge mode. XOSPIPSV_EDGE_MODE_* represents valid values.
 * @param	DualByteOpCode (1/2) to enable dual-byte opcode, 0 to disable.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if fail to set.
 *
 ******************************************************************************/
u32 XSfl_OspiSetSdrDdr(u8 Mode, u8 DualByteOpCode){

	u32 Status;
	Status = XOspiPsv_ConfigDualByteOpcode(&OspiInstance, DualByteOpCode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (OspiInstance.OpMode == XOSPIPSV_DAC_EN_OPTION ) {
		XOspiPsv_ConfigureAutoPolling(&OspiInstance, Mode);
	}

	Status = XOspiPsv_SetSdrDdrMode(&OspiInstance, Mode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * Prepares the Flash message to transfer to the driver.
 *
 * @param	Index is the XOspiPsv instance index in the instance array.
 * @param	SflMsg Pointer to the structure containing transfer data.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if fail to set.
 *
 ******************************************************************************/
u32 XSfl_OspiTransfer(u8 Index, XSfl_Msg *SflMsg) {

	/* Validate input parameters */
	Xil_AssertNonvoid(SflMsg != NULL);
	(void)Index;

	u32 Status ;
	static XOspiPsv_Msg FlashMsg;

	FlashMsg.Opcode = SflMsg->Opcode;
	FlashMsg.TxBfrPtr = SflMsg->TxBfrPtr;
	FlashMsg.RxBfrPtr = SflMsg->RxBfrPtr;
	FlashMsg.ByteCount = SflMsg->ByteCount;
	FlashMsg.Dummy = SflMsg->Dummy;
	FlashMsg.Addr = SflMsg->Addr;
	FlashMsg.Addrsize = SflMsg->Addrsize;
	FlashMsg.Addrvalid = SflMsg->Addrvalid;
	FlashMsg.Proto = SflMsg->Proto;

        if(SflMsg->Xfer64bit) {
		FlashMsg.RxAddr64bit = SflMsg->RxAddr64bit;
                FlashMsg.Xfer64bit = SflMsg->Xfer64bit;
	}

	if(FlashMsg.RxBfrPtr != NULL){
		FlashMsg.Dummy = SflMsg->Dummy + OspiInstance.Extra_DummyCycle;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	} else if(FlashMsg.RxBfrPtr == NULL) {
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	}

	if (SflMsg->DualByteOpCode){
		FlashMsg.ExtendedOpcode = SflMsg->DualByteOpCode;
	}

	Status = XOspiPsv_PollTransfer(&OspiInstance, &FlashMsg);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * Prepares the Flash message for non blocking transfer.
 *
 * @param	Index is the XOspiPsv instance index in the instance array.
 * @param	SflMsg Pointer to the structure containing transfer data.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if fail to set.
 *
 ******************************************************************************/
u32 XSfl_OspiNonBlockingTransfer(u8 Index, XSfl_Msg *SflMsg) {

	/* Validate input parameters */
	Xil_AssertNonvoid(SflMsg != NULL);
	(void)Index;

	u32 Status ;
	static XOspiPsv_Msg FlashMsg;

	FlashMsg.Opcode = SflMsg->Opcode;
	FlashMsg.TxBfrPtr = SflMsg->TxBfrPtr;
	FlashMsg.RxBfrPtr = SflMsg->RxBfrPtr;
	FlashMsg.ByteCount = SflMsg->ByteCount;
	FlashMsg.Dummy = SflMsg->Dummy;
	FlashMsg.Addr = SflMsg->Addr;
	FlashMsg.Addrsize = SflMsg->Addrsize;
	FlashMsg.Addrvalid = SflMsg->Addrvalid;
	FlashMsg.Proto = SflMsg->Proto;

        if(SflMsg->Xfer64bit) {
		FlashMsg.RxAddr64bit = SflMsg->RxAddr64bit;
                FlashMsg.Xfer64bit = SflMsg->Xfer64bit;
	}

	if(FlashMsg.RxBfrPtr != NULL){
		FlashMsg.Dummy = SflMsg->Dummy + OspiInstance.Extra_DummyCycle;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	} else if(FlashMsg.RxBfrPtr == NULL) {
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	}

	if (SflMsg->DualByteOpCode){
		FlashMsg.ExtendedOpcode = SflMsg->DualByteOpCode;
	}

	Status = XOspiPsv_StartDmaTransfer(&OspiInstance, &FlashMsg);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
/*****************************************************************************/
/**
 * @brief
 * Checks for DMA transfer complete.
 *
 * @param	array index of the XOspiPsv instance.
 *
 * @return
 *		- XST_SUCCESS if DMA transfer complete.
 *		- XST_FAILURE if DMA transfer is not completed.
 *
 ******************************************************************************/
u32 XSfl_OspiTransferDmaDone(u8 Index){
	u32 Status;
	(void)Index;

	Status = XOspiPsv_CheckDmaDone(&OspiInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function reset the OSPI flash device.
*
* @param	Type is Reset type.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if reset fails.
*
******************************************************************************/
u32 XSfl_OspiDeviceRest(u8 Type) {
	u32 Status;

#if defined (versal) && !defined (VERSAL_NET)
	Status = XOspiPsv_DeviceReset(Type);
#else
	Status = XOspiPsv_DeviceResetViaOspi(&OspiInstance, Type);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * Initializes a specific XOspiPsv instance so that the driver is ready to use.
 *
 * @param	Ptr is pointer to the Sfl_Interface instance.
 * @param	UserConfig the structure containing information
 *		    about a specific OSPI device. This function initializes an
 *		    Instance object for a specific device specified by the
 *		    contents of UserConfig.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if init fails.
 *
 ******************************************************************************/
u32 XSfl_OspiInit(XSfl_Interface *Ptr, const XSfl_UserConfig *UserConfig) {

	Xil_AssertNonvoid(Ptr != NULL);

	u32 Status = 0;
        u8 Prescalar;
	XOspiPsv_Config *OspiConfig;

	OspiConfig = XOspiPsv_LookupConfig(UserConfig->Ospi_Config.BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XOspiPsv_CfgInitialize(&OspiInstance, OspiConfig);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal) && !defined (VERSAL_NET)
	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
#else
	Status = XOspiPsv_DeviceResetViaOspi(&OspiInstance,
			XOSPIPSV_HWPIN_RESET);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable IDAC/DAC in OSPI controller
	 */
	Status = XOspiPsv_SetOptions(&OspiInstance, UserConfig->Ospi_Config.ReadMode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the prescaler for OSPIPSV clock
	 */
	Prescalar = (OspiInstance.Config.InputClockHz / XSFL_SDR_NON_PHY_MAX_FREQ);
	XOspiPsv_SetClkPrescaler(&OspiInstance, Prescalar);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Ptr->CntrlInfo.Transfer = XSfl_OspiTransfer;
	Ptr->CntrlInfo.NonBlockingTransfer = XSfl_OspiNonBlockingTransfer;
	Ptr->CntrlInfo.SelectFlash = XSfl_OspiSelectFlash;
	Ptr->CntrlInfo.SetSdrDdr = XSfl_OspiSetSdrDdr;
	Ptr->CntrlInfo.TransferDone = XSfl_OspiTransferDmaDone;
	Ptr->CntrlInfo.RxTunning = XSfl_OspiRxTuning;
	Ptr->CntrlInfo.DeviceReset = XSfl_OspiDeviceRest;

	Ptr->CntrlInfo.DeviceId = Index;
	Ptr->CntrlInfo.RefClockHz =  OspiInstance.Config.InputClockHz;
	Ptr->CntrlInfo.DeviceIdData = &OspiInstance.DeviceIdData;
	Ptr->SflFlashInfo.ConnectionMode = OspiInstance.Config.ConnectionMode;
	Ptr->CntrlInfo.OpMode = UserConfig->Ospi_Config.ReadMode;
	Ptr->CntrlInfo.ChipSelectNum = UserConfig->Ospi_Config.ChipSelect;
	Ptr->CntrlInfo.CntrlType = XSFL_OSPI_CNTRL;
	Index++;

	return Status;
}
#endif
/** @} */
