/******************************************************************************
*
* Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilisf.c
*
* This file contains the library functions to initialize, control and read the
* device information of the Serial Flash devices. Refer xilisf.h for detailed
* description.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------  -------- -----------------------------------------------
* 1.00a ksu/sdm  03/03/08 First release
* 1.00a sdm      07/02/08 Changed the initialization so that the SPI
*			  Master works in Spi Mode 3 as the In-System Flash
*			  works only in Spi Mode 3
* 2.00a ktn      11/27/09 Updated to use HAL processor APIs/definitions
* 2.01a sdm      01/04/10 Added Support for Winbond W25QXX/W25XX devices
*			  The parameter PagesPerBlock in the struct
*			  IntelStmDeviceGeometry has been renamed to
*			  PagesPerSector.
* 2.03a sdm      04/17/10 Updated to support Winbond memory W25Q128.
* 2.04a sdm      08/17/10 Updated to support Numonyx (N25QXX) and Spansion
*			  flash memories
* 3.00a srt	 06/20/12 Updated to support interfaces SPI PS and QSPI PS.
*			  New API:
*			  	XIsf_RegisterInterface()
*				XIsf_SetSpiConfiguration()
*				XIsf_SetTransferMode()
*			  Changed API:
*			 	XIsf_Initialize()
*				XIsf_Transfer()
*			  Added support to SST flash.
* 3.01a srt	 02/06/13 Updated for changes made in QSPIPS driver (CR 698107).
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilisf.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
/**
 * The following structure specifies the geometry of the Atmel Serial Flash.
 */
typedef struct {
	u8 DeviceCode;			/**< Device code */
	u16 BytesPerPageDefaultMode; 	/**< Bytes per Page in Default mode */
	u16 BytesPerPagePowerOf2Mode;	/**< Bytes per Page in PowerOf2 mode */
	u8 PagesPerBlock;		/**< Number of Pages per Block */
	u8 BlocksPerSector;		/**< Number of Blocks per Sector */
	u8 NumOfSectors;		/**< Number of Sectors in the device */
} AtmelDeviceGeometry;
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION) \
	|| (XPAR_XISF_FLASH_FAMILY == SST))
/**
 * The following structure specifies the geometry of the Intel/STM Serial Flash.
 */
typedef struct {
	u8 ManufacturerID;		/**< Manufacturer code */
	u16 DeviceCode;			/**< Device code */
	u16 BytesPerPage;		/**< Bytes per Page */
	u16 PagesPerSector;		/**< Number of Pages per Sector */
	u16 NumOfSectors;		/**< Number of Sectors in the device */
} IntelStmDeviceGeometry;
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr, u8* ReadPtr,u32 ByteCount);

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
static int AtmelFlashInitialize(XIsf *InstancePtr, u8 *ReadBuf);
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION) \
	|| (XPAR_XISF_FLASH_FAMILY == SST))
static int IntelStmFlashInitialize(XIsf *InstancePtr, u8 *ReadBuf);
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

/************************** Variable Definitions *****************************/

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
static const AtmelDeviceGeometry AtmelDevices[] = {

	{XISF_ATMEL_DEV_AT45DB011D, XISF_BYTES264_PER_PAGE,
	XISF_BYTES256_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS16_PER_SECTOR, XISF_NUM_OF_SECTORS4},

	{XISF_ATMEL_DEV_AT45DB021D, XISF_BYTES264_PER_PAGE,
	XISF_BYTES256_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS16_PER_SECTOR, XISF_NUM_OF_SECTORS8},

	{XISF_ATMEL_DEV_AT45DB041D, XISF_BYTES264_PER_PAGE,
	XISF_BYTES256_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS32_PER_SECTOR, XISF_NUM_OF_SECTORS8},

	{XISF_ATMEL_DEV_AT45DB081D, XISF_BYTES264_PER_PAGE,
	XISF_BYTES256_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS32_PER_SECTOR, XISF_NUM_OF_SECTORS16},

	{XISF_ATMEL_DEV_AT45DB161D, XISF_BYTES528_PER_PAGE,
	XISF_BYTES512_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS32_PER_SECTOR, XISF_NUM_OF_SECTORS16},

	{XISF_ATMEL_DEV_AT45DB321D, XISF_BYTES528_PER_PAGE,
	XISF_BYTES512_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS16_PER_SECTOR, XISF_NUM_OF_SECTORS64},

	{XISF_ATMEL_DEV_AT45DB642D, XISF_BYTES1056_PER_PAGE,
	XISF_BYTES1024_PER_PAGE, XISF_PAGES8_PER_BLOCK,
	XISF_BLOCKS32_PER_SECTOR, XISF_NUM_OF_SECTORS32}
};
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM)|| \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION)\
	|| (XPAR_XISF_FLASH_FAMILY == SST))
