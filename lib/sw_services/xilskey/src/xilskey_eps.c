/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilskey_eps.c
 * This file contains the PS eFUSE API's to program/read the eFUSE array.
 *
 * @note	None.
 *
 *
 * MODIFICATION HISTORY:
 *
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added API to read status register.PR# 735957
* 2.00  hk      23/01/14 Changed PS efuse error codes for voltage out of range.
* 2.1   sk      04/03/15 Initialized RSAKeyReadback with Zeros CR# 829723.
* 3.00  vns     31/07/15 Removed redundant code to initialise timer.
* 4.00  vns     09/10/15 Added DFT control bits programming fecility for
*                        eFuse PS on Zynq. PR#862778
*                        Added Conditional compilation to support Zynq Mp
*                        also.
* 5.00  vns     27/01/16 Fixed array out of bounds error CR #931207
* 6.0   vns     29/06/16 Added Margin 2 read verification after programming
*                        every Zynq's eFUSE PS bit CR #953052.
*       vns     07/17/16 Fixed CR# 954260, Modified XilSKey_EfusePs_Write
*                        API to program eFUSE protect bit after programming
*                        DFT bits
* 6.6   vns     06/06/18 Added doxygen tags
* 6.8   vns     08/29/19 Initialized Status variables
* 7.2   am      07/13/21 Fixed doxygen warnings
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xilskey_utils.h"
#include "xilskey_epshw.h"
#include "xilskey_eps.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
u8 Matrix[31][5]; /**< PS eFUSE Matrix*/
u8 ErrorCodeIndex[32]; /**< Error Code Array */
/************************** Function Prototypes *****************************/
static u32 XilSKey_EfusePs_WriteWithXadcCheckAndVerify(u32 EfuseAddress, u32 RefClk);
static u32 XilSKey_EfusePs_WriteRsaKeyHash(u8 *RsaKeyHashBuf, u32 RefClk);
static u32 XilSKey_EfusePs_ReadRsaKeyHash(u8 *RsaKeyHashBuf, u32 RefClk);
static u32 XilSKey_EfusePs_ReadWithXadcCheck(u32 EfuseAddress, u32 RefClk, u8 *Data);
static u32 XilSKey_EfusePs_VerifyWithXadcCheck(u32 EfuseAddress, u32 RefClk);
static u32 XilSKey_EfusePs_CheckRsaHashForAllZeros(u32 RefClk);
static void XilSKey_EfusePs_SetControllerMode (u8 Mode);
static void XilSKey_EfusePs_SetControllerReadMode (u8 ReadMode);
static u8 XilSKey_EfusePs_IsReadModeSupported (u8 ReadMode);

