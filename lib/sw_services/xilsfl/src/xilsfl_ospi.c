/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 * @file xilsfl_ospi.c
 * @brief Implements the OSPI controller interface of the SFL library.
 *
 * This file implements the functions required to use the OSPIPS hardware to
 * perform a transfer. The serial flash library (SFL) uses these functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24 First release
 * 1.0   sb  9/25/24 Update XSfl_OspiTransfer api to support unaligned bytes read
 *                   and add support for non-blocking read
 * 1.1   sb  01/28/25  Add support to read in stig when DMA is not available.
 * 1.1   sb  02/11/25  Add support for x2/x4 operation.
 * 1.4   sb  09/01/26  Fix stale flash message fields.
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

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
u8 Index=0; /**< Index of the XOspiPsv instance. */
XOspiPsv OspiInstance; /**< OSPI driver instance. */

/*****************************************************************************/
/**
 * @brief Configures the TX and RX DLL delay.
 *
 * Based on the mode and reference clock, this API calculates the RX delay and
 * configures it in the PHY configuration register.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if it fails.
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
 * @brief Selects the specific flash device.
 *
 * Call this API at least once in the application. If desired, call it multiple
 * times when switching between communicating with different flash devices or
 * using different configurations.
 *
 * @param[in]   ChipSelNum Flash chip select.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if it fails to set.
 *
 * @note        If the application does not call this function, the driver
 *              assumes that a single flash is connected to the lower bus and
 *              CS line.
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
 * @brief Configures the OSPI controller.
 *
 * @param[in]   Mode Edge mode. XOSPIPSV_EDGE_MODE_* represents valid values.
 * @param[in]   DualByteOpCode 1 or 2 to enable the dual-byte opcode, 0 to
 *              disable it.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if it fails to set.
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
 * @brief Prepares the flash message to transfer to the driver.
 *
 * @param[in]   Index XOspiPsv instance index in the instance array.
 * @param[in]   SflMsg Pointer to the structure containing transfer data.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if it fails to set.
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

	FlashMsg.RxAddr64bit = SflMsg->RxAddr64bit;
	FlashMsg.Xfer64bit = SflMsg->Xfer64bit;

	if(FlashMsg.RxBfrPtr != NULL){
		FlashMsg.Dummy = SflMsg->Dummy + OspiInstance.Extra_DummyCycle;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	} else if(FlashMsg.RxBfrPtr == NULL) {
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	}

	FlashMsg.ExtendedOpcode = SflMsg->DualByteOpCode;

	Status = XOspiPsv_PollTransfer(&OspiInstance, &FlashMsg);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief Prepares the flash message for a nonblocking transfer.
 *
 * @param[in]   Index XOspiPsv instance index in the instance array.
 * @param[in]   SflMsg Pointer to the structure containing transfer data.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if it fails to set.
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

	FlashMsg.RxAddr64bit = SflMsg->RxAddr64bit;
	FlashMsg.Xfer64bit = SflMsg->Xfer64bit;

	if(FlashMsg.RxBfrPtr != NULL){
		FlashMsg.Dummy = SflMsg->Dummy + OspiInstance.Extra_DummyCycle;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	} else if(FlashMsg.RxBfrPtr == NULL) {
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	}

	FlashMsg.ExtendedOpcode = SflMsg->DualByteOpCode;

	Status = XOspiPsv_StartDmaTransfer(&OspiInstance, &FlashMsg);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}
/*****************************************************************************/
/**
 * @brief Checks for DMA transfer completion.
 *
 * @param[in]   Index Array index of the XOspiPsv instance.
 *
 * @return
 *              - XST_SUCCESS if the DMA transfer is complete.
 *              - XST_FAILURE if the DMA transfer is not complete.
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
 * @brief Resets the OSPI flash device.
 *
 * @param[in]   Type Reset type.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE if reset fails.
 *
 ******************************************************************************/
u32 XSfl_OspiDeviceRest(u8 Type) {
	u32 Status;

#if defined (versal) && !defined (VERSAL_2VE_2VM)
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
 * @brief Initializes an XOspiPsv instance so that the driver is ready to use.
 *
 * @note        See xilsfl_control.h, where the parameters and return values
 *              are documented with the prototype.
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

#if defined (versal) && !defined (VERSAL_2VE_2VM)
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
	Ptr->CntrlInfo.BusWidth = OspiInstance.Config.BusWidth;
	Ptr->Quirks = UserConfig->Ospi_Config.Quirks;
	Index++;

	return Status;
}
#endif
/** @} */