static const IntelStmDeviceGeometry IntelStmDevices[] = {
	{XISF_MANUFACTURER_ID_INTEL, XISF_INTEL_DEV_S3316MBIT,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS32},

	{XISF_MANUFACTURER_ID_INTEL, XISF_INTEL_DEV_S3332MBIT,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS64},

	{XISF_MANUFACTURER_ID_INTEL, XISF_INTEL_DEV_S3364MBIT,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS128},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P05_A,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES128_PER_SECTOR,
	 XISF_NUM_OF_SECTORS2},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P10_A,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS2},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P20,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS4},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P40,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS8},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P80,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS16},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P16,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS32},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P32,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS64},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P64,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS128},

	{XISF_MANUFACTURER_ID_STM, XISF_STM_DEV_M25P128,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES1024_PER_SECTOR,
	 XISF_NUM_OF_SECTORS64},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25Q80,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS256},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25Q16,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS512},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25Q32,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS1024},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25Q64,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS2048},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25Q128,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS4096},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X10,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS32},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X20,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS64},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X40,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS128},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X80,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS256},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X16,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS512},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X32,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS1024},

	{XISF_MANUFACTURER_ID_WINBOND, XISF_WB_DEV_W25X64,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES16_PER_SECTOR,
	 XISF_NUM_OF_SECTORS2048},

	{XISF_MANUFACTURER_ID_STM, XISF_NM_DEV_N25Q32,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS64},

	{XISF_MANUFACTURER_ID_STM, XISF_NM_DEV_N25Q64,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS128},

	{XISF_MANUFACTURER_ID_STM, XISF_NM_DEV_N25Q128,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS256},

	{XISF_MANUFACTURER_ID_STM, XISF_MIC_DEV_N25Q128,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS256},

	{XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_DEV_S25FL004,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS8},

	{XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_DEV_S25FL008,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS16},

	{XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_DEV_S25FL016,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS32},

	{XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_DEV_S25FL032,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS64},

	{XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_DEV_S25FL064,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS128},

	{XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_DEV_S25FL128,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS256},

	{XISF_MANUFACTURER_ID_SST, XISF_SST_DEV_SST25WF080,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS256},

};
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* The geometry of the underlying Serial Flash is determined by reading the
* Joint Electron Device Engineering Council (JEDEC) Device Information and
* the Status Register of the Serial Flash.
* This API when called initializes the SPI interface with default settings.
* With custom settings, user should call XIsf_SetSpiConfiguration() and then
* call this API.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	SpiInstPtr is a pointer to the XIsf_Iface instance to be worked on.
* @param	SlaveSelect is a 32-bit mask with a 1 in the bit position of the
*		slave being selected. Only one slave can be selected at a time.
* @param	WritePtr is a pointer to the buffer allocated by the user to be
*		used by the In-system and Serial Flash Library to perform any
*		read/write operations on the Serial Flash device.
*		User applications must pass the address of this buffer for the
*		Library to work.
*		- Write operations :
*			- The size of this buffer should be equal to the Number
*			of bytes to be written to the Serial Flash +
*			XISF_CMD_MAX_EXTRA_BYTES.
*			- The size of this buffer should be large enough for
*			usage across all the applications that use a common
*			instance of the Serial Flash.
*			- A minimum of one byte and a maximum of ISF_PAGE_SIZE
*			bytes can be written to the Serial Flash, through a
*			single Write operation.
* 		- Read operations :
* 			- The size of this buffer should be equal to
*			XISF_CMD_MAX_EXTRA_BYTES, if the application only reads
*			from the Serial Flash (no write operations).
*
* @return	- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STOPPED if the device must be started before
*		transferring data.
*		- XST_FAILURE, otherwise.
*
* @note		- The XIsf_Initialize() API is a blocking call (for both
*		polled and interrupt modes of the Spi driver). It reads the
*		JEDEC information of the device and waits till the transfer is
*		complete before checking if the information is valid.
*		- This library can support multiple instances of Serial Flash at
*		a time, provided they are of the same device family (either
*		Atmel, Intel or STM, Winbond or Spansion) as the device family
*		is selected at compile time.
*
******************************************************************************/
int XIsf_Initialize(XIsf *InstancePtr, XIsf_Iface *SpiInstPtr, u8 SlaveSelect,
				u8 *WritePtr)
{
	int Status;
	u8 ReadBuf[XISF_INFO_READ_BYTES + XISF_INFO_EXTRA_BYTES];

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (SpiInstPtr == NULL) {
		return XST_FAILURE;
	}

	if (WritePtr == NULL) {
		return XST_FAILURE;
	}

	InstancePtr->IsReady = FALSE;
	InstancePtr->SpiSlaveSelect = SlaveSelect;
	InstancePtr->WriteBufPtr = WritePtr;

#ifdef XPAR_XISF_INTERFACE_AXISPI
	if (SpiInstPtr->IsStarted != XIL_COMPONENT_IS_STARTED) {
		 return XST_DEVICE_IS_STOPPED;
	}
#endif

	if (!InstancePtr->RegDone) {
		XIsf_SetSpiConfiguration(InstancePtr, SpiInstPtr,
				XISF_SPI_OPTIONS, XISF_SPI_PRESCALER);
	}

	/*
         * Get the JEDEC Device Info.
         * The IsReady is temporarily made TRUE and
	 * transfer is set in polling mode to fetch the JEDEC Info.
         */
	XIsf_SetTransferMode(InstancePtr, XISF_POLLING_MODE);
        InstancePtr->IsReady = TRUE;
        Status = XIsf_GetDeviceInfo(InstancePtr, ReadBuf);
        InstancePtr->IsReady = FALSE;
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef XPAR_XISF_INTERFACE_AXISPI
	/*
	 * Wait until the transfer is complete.
	 */
	do {
		Status =
		InstancePtr->XIsf_Iface_SetSlaveSelect(InstancePtr->SpiInstPtr,
					InstancePtr->SpiSlaveSelect);
	} while(Status == XST_DEVICE_BUSY);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Check for Atmel Serial Flash.
	 */
	Status = AtmelFlashInitialize(InstancePtr, ReadBuf);

#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION) \
	|| (XPAR_XISF_FLASH_FAMILY == SST))

	/*
	 * Check for Intel/STM/Winbond/Spansion Serial Flash.
	 */
	Status = IntelStmFlashInitialize(InstancePtr, ReadBuf);