/***************************************************************************/
/**
* This function is used to write to the PS eFUSE.
*
* @param	InstancePtr - Pointer to the PsEfuseHandle which describes
*                             which PS eFUSE bit should be burned.
*
* @return
* 		- XST_SUCCESS.
* 		- In case of error, value is as defined in xilskey_utils.h
* 		Error value is a combination of Upper 8 bit value and
* 		Lower 8 bit value. For example, 0x8A03 should be checked
* 		in error.h as 0x8A00 and 0x03. Upper 8 bit value signifies
* 		the major error and lower 8 bit values tells more precisely.
* @note		When called, this Initializes the timer, XADC subsystems.
*		Unlocks the PS eFUSE controller.Configures the PS eFUSE
*		controller. Writes the hash and control bits if requested.
*		Programs the PS eFUSE to enable the RSA authentication
*		if requested. Locks the PS eFUSE controller.
*		Returns an error, if the reference clock frequency is not
*		in between 20 and 60 MHz or if the system not in a position to
*		write the requested PS eFUSE bits (because the bits are already
*		written or not allowed to write) or if the temperature and
*		voltage are not within range
*
****************************************************************************/
u32 XilSKey_EfusePs_Write(XilSKey_EPs *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 StatusRedundantBit = (u32)XST_FAILURE;
	u32 RetValue = (u32)XST_FAILURE;
	u32 RefClk;


	if (NULL == InstancePtr) {
		return XSK_EFUSEPS_ERROR_PS_STRUCT_NULL;
	}

	RefClk = XilSKey_Timer_Intialise();

	/**
	 *  Check the variables
	 */

	if (((InstancePtr->EnableWriteProtect != TRUE) &&
		 (InstancePtr->EnableWriteProtect != FALSE)) ||
		((InstancePtr->EnableRsaAuth != TRUE) &&
		 (InstancePtr->EnableRsaAuth != FALSE)) ||
		((InstancePtr->EnableRom128Crc != TRUE) &&
		 (InstancePtr->EnableRom128Crc != FALSE)) ||
		((InstancePtr->EnableRsaKeyHash != TRUE) &&
		 (InstancePtr->EnableRsaKeyHash != FALSE)) ||
		((InstancePtr->DisableDftJtag != TRUE) &&
		 (InstancePtr->DisableDftJtag != FALSE)) ||
		((InstancePtr->DisableDftMode != TRUE) &&
		 (InstancePtr->DisableDftMode != FALSE))) {
		return XSK_EFUSEPS_ERROR_PS_PARAMETER_WRONG;
	}

	/**
	 * XAdc Initialization
	 */
	Status = XilSKey_EfusePs_XAdcInit();
	if (Status != XST_SUCCESS) {
		RetValue = XSK_EFUSEPS_ERROR_XADC_INIT;
		goto ExitFinal;
	}

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,"Writing PS EFUSE \n");
	/**
	 *  Unlock the eFUSE controller
	 */
	XSK_EFUSEPS_CONTROLER_UNLOCK();

	/**
	 *  Check if the controller is unlocked
	 */
	if (XSK_EFUSEPS_CONTROLER_LOCK_STATUS() == TRUE) {
		RetValue = XSK_EFUSEPS_ERROR_CONTROLLER_LOCK;
		goto ExitFinal;
	}

	/**
	 *  If eFUSE Array is write protected
	 *  no more writes are possible
	 */

	if (XilSKey_EfusePs_IsEfuseWriteProtected() == TRUE) {
		RetValue = XSK_EFUSEPS_ERROR_EFUSE_WRITE_PROTECTED;
		goto ExitCtrlLock;
	}

	/**
	 *  Configure the eFUSE controller with mode, strobe width values.
	 */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_SINGLE_MODE,
			RefClk, XSK_EFUSEPS_READ_MODE_NORMAL);
	if (Status != XST_SUCCESS) {
		RetValue = XSK_EFUSEPS_ERROR_CONTROLLER_CONFIG + Status;
		goto ExitCtrlResetStatus;
	}

	/**
	 * Enable Programming, write and read
	 */
	XilSKey_EfusePs_ControllerSetReadWriteEnable(XSK_EFUSEPS_ENABLE_PROGRAMMING |
			XSK_EFUSEPS_ENABLE_READ | XSK_EFUSEPS_ENABLE_WRITE);

	/**
	 * Initialize the timer for delay while programming the eFUSE
	 */
	XilSKey_Efuse_StartTimer();

	/**
	 * Program the eFUSE based
	 * on the structure values
	 */

	/**
	 * Program the eFUSE bit 0xA to enable
	 * ROM 128K CRC
	 */
	if (InstancePtr->EnableRom128Crc) {
		Status = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
				XSK_EFUSEPS_APB_ROM_128K_CRC_ENABLE, RefClk);
		if (Status != XST_SUCCESS) {
			RetValue = XSK_EFUSEPS_ERROR_WRITE_128K_CRC_BIT + Status;
			goto ExitCtrlResetStatus;
		}
	}

	/**
	 *  Program the RSA key Hash value in eFUSE Array \
	 */
	if (InstancePtr->EnableRsaKeyHash) {
		/**
		 * Check if all the hash eFUSE bits are zero or not
		 */
		/* NEW: Changed the ReadRsaKeyHash to new function
		 * XilSKey_EfusePs_CheckRsaHashForAllZeros because of 2 bugs
		 * 1. If we are using ReadRsaKeyHash function here, if only 1 bit is
		 * written it will correct it and allow to write
		 * 2. Controller is in SINGLE mode, so even if 2nd half is written
		 * we will continue to write
		 * */
		Status = XilSKey_EfusePs_CheckRsaHashForAllZeros(RefClk);
		if (Status != XST_SUCCESS) {

			RetValue = XSK_EFUSEPS_ERROR_READ_HASH_BEFORE_PROGRAMMING + Status;
			goto ExitCtrlResetStatus;
		}

		/**
		 * Program the RSA hash
		 */
		Status = XilSKey_EfusePs_WriteRsaKeyHash(
				InstancePtr->RsaKeyHashValue, RefClk);
		if (Status != XST_SUCCESS) {
			RetValue = XSK_EFUSEPS_ERROR_WRITE_RSA_HASH + Status;
			goto ExitCtrlResetStatus;
		}
	}

	/**
	 * Program the eFUSE bit 0xB to enable
	 * RSA authentication
	 */
	if (InstancePtr->EnableRsaAuth) {
		Status = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
				XSK_EFUSEPS_APB_RSA_AUTH_ENABLE, RefClk);
		if (Status != XST_SUCCESS) {
			RetValue = XSK_EFUSEPS_ERROR_WRITE_RSA_AUTH_BIT + Status;
			goto ExitCtrlResetStatus;
		}
	}

	/* Programs 0xC eFuse bit to disable DFT JTAG */
	if (InstancePtr->DisableDftJtag) {
		Status = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
				XSK_EFUSEPS_APB_DFT_JTAG_DISABLE, RefClk);
		if (Status != XST_SUCCESS) {
			RetValue = XSK_EFUSEPS_ERROR_WRTIE_DFT_JTAG_DIS_BIT + Status;
			goto ExitCtrlResetStatus;
		}
	}

	/* Programs 0xD eFuse bit to disable DFT Mode */
	if (InstancePtr->DisableDftMode) {
		Status = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
				XSK_EFUSEPS_APB_DFT_MODE_DISABLE, RefClk);
		if (Status != XST_SUCCESS) {
			RetValue = XSK_EFUSEPS_ERROR_WRTIE_DFT_MODE_DIS_BIT + Status;
			goto ExitCtrlResetStatus;
		}
	}

	/**
	 *  Program the eFUSE bit 0x8,0x9 to enable
	 * eFUSE Array Write Protection
	 *
	 * Once enabled the write protection, we will not be
	 * able to program any PS eFUSE afterwards
	 */

	/**
	 *  SLCR reset is required for write protect
	 *  to take into effect
	 */
	if (InstancePtr->EnableWriteProtect) {
		Status = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
				XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_1, RefClk);

		StatusRedundantBit = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
				XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_2, RefClk);


		/* NEW: Error only if burning of the both the bits are not success */
		if ((Status != XST_SUCCESS) && (StatusRedundantBit != XST_SUCCESS)) {
			RetValue = XSK_EFUSEPS_ERROR_WRITE_WRITE_PROTECT_BIT + Status;
			goto ExitCtrlResetStatus;
		}
	}

	RetValue = (u32)XST_SUCCESS;
ExitCtrlResetStatus:
	/**
	 * Disable Programming, write and read
	 */
	XilSKey_EfusePs_ControllerSetReadWriteEnable(0);

ExitCtrlLock:
	/**
	 *  Lock the eFUSE controller
	 */
	XSK_EFUSEPS_CONTROLER_LOCK();

ExitFinal:
	return RetValue;
}

/***************************************************************************/
/**
* This function is used to read the PS eFUSE.
*
* @param	InstancePtr - Pointer to the PsEfuseHandle which describes
*                             which PS eFUSE should be burned.
*
* @return
* 		- XST_SUCCESS no errors occurred.
* 		- In case of error, value is as defined in xilskey_utils.h.
* 		Error value is a combination of Upper 8 bit value and
* 		Lower 8 bit value. For example, 0x8A03 should be checked
* 		in error.h as 0x8A00 and 0x03. Upper 8 bit value signifies
* 		the major error and lower 8 bit values tells more precisely.
* @note		When called: This API initializes the timer, XADC subsystems.
*		Unlocks the PS eFUSE Controller. Configures the PS eFUSE
*		Controller and enables read-only mode. Reads the PS eFUSE
*		(Hash Value), and enables read-only mode. Locks the PS eFUSE
*		Controller.
*		Returns an error, if the reference clock frequency is not in
*		between 20 and 60MHz. or if unable to unlock PS eFUSE
*		controller or requested address corresponds to restricted bits.
*		or if the temperature and voltage are not within range
*
****************************************************************************/
u32 XilSKey_EfusePs_Read(XilSKey_EPs *InstancePtr)
{

	u32 Status = (u32)XST_FAILURE;
	u32 RetValue = (u32)XST_FAILURE;
	u32 RefClk;
	u32 Index;

	if (NULL == InstancePtr) {
		return XSK_EFUSEPS_ERROR_PS_STRUCT_NULL;
	}

	/* Initializing RSAKeyReadback to Zero since when reading
	 * the key with XSK_EFUSEPS_ENABLE_RSA_KEY_HASH
	 * as FALSE it will give all zeros.
	 */
	for(Index = 0; Index < XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES; Index++) {
		InstancePtr->RsaKeyReadback[Index] = 0;
	}

	RefClk = XilSKey_Timer_Intialise();

	/**
	 * Check the variables
	 */

	if ((InstancePtr->EnableRsaKeyHash != TRUE) &&
		(InstancePtr->EnableRsaKeyHash != FALSE)) {
		return XSK_EFUSEPS_ERROR_PS_PARAMETER_WRONG;
	}

	/**
	 * XAdc Initialization
	 */
	Status = XilSKey_EfusePs_XAdcInit();
	if (Status != XST_SUCCESS) {
		RetValue = XSK_EFUSEPS_ERROR_XADC_INIT;
		goto ExitFinal;
	}

	/**
	 *  Unlock the eFUSE controller
	 */
	XSK_EFUSEPS_CONTROLER_UNLOCK();

	/**
	 *  Check if the controller is unlocked
	 */
	if (XSK_EFUSEPS_CONTROLER_LOCK_STATUS() == TRUE) {
		RetValue = XSK_EFUSEPS_ERROR_CONTROLLER_LOCK;
		goto ExitFinal;
	}

	/**
	 *  Configure the eFUSE controller with mode, strobe width values
	 */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_REDUNDANCY_MODE,
			RefClk, XSK_EFUSEPS_READ_MODE_NORMAL);
	if (Status != XST_SUCCESS) {
		RetValue = XSK_EFUSEPS_ERROR_CONTROLLER_CONFIG + Status;
		goto ExitCtrlLock;
	}

	/**
	 * Enable eFUSE reading only
	 */
	XilSKey_EfusePs_ControllerSetReadWriteEnable(XSK_EFUSEPS_ENABLE_READ);

	/**
	 *  Read the eFUSE
	 */

	/**
	 *  Read the RSA key Hash value from eFUSE Array
	 */
	if (InstancePtr->EnableRsaKeyHash) {
		Status = XilSKey_EfusePs_ReadRsaKeyHash(
				InstancePtr->RsaKeyReadback, RefClk);
		if (Status != XST_SUCCESS) {
			RetValue = XSK_EFUSEPS_ERROR_READ_RSA_HASH + Status;
			goto ExitCtrlResetStatus;
		}
	}

	RetValue = (u32)XST_SUCCESS;
ExitCtrlResetStatus:
	/**
	 * Disable Programming, write and read
	 */
	XilSKey_EfusePs_ControllerSetReadWriteEnable(0);

ExitCtrlLock:
	/**
	 *  Lock the eFUSE controller
	 */
	XSK_EFUSEPS_CONTROLER_LOCK();

ExitFinal:
	return RetValue;
}

/***************************************************************************/
/**
* This function is used to check the RSA Hash for all zeros.
*
* @param RefClk is the reference clock frequency. Clock frequency can be
* 				between 20MHz to 60MHz specified in Hz
*
* @return
* 		- XST_SUCCESS if RSA Hash eFUSE bits are all zeros.
*		- XSK_EFUSEPS_ERROR_RSA_HASH_ALREADY_PROGRAMMED if any RSA hash
*		  eFUSE bit is set
*		- Other errors because of internal controller functions and
*		  can be checked in xilskey_utils.h.
*
****************************************************************************/

static u32 XilSKey_EfusePs_CheckRsaHashForAllZeros(u32 RefClk)
{
	u32 EfuseAddress;
	u32 Status = (u32)XST_FAILURE;
	u32 RetValue = (u32)XST_FAILURE;
	u32 LoopIndex;
	u8 Data;

	/**
	 * Set the controller to read in redundancy mode
	 */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_REDUNDANCY_MODE,
			RefClk, XSK_EFUSEPS_READ_MODE_NORMAL);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/**
	 * Check the Hash eFUSE Bits for all Zero's
	 */
	LoopIndex = 0;
	EfuseAddress=XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_START_ADDR;
	while (LoopIndex < (XSK_EFUSEPS_HAMMING_LOOPS*XSK_EFUSEPS_HAMMING_LENGTH)) {
		/**
		 *  Escape the Xilinx Reserved Bits
		 */
		while (XilSKey_EfusePs_IsAddressXilRestricted(EfuseAddress) == TRUE) {
			EfuseAddress += 4;
		}
		Status = XilSKey_EfusePs_ReadWithXadcCheck(EfuseAddress, RefClk, &Data);
		if (Status != XST_SUCCESS) {
			RetValue = Status;
			goto ExitControllerReset;
		}
		/**
		 * Data value of 1 indicates Hash is already written
		 */
		if (Data != 0) {
			RetValue = XSK_EFUSEPS_ERROR_RSA_HASH_ALREADY_PROGRAMMED;
			goto ExitControllerReset;
		}
		EfuseAddress += 4;
		LoopIndex++;
	} /* End of  while (LoopIndex < (XSK_EFUSEPS_HAMMING_LOOPS* ....) */
	
	RetValue = (u32)XST_SUCCESS;

ExitControllerReset:
	/**
	 * Set the controller back to single mode
	 */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_SINGLE_MODE,
			RefClk, XSK_EFUSEPS_READ_MODE_NORMAL);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return RetValue;
}

/***************************************************************************/
/**
* This function is used to to write the Single eFUSE which checks for
* XADC temperature and voltage and verifies the written bit in the eFUSE
* Verification of the bit is done by reading the bit in margin 1  mode and
* normal mode.
* Temperature and voltage are read from XADC and verified for the
* correct range
*
* @param EfuseAddress is the address of the eFUSE bit to be written
* @param RefClk is the reference clock frequency. Clock frequency can be
* 				between 20MHz to 60MHz specified in Hz
* @return
* 		- XST_SUCCESS no errors occurred
*		- an error XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE when
*		  voltage not in range
*		- an error XSK_EFUSEPS_ERROR_WRITE_TEMPERATURE_OUT_OF_RANGE when
*		  temperature not in range
*		- Other errors because of internal functions and
*		  can be checked in xilskey_utils.h
****************************************************************************/
static u32
XilSKey_EfusePs_WriteWithXadcCheckAndVerify(u32 EfuseAddress, u32 RefClk)
{
	XSKEfusePs_XAdc XAdcInstancePtr;
	u32 Status = (u32)XST_FAILURE;
	u32 StatusLowerAddr = (u32)XST_FAILURE;
	u32 RedundantEfuseAddress;

	/**
	 * If the eFUSE bit is already programmed, no need to program again.
	 * That is taken care in eFUSE controller write function
	 */
	XAdcInstancePtr.VType = XSK_EFUSEPS_VPAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&XAdcInstancePtr);

	/**
	 * Check the temperature and voltage
	 */
#ifdef XSK_ZYNQ_PLATFORM
	if ((XAdcInstancePtr.Temp < XSK_EFUSEPS_TEMP_MIN_RAW) ||
			((XAdcInstancePtr.Temp > XSK_EFUSEPS_TEMP_MAX_RAW))) {
		return XSK_EFUSEPS_ERROR_WRITE_TEMPERATURE_OUT_OF_RANGE;
	}

	if ((XAdcInstancePtr.V < XSK_EFUSEPS_WRITE_VPAUX_MIN_RAW) ||
			((XAdcInstancePtr.V > XSK_EFUSEPS_WRITE_VPAUX_MAX_RAW))) {
		return XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE;
	}
#endif
	/**
	 * Write the eFUSE bit
	 */
	StatusLowerAddr = XilSKey_EfusePs_WriteEfuseBit(EfuseAddress);

	/**
	 * verify only if programming is success
	 * verification of lower address is checked while
	 * higher address verification
	 */
	if (StatusLowerAddr == XST_SUCCESS) {
		StatusLowerAddr = XilSKey_EfusePs_VerifyWithXadcCheck(EfuseAddress,
															 RefClk);
	}

	/**
	 * Write the Redundant eFUSE bit address
	 */
	RedundantEfuseAddress  = XSK_EFUSEPS_APB_MIRROR_ADDRESS(EfuseAddress);
	Status = XilSKey_EfusePs_WriteEfuseBit(RedundantEfuseAddress);
	if (Status != XST_SUCCESS) {
		/**
		 * if higher address is not success, check lower address and return
		 */
		if (StatusLowerAddr != XST_SUCCESS) {
			return Status;
		} else {
			/**
			 *  Successfully written lower address, so success
			 */
			return XST_SUCCESS;
		}
	}

	Status = XilSKey_EfusePs_VerifyWithXadcCheck(RedundantEfuseAddress,
												RefClk);
	if (Status != XST_SUCCESS) {
		/**
		 * if higher address is not success, check lower address and return
		 */
		if (StatusLowerAddr != XST_SUCCESS) {
			return Status;
		} else {
			/**
			 *  Successfully written lower address, so success
			 */
			return XST_SUCCESS;
		}
	}

	/**
	 * Ideal case where lower and higher address is written properly
	 */
	return XST_SUCCESS;
}


/***************************************************************************/
/**
* This function is used to write the RSA hash in the eFUSE Array in Little
* Endian.
*
* @param RsaKeyHashBuf is the buffer containing the RSA hash to be written
* 		 to the eFUSE array.
* @param RefClk is the reference clock frequency. Clock frequency can be
* 				between 20MHz to 60MHz specified in Hz
* @return
* 		- XST_SUCCESS no errors occurred.
*		- Other errors because of internal controller functions and
*		  can be checked in xilskey_utils.h
*
****************************************************************************/
static u32 XilSKey_EfusePs_WriteRsaKeyHash(u8 *RsaKeyHashBuf, u32 RefClk)
{
	int LoopIndex,BitIndex;
	u8 DataBytes[XSK_EFUSEPS_RSA_HASH_LEN_ECC_CALC] = {0}, Ecc[32];
	u32 EfuseAddress;
	u32 Status = (u32)XST_FAILURE;


	/**
	 * Convert the RSA Key hash to bit data
	 */
	 XilSKey_Efuse_ConvertBitsToBytes(RsaKeyHashBuf, DataBytes,
			 (XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES*8));

	/**
	 * Prepare the hamming matrix for encoding the RSA Key hash
	 */
	XilSKey_EfusePs_GenerateMatrixMap();

	/**
	 * Encode the RSA Key hash with (31,26) hamming and write into
	 * the eFUSE array
	 */
	LoopIndex=0;
	EfuseAddress=XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_START_ADDR;
	while (LoopIndex < XSK_EFUSEPS_HAMMING_LOOPS) {
		/**
		 *  Encode the data
		 */
		XilSKey_EfusePs_EccEncode(DataBytes + (LoopIndex*26), Ecc);
		/* Write 31 bit Row in eFUSE Array */
		for (BitIndex=0;BitIndex<31;BitIndex++) {
			/**
			 *  Escape the Xilinx Reserved Bits
			 */
			if (XilSKey_EfusePs_IsAddressXilRestricted(EfuseAddress) == TRUE) {
				xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
							  "Discarding eFUSE Address %0x\n",
							  EfuseAddress);
				EfuseAddress = EfuseAddress+4;
			}

			/**
			 *  Write the eFUSE bit only if the value is one
			 */
			if (Ecc[BitIndex] & 0x1) {
				xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
							 "Writing eFUSE Addr %0x, LoopIndex %0x, "
							 "BitIndex %0x\n",
							  EfuseAddress, LoopIndex, BitIndex);
				Status = XilSKey_EfusePs_WriteWithXadcCheckAndVerify(
												EfuseAddress, RefClk);
				if (Status != XST_SUCCESS) {
					return Status;
				}
			} /* End of (Ecc[BitIndex] & 0x1) */
			/**
			 *  Ready for next eFUSE Bit
			 */
			EfuseAddress += 4;

		} /* End of for(BitIndex=0;BitIndex<31;BitIndex++) */
		LoopIndex++;
	} /* End of  while(LoopIndex < XSK_EFUSEPS_HAMMING_LOOPS) */

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to Read the RSA hash in the eFUSE Array in Little
* Endian.
*
* @param    RsaKeyHashBuf is the output buffer where RSA hash is copied
* 			from the eFUSE array after hamming decode.
* @param	RefClk is the reference clock frequency. Clock frequency can be
* 				between 20MHz to 60MHz specified in Hz
* @return
* 		- XST_SUCCESS no errors occurred.
*		- Other errors because of internal controller functions and
*		  can be checked in xeFUSEutils.h
****************************************************************************/
static u32 XilSKey_EfusePs_ReadRsaKeyHash(u8 *RsaKeyHashBuf, u32 RefClk)
{
	int LoopIndex, BitIndex, Index;
	u8 DataBytes[260], Recover[32], Syndrome[5], Pos;
	u32 EfuseAddress;
	u32 Status = (u32)XST_FAILURE;

	/**
	 * Prepare the hamming matrix for encoding the RSA Key hash
	 */
	XilSKey_EfusePs_GenerateMatrixMap();

	/**
	 * Encode the RSA Key hash with (31,26) hamming and write to
	 * the eFUSE array
	 */
	LoopIndex=0;
	EfuseAddress=XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_START_ADDR;
	while (LoopIndex < XSK_EFUSEPS_HAMMING_LOOPS) {
		/**
		 * Write 31 bit Row in eFUSE Array
		 */
		for (BitIndex=0;BitIndex<31;BitIndex++) {
			/**
			 *  Escape the Xilinx Reserved Bits
			 */
			if (XilSKey_EfusePs_IsAddressXilRestricted(EfuseAddress) == TRUE) {
				xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
						"Discarding eFUSE Address %0x\n",
						EfuseAddress);
				EfuseAddress += 4;
			}
			xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
					"Reading eFUSE Addr %0x, LoopIndex %0x, "
					"BitIndex %0x\n",
					 EfuseAddress, LoopIndex, BitIndex);
			Status = XilSKey_EfusePs_ReadWithXadcCheck(
										EfuseAddress,
										RefClk,
										Recover+BitIndex);
			if (Status != XST_SUCCESS) {
				return Status;
			}
			EfuseAddress += 4;
		} /* End of for(BitIndex=0;BitIndex<31;BitIndex++) */

		/**
		 *  Decode the data
		 */
		Pos = XilSKey_EfusePs_EccDecode(Recover, Syndrome);
		/**
		 *  Repair the corrupt bit
		 */
		if(Pos < 31)
		{
			Recover[Pos] = (Recover[Pos] + 1)% 2;
		}
		/**
		 *  Copy the data
		 */
		for(Index=0;Index<26;Index++)
		{
			*(DataBytes + (LoopIndex * 26) + Index) = Recover[Index];
		}
		LoopIndex++;
	} /* End of  while(LoopIndex < XSK_EFUSEPS_HAMMING_LOOPS) */

	/**
	 * Convert the bit data to RSA key hash
	 */
	XilSKey_EfusePs_ConvertBytesToBits(DataBytes,
									  RsaKeyHashBuf,
									  (XSK_EFUSEPS_RSA_KEY_HASH_LEN_BITS));

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to read the PS eFUSE bit. Reading of the eFUSE bit
* is done in NORMAL read mode. Temperature and voltage must be in specified
* range for reading.
*
* @param EfuseAddress is the address of the eFUSE bit
* @param RefClk is the reference clock frequency. Clock frequency can be
* 				between 20MHz to 60MHz specified in Hz
* @param Data is where the read data is stored. It will have values 0 or 1
* @return
* 		- XST_SUCCESS no errors occurred.
*		- an error XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE when
*		  voltage not in range
*		- an error XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE when
*		  temperature not in range
*		- Other errors because of internal controller functions and
*		  can be checked in xilskey_utils.h
****************************************************************************/
static u32
XilSKey_EfusePs_ReadWithXadcCheck(u32 EfuseAddress, u32 RefClk, u8 *Data)
{
	XSKEfusePs_XAdc XAdcInstancePtr;
	u32 Status = (u32)XST_FAILURE;
	(void) RefClk;

	XAdcInstancePtr.VType = XSK_EFUSEPS_VPAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&XAdcInstancePtr);

	/**
	 * Check the temperature and voltage
	 */
#ifdef XSK_ZYNQ_PLATFORM
	if ((XAdcInstancePtr.Temp < XSK_EFUSEPS_TEMP_MIN_RAW) ||
			((XAdcInstancePtr.Temp > XSK_EFUSEPS_TEMP_MAX_RAW))) {
		return XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
	}

	if ((XAdcInstancePtr.V < XSK_EFUSEPS_READ_VPAUX_MIN_RAW) ||
			((XAdcInstancePtr.V > XSK_EFUSEPS_READ_VPAUX_MAX_RAW))) {
		return XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE;
	}
#endif
	/**
	 * Read the eFUSE bit
	 */
	Status = XilSKey_EfusePs_ReadEfuseBit(EfuseAddress, Data);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to verify the written eFUSE bit. It is checked
* against the value of 1 only. Verification of the eFUSE bit is done by reading
* in MARGIN 2 mode which was highest margin.
*
* @param EfuseAddress is the address of the eFUSE bit
* @param RefClk is the reference clock frequency. Clock frequency can be
* 				between 20MHz to 60MHz specified in Hz
* @return
* 		- XST_SUCCESS no errors occurred.
*		- XST_FAILURE an error occurred during verifying the PS eFUSE.
*		- an error XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE when
*		  voltage not in range
*		- an error XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE when
*		  temperature not in range
*		- an error XSK_EFUSEPS_ERROR_VERIFICATION when verification fails
*		- Other errors because of internal controller functions and
*		  can be checked in xilskey_utils.h
****************************************************************************/
static u32 XilSKey_EfusePs_VerifyWithXadcCheck(u32 EfuseAddress, u32 RefClk)
{
	XSKEfusePs_XAdc XAdcInstancePtr;
	u32 Status = (u32)XST_FAILURE;
	u8 Data;

	XAdcInstancePtr.VType = XSK_EFUSEPS_VPAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&XAdcInstancePtr);

	/**
	 * Check the temperature and voltage
	 */
#ifdef XSK_ZYNQ_PLATFORM
	if ((XAdcInstancePtr.Temp < XSK_EFUSEPS_TEMP_MIN_RAW) ||
			((XAdcInstancePtr.Temp > XSK_EFUSEPS_TEMP_MAX_RAW))) {
		return XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
	}

	if ((XAdcInstancePtr.V < XSK_EFUSEPS_READ_VPAUX_MIN_RAW) ||
			((XAdcInstancePtr.V > XSK_EFUSEPS_READ_VPAUX_MAX_RAW))) {
		return XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE;
	}
#endif

	/**
	 * Read the eFUSE bit in Normal mode
	 */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_SINGLE_MODE,
				RefClk, XSK_EFUSEPS_READ_MODE_NORMAL);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	Status = XilSKey_EfusePs_ReadEfuseBit(EfuseAddress, &Data);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	if (0 == ((Data) & 0x1)) {
		return XSK_EFUSEPS_ERROR_VERIFICATION;
	}


	/**
	 * Read the eFUSE bit in Margin 1 Mode
	 */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_SINGLE_MODE,
				RefClk, XSK_EFUSEPS_READ_MODE_MARGIN_1);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XilSKey_EfusePs_ReadEfuseBit(EfuseAddress, &Data);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	if (0 == ((Data) & 0x1)) {
		return XSK_EFUSEPS_ERROR_VERIFICATION;
	}

	/* Read the eFUSE bit in Margin 2 Mode */
	Status = XilSKey_EfusePs_ControllerConfig(XSK_EFUSEPS_SINGLE_MODE,
				RefClk, XSK_EFUSEPS_READ_MODE_MARGIN_2);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	Status = XilSKey_EfusePs_ReadEfuseBit(EfuseAddress, &Data);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	if (0 == ((Data) & 0x1)) {
		return XSK_EFUSEPS_ERROR_VERIFICATION;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to generate the matrix map of the G and H for
* hamming code (31,26). G is [31,5] and defined as [A|I], I is identity
* matrix of [5,5].
*
* @return None
*
* TDD Cases:
* 		Check the generated matrix
* 		Check the memory corruption of the generated matrix
*
****************************************************************************/

void XilSKey_EfusePs_GenerateMatrixMap(void)
{

	u8 y, x, Index;
	u8 DataIndex, ParityIndex;

	/* Matrix[31][5]={0} */
	for(y = 0; y < 5; y++) {
		for(x = 0; x < 31; x++) {
			Matrix[x][y] = 0;
		}
	}

	/* Generate Hamming Code Matrix for both Encoding and Decoding */
	for(y = 0; y < 5; y++) {
		Index = 0;
		for(x = 1; x < 32; x++) {
			if ((x != 1) && (x != 2) && (x != 4) && (x != 8) && (x != 16)) {
				if ( ((x & (1 << y)) >> y) == 1)
					Matrix[Index++][y] = 1;
				else
					Matrix[Index++][y] = 0;
			}
		}
	}

	/* Fill Identity Matrix */
	for(y = 0; y < 5; y++) {
		for(x = 0; x < 5; x++) {
			if (x == y) Matrix[26+x][y] = 1;
			else Matrix[26+x][y] = 0;
		}
	}
#if 0
// Print out format index
	for(x = 1; x < 27; x++) {
		printf("%*dd", 3, x);
	}
	printf("\n");

	for(y = 0; y < 5; y++) {
		for(x = 0; x < 31; x++) {
			printf("%*d", 4, Matrix[x][y]);
		}
		printf("\n");
	}

#endif

	/* Create Error Code Map Index */
	ErrorCodeIndex[0] = 0xFF; /* No Error */
	DataIndex = 0;
	ParityIndex = 26;
	for(x = 1; x < 32; x++) {
		if ((x != 1) && (x!= 2) && (x!= 4) && (x!= 8) && (x != 16)) {
			ErrorCodeIndex[x] = DataIndex;
			DataIndex ++;
		} else {
			ErrorCodeIndex[x] = ParityIndex;
			ParityIndex ++;
		}
	}
}


/***************************************************************************/
/**
* This function is used to encode the incoming data byte. It uses
* hamming (31,26) algorithm. 26 bits are encoded to 31 bits
*
* @param InData is 26 bit input data with each bit represented in one byte
*
* @param Ecc is the 31 bit encoded data with each bit represented in one byte
*
* @return None
*
* TDD Cases:
* 		Check the parameters
* 		Check the encoded data for different input data
* 		Check the input data for boundary cases
* 		Check for memory corruption
*
****************************************************************************/
void XilSKey_EfusePs_EccEncode(const u8 *InData, u8 *Ecc)
{
	u8  x, y, Index;

	for(x = 0; x < 26; x++)
		Ecc[x] = InData[x];
	for(y = 0; y < 5; y++) {
		Ecc[y+26] = 0;
		for( Index =0; Index < 26; Index++)
			Ecc[y+26] += Matrix[Index][y] * InData[Index];
		Ecc[y+26] %= 2;
	}
}


/***************************************************************************/
/**
* This function is used to decode the incoming encoded byte.
*
* @param Corrupt is the input encoded data. It has 26 bit data with
* 		 5 bit parity data
*
* @param Syndrome is the output updated with the parity error
* 		 information.
*
* @return position of the error in the data byte
*
* TDD Cases:
* 		Check the parameters
* 		Check the decode with out any error
* 		Check the decode with 1 bit error
* 		Check the decode with 2 bit error
* 		Check the decode for boundary cases
* 		Check for memory corruption
*
****************************************************************************/
u8 XilSKey_EfusePs_EccDecode(const u8 *Corrupt, u8 *Syndrome)
{
	u8 x, y;
	u8 Pos;

	// Calculate Error Syndrome
	for(y = 0; y < 5; y++) {
		Syndrome[y] = 0;
		for( x =0; x < 31; x++)
			Syndrome[y] += Matrix[x][y] * Corrupt[x];
		Syndrome[y] %= 2;
	}
	Pos = (Syndrome[4] << 4) + (Syndrome[3] << 3) +
			(Syndrome[2] << 2) + (Syndrome[1] << 1) + Syndrome[0];
	Pos = ErrorCodeIndex[Pos];

	return Pos;
}

/***************************************************************************/
/**
* This function is used to set the controller mode to Redundancy/Single mode.
*
* @param Mode is mode of the controller to the set.
* 		- REDUNDANCY_MODE
* 		- SINGLE_MODE
*
* @return None
*
****************************************************************************/

static void XilSKey_EfusePs_SetControllerMode (u8 Mode)
{
	u32 RegVal;
	if(Mode == XSK_EFUSEPS_REDUNDANCY_MODE) {
		/**
		 *  Configuration Reg - Redundancy Mode
		 */
		RegVal = Xil_In32(XSK_EFUSEPS_CONFIG_REG);
		RegVal |= XSK_EFUSEPS_CONFIG_REDUNDANCY;
		Xil_Out32(XSK_EFUSEPS_CONFIG_REG, RegVal);
	}
	else if(Mode == XSK_EFUSEPS_SINGLE_MODE)	{
		/**
		 *  Configuration Reg - Single Mode
		 */
		RegVal = Xil_In32(XSK_EFUSEPS_CONFIG_REG);
		RegVal &= ~XSK_EFUSEPS_CONFIG_REDUNDANCY;
		Xil_Out32(XSK_EFUSEPS_CONFIG_REG, RegVal);
	}
	return ;
}


/***************************************************************************/
/**
* This function is used to set the controller read mode to
* Normal/Margin 1/Margin 2.
*
* @param ReadMode is the read mode of the controller
* 		- XSK_EFUSEPS_READ_MODE_NORMAL
* 		- XSK_EFUSEPS_READ_MODE_MARGIN_1
* 		- XSK_EFUSEPS_READ_MODE_MARGIN_2
*
* @return  None
*
****************************************************************************/

static void XilSKey_EfusePs_SetControllerReadMode (u8 ReadMode)
{

	u32 RegVal;

	/**
	 * Read Configuration Reg
	 */
	RegVal = Xil_In32(XSK_EFUSEPS_CONFIG_REG);
	/**
	 *  Reset the read mode
	 */
	RegVal &= ~XSK_EFUSEPS_CONFIG_MARGIN_RD;
	if (ReadMode == XSK_EFUSEPS_READ_MODE_NORMAL) {
		/**
		 *  Set to Normal read mode
		 */
		RegVal |= XSK_EFUSEPS_CONFIG_RD_NORMAL;
	}
	else if (ReadMode == XSK_EFUSEPS_READ_MODE_MARGIN_1)	{
		/**
		 *  Set to Margin 1 read mode
		 */
		RegVal |= XSK_EFUSEPS_CONFIG_RD_MARGIN_1;
	}
	else if (ReadMode == XSK_EFUSEPS_READ_MODE_MARGIN_2)	{
		/**
		 * Set to Margin 2 read mode
		 */
		RegVal |= XSK_EFUSEPS_CONFIG_RD_MARGIN_2;
	}
	Xil_Out32(XSK_EFUSEPS_CONFIG_REG, RegVal);
	return ;
}

/***************************************************************************/
/**
* This function is used to check the read mode supported
*
* @param ReadMode is input read mode to be checked against
*
* @return
* 		- XST_SUCCESS if read mode is supported
*		- XST_FAILURE if read mode is not supported
*
****************************************************************************/

static u8 XilSKey_EfusePs_IsReadModeSupported (u8 ReadMode)
{
	if ((ReadMode == XSK_EFUSEPS_READ_MODE_NORMAL)   ||
		(ReadMode == XSK_EFUSEPS_READ_MODE_MARGIN_1) ||
		(ReadMode == XSK_EFUSEPS_READ_MODE_MARGIN_2)) {
		return TRUE;
	}

	return FALSE;
}


/***************************************************************************/
/**
* This function is used to set the controller mode, read mode along with
* the read and program strobe width values based on the reference clock.
*
* @param CtrlMode is the mode of the controller
* 		- XSK_EFUSEPS_REDUNDANCY_MODE
* 		- XSK_EFUSEPS_SINGLE_MODE
*
* @param RefClk is the CPU 1x reference clock frequency. Clock frequency can be
* 		 between 20MHz to 100MHz specified in Hz
*
* @param ReadMode is the read mode of the controller
* 		- XSK_EFUSEPS_READ_MODE_NORMAL
* 		- XSK_EFUSEPS_READ_MODE_MARGIN_1
* 		- XSK_EFUSEPS_READ_MODE_MARGIN_2
*
* @return
* 		- XST_SUCCESS no errors occurred.
*		- an error when controller mode is not supported
*		- an error when reference clock is not supported
*		- an error when read mode is not supported
*
* Test Cases:
	Check single mode in CFG Reg
	Check redundancy mode in CFG Reg
	Check strobe width values for write mode
	Check strobe width values for various read mode
	Check Normal Read mode setting in CFG Reg
	Check Margin 1 Read mode setting in CFG Reg
	Check Margin 2 Read mode setting in CFG Reg
	Boundary Conditions

*
****************************************************************************/
u32 XilSKey_EfusePs_ControllerConfig(u8 CtrlMode, u32 RefClk, u8 ReadMode)
{
	u32 RdStrWdthVal=0, PrgmStrWdthVal=0;

	/**
	 *  Check the parameters
	 *  Mode can be Single or Redundancy mode
	 */

	if ((CtrlMode != XSK_EFUSEPS_SINGLE_MODE) &&
			(CtrlMode != XSK_EFUSEPS_REDUNDANCY_MODE)) {
		return XSK_EFUSEPS_ERROR_CONTROLLER_MODE;
	}


	/**
	 *  Ref Clock should be between 20MHz - 60MHz
	 */
	if ((RefClk < XSK_EFUSEPS_REFCLK_LOW_FREQ) ||
			(RefClk > XSK_EFUSEPS_REFCLK_HIGH_FREQ)) {
		return XSK_EFUSEPS_ERROR_REF_CLOCK;
	}

	/**
	 *  3 read modes are supported
	 */
	if (!XilSKey_EfusePs_IsReadModeSupported(ReadMode)) {
		return XSK_EFUSEPS_ERROR_READ_MODE;
	}

	/**
	 *  Set the controller mode
	 */
	XilSKey_EfusePs_SetControllerMode(CtrlMode);

	/**
	 * Set the controller read mode
	 */
	XilSKey_EfusePs_SetControllerReadMode(ReadMode);


#if 0
	u32 RegVal;
	/* Commented as we are not supporting Reference Freq > 60MHz */
	/**
	 *  Set TSU_H_A to 1 if RefClk > 80MHz
	 */
	RegVal = Xil_In32(XSK_EFUSEPS_CONFIG_REG);
	if (RefClk > 80000000) {
		RegVal |= XSK_EFUSEPS_CONFIG_TSU_H_A;
	}
	Xil_Out32(XSK_EFUSEPS_CONFIG_REG, RegVal);
#endif
	/**
	 * Program the Strobe width values for read and write
	 * 12us is required for write and 150ns is required for read
	 * PGM_STBW = ceiling(12us/ref_clk period)
	 * RD_STBW = ceiling(150ns/ref_clk period)
	 */
	PrgmStrWdthVal = XSK_EFUSEPS_PRGM_STROBE_WIDTH(RefClk);
	Xil_Out32(XSK_EFUSEPS_PGM_STBW_REG, PrgmStrWdthVal);

	RdStrWdthVal = XSK_EFUSEPS_RD_STROBE_WIDTH(RefClk);
	Xil_Out32(XSK_EFUSEPS_RD_STBW_REG, RdStrWdthVal);

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to check whether eFuse bit is xilinx reserved bit or
* not
*
* @param Addr is the address of the eFuse bit.
*
* @return
* 		- XST_SUCCESS if address corresponds to restricted eFuse bit.
*		- XST_FAILURE is address corresponds to non-restricted eFuse bit.
*
* Test Cases:
* 		with different address values
* 		Boundary values for addr
*
****************************************************************************/

u8 XilSKey_EfusePs_IsAddressXilRestricted (u32 Addr)
{
	/**
	 *  Check for xilinx test bits
	 */
	if(	(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x20)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x41)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x62)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x83)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xA4)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xC5)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xE6)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x107) ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x128) ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x149) ||

		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x23F)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x25E)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x27D)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x29C)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2BB)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2DA)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2F9)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x318)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x337)  ||
		(Addr == XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x356)	)
	{
		return TRUE;
	}

	/**
	 *  Check for xilinx reserved bits
	 */
	if ( ((Addr >= XSK_EFUSEPS_APB_TRIM_BITS_START_ADDR) &&
		  (Addr <= XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR)) ||
		 ((Addr >= XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR) &&
		  (Addr <= XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR)) ||
		 ((Addr >= XSK_EFUSEPS_APB_BISR_BITS_START_ADDR) &&
		  (Addr <= XSK_EFUSEPS_APB_BISR_BITS_END_ADDR)) ||
		 ((Addr >= XSK_EFUSEPS_APB_TRIM_BITS_START_ADDR_2ND_HALF) &&
		  (Addr <= XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR_2ND_HALF)) ||
		 ((Addr >= XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR_2ND_HALF) &&
		  (Addr <= XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR_2ND_HALF)) ||
		 ((Addr >= XSK_EFUSEPS_APB_BISR_BITS_START_ADDR_2ND_HALF) &&
		  (Addr <= XSK_EFUSEPS_APB_BISR_BITS_END_ADDR_2ND_HALF))
		) {
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************/
/**
* This function is used to enable the read/write/program the eFUSE
* array.
*
* @param ReadWriteEnable
* 			0x1 - Enable programming
* 			0x2 - Enable read
* 			0x4 - Enable write
* @return
*
* Test Cases
*
****************************************************************************/

void XilSKey_EfusePs_ControllerSetReadWriteEnable(u32 ReadWriteEnable)
{
	u32 RegVal;

	RegVal = Xil_In32(XSK_EFUSEPS_CONTROL_REG);

	/**
	 *  Reset the values
	 * Disable programming
	 * Disable reading
	 * Disable writing
	 */
	RegVal &= ~XSK_EFUSEPS_CONTROL_PS_EN;
	RegVal |= XSK_EFUSEPS_CONTROL_RD_DIS | XSK_EFUSEPS_CONTROL_WR_DIS;

	if (ReadWriteEnable & XSK_EFUSEPS_ENABLE_PROGRAMMING) {
		RegVal |= XSK_EFUSEPS_CONTROL_PS_EN;
	}

	if (ReadWriteEnable & XSK_EFUSEPS_ENABLE_READ) {
		RegVal &= ~XSK_EFUSEPS_CONTROL_RD_DIS;
	}

	if (ReadWriteEnable & XSK_EFUSEPS_ENABLE_WRITE) {
		RegVal &= ~XSK_EFUSEPS_CONTROL_WR_DIS;
	}

	Xil_Out32(XSK_EFUSEPS_CONTROL_REG, RegVal);

	return;
}


/***************************************************************************/
/**
* This function is used to read the eFuse bit value. Before using this
* function set the controller mode and read mode as required. Also, strobe
* width values are to be set properly based on the reference clock for
* successful reading
*
* @param Addr is the address of the eFuse bit.
*
* @param Data has the read eFuse value stored in it.
*
* @return
* 		- XST_SUCCESS for successfully reading the value.
*		- an error when addr is restricted
*
* Test Cases
	Read in Single mode
	Read in redundancy mode
	Read for restricted address
	Boundary Checks for address
*
****************************************************************************/

u32 XilSKey_EfusePs_ReadEfuseBit(u32 Addr, u8 *Data)
{
	if(XilSKey_EfusePs_IsAddressXilRestricted(Addr) != FALSE) {
		return XSK_EFUSEPS_ERROR_ADDRESS_XIL_RESTRICTED;
	}

	*Data = Xil_In32(Addr) & 0x1;

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,"Reading Addr %0x, Data %0x\n", Addr, *Data);

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to program the eFuse bit value. Before using this
* function set the controller mode and read mode as required. Also, strobe
* width values are to be set properly based on the reference clock for
* successful programming
*
* @param Addr is the address of the eFuse bit.
*
* @return
* 		- XST_SUCCESS after successful writing.
*		- an error when addr is restricted
*
* Test Cases
	Write in Single mode
	Write in redundancy mode
	Write for restricted address
	Boundary Checks for address
	Strobe width are not proper (Check if it makes sense)
*
****************************************************************************/

u32 XilSKey_EfusePs_WriteEfuseBit(u32 Addr)
{

	u64 time = 0;

	/**
	 *  Check if Address is restricted
	 */

	if(XilSKey_EfusePs_IsAddressXilRestricted(Addr) != FALSE) {
		return XSK_EFUSEPS_ERROR_ADDRESS_XIL_RESTRICTED;
	}

	/**
	 * Send success when bit is already programmed
	 */
	if ((Xil_In32(Addr)&0x1) == 1) {
		xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
				"Addr %0x already programmed\n", Addr);
		return XST_SUCCESS;
	}

	Xil_Out32(Addr, 0x1);

	/**
	 * Providing 15us delay
	 * Timer takes 100ns as slice. 15us = 150 * 100ns
	 */
	XilSKey_Efuse_SetTimeOut(&time, 150);
	while(1) {
		if(XilSKey_Efuse_IsTimerExpired(time) == 1)
			break;
	}

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
			"Writing Addr %0x, Data %0x\n", Addr, Xil_In32(Addr));

	return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to read the PS efuse status register.
*
* @param	InstancePtr	Pointer to the PS eFUSE instance.
* @param	StatusBits	Buffer to store the status register read.
*
* @return
* 		- XST_SUCCESS.
* 		- XST_FAILURE
* @note		This API unlocks the controller and reads the Zynq
*		PS eFUSE status register.
*
****************************************************************************/
u32 XilSKey_EfusePs_ReadStatus(XilSKey_EPs *InstancePtr, u32 *StatusBits)
{
	u32 RetValue = (u32)XST_FAILURE;


	if (NULL == InstancePtr) {
		return XSK_EFUSEPS_ERROR_PS_STRUCT_NULL;
	}

	/**
	 *  Unlock the eFUSE controller
	 */
	XSK_EFUSEPS_CONTROLER_UNLOCK();

	/**
	 *  Check if the controller is unlocked
	 */
	if (XSK_EFUSEPS_CONTROLER_LOCK_STATUS() == TRUE) {
		RetValue = XSK_EFUSEPS_ERROR_CONTROLLER_LOCK;
		goto ExitFinal;
	}

	/**
	 *  Read the eFUSE status
	 */
	*StatusBits = Xil_In32(XSK_EFUSEPS_STATUS_REG);
	
	RetValue = (u32)XST_SUCCESS;

ExitFinal:
	return RetValue;
}