#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	(XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) */

	return Status;
}


/*****************************************************************************/
/**
*
* This API sets the configuration of SPI.  This will set the options and
* clock prescaler (if applicable).
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	SpiInstPtr is a pointer to the XIsf_Iface instance to be worked on.
* @param	Options contains specified options to be set.
* @param	PreScaler is the value of the clock prescaler to set.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This API can be called before calling XIsf_Initialize()
*		to initialize the SPI interface in other than default options mode.
*		PreScaler is only applicable to PS SPI/QSPI.
*
******************************************************************************/
int XIsf_SetSpiConfiguration(XIsf *InstancePtr, XIsf_Iface *SpiInstPtr,
				u32 Options, u8 PreScaler)
{
	int Status;

	if (!InstancePtr->RegDone) {
		XIsf_RegisterInterface(InstancePtr);
		InstancePtr->SpiInstPtr = SpiInstPtr;
		InstancePtr->RegDone = TRUE;
	}

	Status = InstancePtr->XIsf_Iface_SetOptions(InstancePtr->SpiInstPtr,
						Options);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (InstancePtr->XIsf_Iface_SetClkPrescaler) {
		Status =
		InstancePtr->XIsf_Iface_SetClkPrescaler(InstancePtr->SpiInstPtr,
						PreScaler);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
        }

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This API reads the Serial Flash Status Register.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	ReadPtr is a pointer to the memory where the Status Register
*		content is copied.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The contents of the Status Register is stored at the second byte
*		pointed by the ReadPtr.
*
******************************************************************************/
int XIsf_GetStatus(XIsf *InstancePtr, u8 *ReadPtr)
{
	int Status;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	if (ReadPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_STATUSREG_READ;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, ReadPtr,
				XISF_STATUS_RDWR_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This API reads the Serial Flash Status Register 2.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	ReadPtr is a pointer to the memory where the Status Register
*		content is copied.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The contents of the Status Register 2 is stored at the second
*		byte pointed by the ReadPtr.
*		This operation is available only in Winbond Serial Flash.
*
******************************************************************************/
#if (XPAR_XISF_FLASH_FAMILY == WINBOND)
int XIsf_GetStatusReg2(XIsf *InstancePtr, u8 *ReadPtr)
{
	int Status;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	if (ReadPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_STATUSREG2_READ;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, ReadPtr,
				XISF_STATUS_RDWR_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This API reads the Joint Electron Device Engineering Council (JEDEC)
* information of the Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	ReadPtr is a pointer to the buffer where the Device information
*		is copied.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The Device information is stored at the second byte pointed
*		by the ReadPtr.
*
******************************************************************************/
int XIsf_GetDeviceInfo(XIsf *InstancePtr, u8 *ReadPtr)
{
	int Status;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	if (ReadPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the Write Buffer.
	 */
	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_ISFINFO_READ;
	InstancePtr->WriteBufPtr[BYTE2] = XISF_DUMMYBYTE;
	InstancePtr->WriteBufPtr[BYTE3] = XISF_DUMMYBYTE;
	InstancePtr->WriteBufPtr[BYTE4] = XISF_DUMMYBYTE;
	InstancePtr->WriteBufPtr[BYTE5] = XISF_DUMMYBYTE;

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
				ReadPtr, XISF_INFO_READ_BYTES);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This API Enables/Disables writes to the Intel, STM, Winbond and Spansion
* Serial Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	WriteEnable specifies whether to Enable (XISF_CMD_ENABLE_WRITE)
*		or Disable (XISF_CMD_DISABLE_WRITE) the writes to the
*		Serial Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This API works only for Intel, STM, Winbond and Spansion Serial
*		Flash. If this API is called for Atmel Flash, XST_FAILURE is
*		returned.
*
******************************************************************************/
int XIsf_WriteEnable(XIsf *InstancePtr, u8 WriteEnable)
{
	int Status = XST_FAILURE;
#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION) \
	|| (XPAR_XISF_FLASH_FAMILY == SST))

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	if (WriteEnable == XISF_WRITE_ENABLE) {

		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_ENABLE_WRITE;

	} else if (WriteEnable == XISF_WRITE_DISABLE) {

		InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_DISABLE_WRITE;
	} else {

		return Status;
	}

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				XISF_CMD_WRITE_ENABLE_DISABLE_BYTES);

#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

	return Status;
}

/*****************************************************************************/
/**
*
* This API configures and controls the Intel, STM, Winbond and Spansion Serial
* Flash.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	Operation is the type of Control operation to be performed
*		on the Serial Flash.
*		The different control operations are
		- XISF_RELEASE_DPD: Release from Deep Power Down (DPD) Mode
		- XISF_ENTER_DPD: Enter DPD Mode
		- XISF_CLEAR_SR_FAIL_FLAGS: Clear the Status Register Fail Flags
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*		- Atmel Serial Flash does not support any of these operations.
*		- Intel Serial Flash support Enter/Release from DPD Mode and
*		Clear Status Register Fail Flags.
*		- STM, Winbond and Spansion Serial Flash support Enter/Release
*		from DPD Mode.
*		- Winbond (W25QXX) Serial Flash support Enable High Performance
*		mode.
*
******************************************************************************/
int XIsf_Ioctl(XIsf *InstancePtr, XIsf_IoctlOperation Operation)
{
	int Status = XST_FAILURE;

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION) \
	|| (XPAR_XISF_FLASH_FAMILY == SST))
	u8 NumBytes;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	switch (Operation) {
		case XISF_IOCTL_RELEASE_DPD:
			InstancePtr->WriteBufPtr[BYTE1] =
				XISF_CMD_RELEASE_FROM_DPD;
			NumBytes = XISF_IOCTL_BYTES;
			break;

		case XISF_IOCTL_ENTER_DPD:
			InstancePtr->WriteBufPtr[BYTE1] =
				XISF_CMD_DEEP_POWER_DOWN;
			NumBytes = XISF_IOCTL_BYTES;
			break;

#if (XPAR_XISF_FLASH_FAMILY == INTEL)
		case XISF_IOCTL_CLEAR_SR_FAIL_FLAGS:
			InstancePtr->WriteBufPtr[BYTE1] =
				XISF_CMD_CLEAR_SRFAIL_FLAGS;
			NumBytes = XISF_IOCTL_BYTES;
			break;
#endif /* (XPAR_XISF_FLASH_FAMILY == INTEL) */

#if (XPAR_XISF_FLASH_FAMILY == WINBOND)
		case XISF_IOCTL_ENABLE_HI_PERF_MODE:
			InstancePtr->WriteBufPtr[BYTE1] =
				XISF_CMD_ENABLE_HPM;
			NumBytes = XISF_HPM_BYTES;
			break;
#endif /* (XPAR_XISF_FLASH_FAMILY == INTEL) */

		default:
			return XST_FAILURE;
	}

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULL,
				NumBytes);
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

	return Status;
}

/*****************************************************************************/
/*
*
* This function performs the SPI transfer.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	WritePtr is a pointer to the memory which contains the data to
*		be transferred to the Serial Flash .
* @param	ReadPtr is a pointer to the memory where the data read from the
*		Serial Flash is stored.
* @param	ByteCount is the number of bytes to be read from/written to the
*		Serial Flash.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function is internal to the In-system and Serial Flash
*		Library. It works with both interrupt mode and polled mode SPI
*		transfers. In polled mode for AXI SPI, the user has to disable
*		the Global Interrupts in the user application, after the Spi
*		is Initialized and Spi driver is started
*
******************************************************************************/
int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr, u8* ReadPtr, u32 ByteCount)
{
	int Status;

	/*
	 * Select the Serial Flash as a slave.
	 */
#ifndef XPAR_XISF_INTERFACE_PSQSPI
	Status = InstancePtr->XIsf_Iface_SetSlaveSelect(InstancePtr->SpiInstPtr,
					InstancePtr->SpiSlaveSelect);
#else
	Status = InstancePtr->XIsf_Iface_SetSlaveSelect(InstancePtr->SpiInstPtr);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the transfer.
	 */
#ifdef XPAR_XISF_INTERFACE_AXISPI
	Status = InstancePtr->XIsf_Iface_Transfer(InstancePtr->SpiInstPtr,
			WritePtr, ReadPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
#endif

	if (InstancePtr->IntrMode == XISF_INTERRUPT_MODE) {
		Status = InstancePtr->XIsf_Iface_Transfer(InstancePtr->SpiInstPtr,
				WritePtr, ReadPtr, ByteCount);
	} else {
		Status =
		InstancePtr->XIsf_Iface_PolledTransfer(InstancePtr->SpiInstPtr,
				WritePtr, ReadPtr, ByteCount);
	}

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/*
*
* This function registers the interface SPI/SPI PS/QSPI PS.
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	None
*
*
******************************************************************************/
void XIsf_RegisterInterface(XIsf *InstancePtr)
{
#ifdef XPAR_XISF_INTERFACE_AXISPI
	InstancePtr->XIsf_Iface_SetOptions = XSpi_SetOptions;
	InstancePtr->XIsf_Iface_SetSlaveSelect = XSpi_SetSlaveSelect;
	InstancePtr->XIsf_Iface_Transfer = XSpi_Transfer;
	InstancePtr->XIsf_Iface_Start = XSpi_Start;
#elif XPAR_XISF_INTERFACE_PSSPI
	InstancePtr->XIsf_Iface_SetOptions = XSpiPs_SetOptions;
	InstancePtr->XIsf_Iface_SetSlaveSelect = XSpiPs_SetSlaveSelect;
	InstancePtr->XIsf_Iface_Transfer = XSpiPs_Transfer;
	InstancePtr->XIsf_Iface_PolledTransfer = XSpiPs_PolledTransfer;
	InstancePtr->XIsf_Iface_SetClkPrescaler = XSpiPs_SetClkPrescaler;
#elif XPAR_XISF_INTERFACE_PSQSPI
	InstancePtr->XIsf_Iface_SetOptions = XQspiPs_SetOptions;
	InstancePtr->XIsf_Iface_SetSlaveSelect = XQspiPs_SetSlaveSelect;
	InstancePtr->XIsf_Iface_Transfer = XQspiPs_Transfer;
	InstancePtr->XIsf_Iface_PolledTransfer = XQspiPs_PolledTransfer;
	InstancePtr->XIsf_Iface_SetClkPrescaler = XQspiPs_SetClkPrescaler;
#endif
}


#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
/*****************************************************************************/
/**
*
* This function initializes the instance structure with the device geometry of
* the Atmel Serial Flash if it is an Atmel device.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferPtr is a pointer to the memory where the device info of
*		the Serial Flash is present.
*
* @return	- XST_SUCCESS if device information matches the JEDEC
*		information of the Atmel Serial Flash.
*		- XST_FAILURE if the device information doesn't match with Atmel
*		Serial Flash.
*
* @note		None
*
******************************************************************************/
static int AtmelFlashInitialize(XIsf *InstancePtr, u8 *BufferPtr)
{
	int Status;
	u32 Index;
	u8 StatusRegister;
	u8 NumOfDevices;
	u8 ManufacturerID;

	ManufacturerID = BufferPtr[BYTE2];
	if (ManufacturerID == XISF_MANUFACTURER_ID_ATMEL) {

		/*
		 * For Atmel Serial Flash the device code is the 3rd byte of the
		 * JEDEC info.
		 */
		InstancePtr->DeviceCode = BufferPtr[BYTE3];

		/*
		 * Get the Status Register contents.
		 * The IsReady is temporarily made TRUE to fetch the Status
		 * Register contents.
		 */
		InstancePtr->IsReady = TRUE;
		Status = XIsf_GetStatus(InstancePtr, BufferPtr);
		InstancePtr->IsReady = FALSE;
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Wait until the transfer is complete.
		 */
		do {
#ifndef XPAR_XISF_INTERFACE_PSQSPI
			Status =
			InstancePtr->XIsf_Iface_SetSlaveSelect(InstancePtr->SpiInstPtr,
					InstancePtr->SpiSlaveSelect);
#else
			Status =
			InstancePtr->XIsf_Iface_SetSlaveSelect(InstancePtr->SpiInstPtr);
#endif
		} while(Status == XST_DEVICE_BUSY);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		StatusRegister = BufferPtr[BYTE2];

		/*
		 * Get the Address mode from the Status Register of the Serial
		 * Flash.
		 */
		InstancePtr->AddrMode = StatusRegister & XISF_SR_ADDR_MODE_MASK;

		/*
		 * Update the Serial Flash instance structure with device
		 * geometry.
		 */
		 NumOfDevices = sizeof(AtmelDevices) /
		 		sizeof(AtmelDeviceGeometry);

		 for(Index = 0; Index < NumOfDevices; Index++) {
			 if (InstancePtr->DeviceCode == AtmelDevices[Index].
			 				DeviceCode) {
				/*
				 * Default address mode device.
				 */
				if (InstancePtr->AddrMode ==
						XISF_DEFAULT_ADDRESS) {
					InstancePtr->BytesPerPage =
						AtmelDevices [Index].
						BytesPerPageDefaultMode;
				} else {
					/*
					 * Power of 2 address mode device.
					 */
					 InstancePtr->BytesPerPage =
					 	AtmelDevices [Index].
					 	BytesPerPagePowerOf2Mode;
				}

				InstancePtr->PagesPerBlock =
					AtmelDevices[Index].PagesPerBlock;

				InstancePtr->BlocksPerSector =
					AtmelDevices[Index].BlocksPerSector;

				InstancePtr->NumOfSectors =
					AtmelDevices[Index].NumOfSectors;

				if (InstancePtr->BytesPerPage >
						XISF_BYTES1024_PER_PAGE ) {
					InstancePtr->ByteMask =
						XISF_BYTES2048_PER_PAGE_MASK;
				}
				else if (InstancePtr->BytesPerPage >
						XISF_BYTES512_PER_PAGE ) {
					InstancePtr->ByteMask =
						XISF_BYTES1024_PER_PAGE_MASK;
				}
				else if (InstancePtr->BytesPerPage >
						XISF_BYTES256_PER_PAGE ) {
					InstancePtr->ByteMask =
						XISF_BYTES512_PER_PAGE_MASK;
				}
				else {
					InstancePtr->ByteMask =
						XISF_BYTES256_PER_PAGE_MASK;
				}

				InstancePtr->IsReady = TRUE;
			}
		}
	}

	/*
	 * If the device is not supported, return Failure.
	 */
	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SPANSION) \
	|| (XPAR_XISF_FLASH_FAMILY == SST))
/*****************************************************************************/
/**
*
* This function initializes the instance structure with the device geometry of
* the Intel/Stm/Winbond Serial Flash if it is an Intel/Stm/Winbond device.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferPtr is a pointer to the memory where the device info of
*		the Serial Flash is present.
*
* @return	- XST_SUCCESS if device information matches the JEDEC
*		information of Intel or Stm or Winbond Serial Flash.
*		- XST_FAILURE if the device information doesn't match with Intel
*		or Stm or Winbond Serial Flash.
*
* @note		None
*
******************************************************************************/
static int IntelStmFlashInitialize(XIsf *InstancePtr, u8 *BufferPtr)
{
	u32 Index;
	u8 NumOfDevices;
	u8 ManufacturerID;

	ManufacturerID = BufferPtr[BYTE2];

#if (XPAR_XISF_FLASH_FAMILY == INTEL)
	/*
	 * For Intel the device code is the 4th byte of the JEDEC info.
	 */
	InstancePtr->DeviceCode = BufferPtr[BYTE4];
#else
	/*
	 * For STM/Winbond/Spansion Serial Flash the device code is the 3rd/4th
	 * byte of the JEDEC info. The Third Byte is the Memory Type and the 4th
	 * byte represents the capacity.
	 */
	InstancePtr->DeviceCode = (BufferPtr[BYTE3] << 8) | BufferPtr[BYTE4];
#endif

	/*
	 * Check for Intel/STM/Winbond/Spansion Serial Flash.
	 */
	 NumOfDevices = sizeof(IntelStmDevices) /
			sizeof(IntelStmDeviceGeometry);

	 for(Index = 0; Index < NumOfDevices; Index++) {
		 if ((InstancePtr->DeviceCode ==
		      IntelStmDevices[Index].DeviceCode) &&
		     (ManufacturerID ==
		      IntelStmDevices[Index].ManufacturerID)) {
			InstancePtr->BytesPerPage =
				IntelStmDevices[Index].BytesPerPage;

			/*
			 * This is number of pages per Sector.
			 */
			InstancePtr->PagesPerBlock =
				IntelStmDevices[Index].PagesPerSector;

			InstancePtr->BlocksPerSector = 0;

			InstancePtr->NumOfSectors =
				IntelStmDevices[Index].NumOfSectors;

			InstancePtr->IsReady = TRUE;
		}
	}

	/*
	 * If the device is not supported, return Failure.
	 */
	if (InstancePtr->IsReady != TRUE) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */
