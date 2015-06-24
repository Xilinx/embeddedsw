/******************************************************************************
*
* Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* 2.00a ktn	11/27/09 Updated to use HAL processor APIs/definitions
* 2.01a sdm	01/04/10 Added Support for Winbond W25QXX/W25XX devices
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
* 3.01a srt	 02/06/13 Updated for changes made in QSPIPS driver
*			  (CR 698107).
* 5.0   sb	 08/05/14 Updated for support for > 128 MB flash for PSQSPI
*			  Interface. Added Library Handler API which will
*			  register to driver interrupts, based upon the
*			  interface selected.
*			  New API:
*				SpaMicWinFlashInitialize()
*				GetRealAddr()
*				SendBankSelect()
*				XIsf_SetStatusHandler()
*				void XIsf_IfaceHandler()
*			  Changed API:
*				- XIsf_Initialize()
*				Added Call back to lib interrupt handler
*				after XIsf_Transfer Call
*				- XIsf_Transfer()
*				- XIsf_GetStatus()
*				- XIsf_GetStatusReg2()
*				- XIsf_GetDeviceInfo()
*				- XIsf_WriteEnable()
*				- XIsf_Ioctl()
* 5.1   sb	 12/23/14 Added check for flash interface for Winbond, Spansion
*			  and Micron flash family for PSQSPI.
* 5.2   asa  05/12/15 Added APIs to support 4 byte addressing for Micron flash.
*                     2 APIs were added, one to enter into 4 byte mode and the other
*                     to exit from the same.
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

#if (((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) \
	|| (XPAR_XISF_FLASH_FAMILY == SST) || 		\
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || 		\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) &&	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))		\
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
#endif /* (((XPAR_XISF_FLASH_FAMILY == INTEL) ||
	(XPAR_XISF_FLASH_FAMILY == STM) || \
	(XPAR_XISF_FLASH_FAMILY == SST) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))*/

#if (((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	defined(XPAR_XISF_INTERFACE_PSQSPI))
/**
 * The following structure specifies the geometry of the Spansion/Micron
 * Serial Flash.
 */
typedef struct {
	u32 SectSize;		/**< Individual sector size or
				 * combined sector size in case of parallel
				 * config*/
	u32 NumSect;		/**< Total no. of sectors in one/two
				 * flash devices */
	u32 PageSize;		/**< Individual page size or
				 * combined page size in case of parallel
				 * config*/
	u32 NumPage;		/**< Total no. of pages in one/two flash
				 * devices */
	u32 FlashDeviceSize;	/**< This is the size of one flash device
				 * NOT the combination of both devices,
				 * if present */
	u8 ManufacturerID;	/**< Manufacturer ID - used to identify make */
	u8 DeviceIDMemSize;	/**< Byte of device ID indicating the memory
				 * size */
	u32 SectMask;		/**< Mask to get sector start address */
	u8 NumDie;		/**< No. of die forming a single flash */
} SpaMicWinDeviceGeometry;

#endif /* (((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	defined(XPAR_XISF_INTERFACE_PSQSPI)) */


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr, u8* ReadPtr,u32 ByteCount);

u32 GetRealAddr(XIsf_Iface *QspiPtr, u32 Address);

#ifdef XPAR_XISF_INTERFACE_PSQSPI
int SendBankSelect(XIsf *InstancePtr, u32 BankSel);
#endif

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
static int AtmelFlashInitialize(XIsf *InstancePtr, u8 *ReadBuf);
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if (((XPAR_XISF_FLASH_FAMILY == SPANSION)) &&	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
static int IntelStmFlashInitialize(XIsf *InstancePtr, u8 *ReadBuf);
#endif /* (((XPAR_XISF_FLASH_FAMILY == INTEL) ||	\
	(XPAR_XISF_FLASH_FAMILY == STM) ||	\
    (XPAR_XISF_FLASH_FAMILY == SST) ||	\
    (XPAR_XISF_FLASH_FAMILY == WINBOND) || \
    (XPAR_XISF_FLASH_FAMILY == SPANSION)) &&	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI))) */

#if (((XPAR_XISF_FLASH_FAMILY == WINBOND) ||	\
    (XPAR_XISF_FLASH_FAMILY == SPANSION)) && 	\
	defined(XPAR_XISF_INTERFACE_PSQSPI))
static int SpaMicWinFlashInitialize(XIsf *InstancePtr, u8 *BufferPtr);

#endif /* (((XPAR_XISF_FLASH_FAMILY == WINBOND) ||	\
    (XPAR_XISF_FLASH_FAMILY == SPANSION)) && 	\
	defined(XPAR_XISF_INTERFACE_PSQSPI)) */

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

#if (((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) \
	|| (XPAR_XISF_FLASH_FAMILY == SST) || 		\
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || 		\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && 	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
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

	{XISF_MANUFACTURER_ID_MICRON, XISF_MIC_DEV_N25Q256_3V0,
	 XISF_BYTES256_PER_PAGE, XISF_PAGES256_PER_SECTOR,
	 XISF_NUM_OF_SECTORS512},

};
#endif /* (((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	|| (XPAR_XISF_FLASH_FAMILY == SST) || 		\
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || 		\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && 	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))*/

#if (((XPAR_XISF_FLASH_FAMILY == WINBOND) || 	\
    (XPAR_XISF_FLASH_FAMILY == SPANSION)) &&	\
    defined(XPAR_XISF_INTERFACE_PSQSPI))
static const SpaMicWinDeviceGeometry SpaMicWinDevices[] = {
	{0x10000, 0x100, 256, 0x10000, 0x1000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_128,
		0xFFFF0000, 1},
	{0x10000, 0x200, 256, 0x20000, 0x1000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_128,
		0xFFFF0000, 1},
	{0x20000, 0x100, 512, 0x10000, 0x1000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_128,
		0xFFFE0000, 1},
	{0x10000, 0x200, 256, 0x20000, 0x2000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_256,
		0xFFFF0000, 1},
	{0x10000, 0x400, 256, 0x40000, 0x2000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_256,
		0xFFFF0000, 1},
	{0x20000, 0x200, 512, 0x20000, 0x2000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_256,
		0xFFFE0000, 1},
	{0x40000, 0x100, 512, 0x20000, 0x4000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_512,
		0xFFFC0000, 1},
	{0x40000, 0x200, 512, 0x40000, 0x4000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_512,
		0xFFFC0000, 1},
	{0x80000, 0x100, 1024, 0x20000, 0x4000000,
		XISF_MANUFACTURER_ID_SPANSION, XISF_SPANSION_ID_BYTE2_512,
		0xFFF80000, 1},
	/* Spansion 1Gbit is handled as 512Mbit stacked */
	/* Micron */
	{0x10000, 0x100, 256, 0x10000, 0x1000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_128,
		0xFFFF0000, 1},
	{0x10000, 0x200, 256, 0x20000, 0x1000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_128,
		0xFFFF0000, 1},
	{0x20000, 0x100, 512, 0x10000, 0x1000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_128,
		0xFFFE0000, 1},
	{0x10000, 0x200, 256, 0x20000, 0x2000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_256,
		0xFFFF0000, 1},
	{0x10000, 0x400, 256, 0x40000, 0x2000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_256,
		0xFFFF0000, 1},
	{0x20000, 0x200, 512, 0x20000, 0x2000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_256,
		0xFFFE0000, 1},
	{0x10000, 0x400, 256, 0x40000, 0x4000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_512,
		0xFFFF0000, 2},
	{0x10000, 0x800, 256, 0x80000, 0x4000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_512,
		0xFFFF0000, 2},
	{0x20000, 0x400, 512, 0x40000, 0x4000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_512,
		0xFFFE0000, 2},
	{0x10000, 0x800, 256, 0x80000, 0x8000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_1G,
		0xFFFF0000, 4},
	{0x10000, 0x1000, 256, 0x100000, 0x8000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_1G,
		0xFFFF0000, 4},
	{0x20000, 0x800, 512, 0x80000, 0x8000000,
		XISF_MANUFACTURER_ID_MICRON, XISF_MICRON_ID_BYTE2_1G,
		0xFFFE0000, 4},
	/* Winbond */
	{0x10000, 0x100, 256, 0x10000, 0x1000000,
		XISF_MANUFACTURER_ID_WINBOND, XISF_WINBOND_ID_BYTE2_128,
		0xFFFF0000, 1},
	{0x10000, 0x200, 256, 0x20000, 0x1000000,
		XISF_MANUFACTURER_ID_WINBOND, XISF_WINBOND_ID_BYTE2_128,
		0xFFFF0000, 1},
	{0x20000, 0x100, 512, 0x10000, 0x1000000,
		XISF_MANUFACTURER_ID_WINBOND, XISF_WINBOND_ID_BYTE2_128,
		0xFFFE0000, 1}
};
#endif /* (((XPAR_XISF_FLASH_FAMILY == WINBOND) || 	\
	  (XPAR_XISF_FLASH_FAMILY == SPANSION)) &&	\
	  defined(XPAR_XISF_INTERFACE_PSQSPI)) */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile unsigned int XIsf_TransferInProgress;
u32 XIsf_StatusEventInfo;
unsigned int XIsf_ByteCountInfo;
static u32 XIsf_FCTIndex;
/************************** Function Definitions *****************************/

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
* @param	SpiInstPtr is a pointer to XIsf_Iface instance to be worked on.
* @param	SlaveSelect is a 32-bit mask with a 1 in the bit position of
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
*		- This library can support multiple instances of Serial Flash
*		at a time, provided they are of the same device family (either
*		Atmel, Intel or STM, Winbond or Spansion) as the device family
*		is selected at compile time.
*
******************************************************************************/
int XIsf_Initialize(XIsf *InstancePtr, XIsf_Iface *SpiInstPtr, u8 SlaveSelect,
				u8 *WritePtr)
{
	int Status;
	u8 ReadBuf[XISF_INFO_READ_BYTES + XISF_INFO_EXTRA_BYTES] = {0};

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (SpiInstPtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (WritePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	InstancePtr->IsReady = FALSE;
	InstancePtr->SpiSlaveSelect = SlaveSelect;
	InstancePtr->WriteBufPtr = WritePtr;

#ifdef XPAR_XISF_INTERFACE_AXISPI
	if (SpiInstPtr->IsStarted != XIL_COMPONENT_IS_STARTED) {
		 return XST_DEVICE_IS_STOPPED;
	}
#endif

	if ((!InstancePtr->RegDone) != 0) {
		(void)XIsf_SetSpiConfiguration(InstancePtr, SpiInstPtr,
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
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
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

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

#endif

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
	/*
	 * Check for Atmel Serial Flash.
	 */
	Status = AtmelFlashInitialize(InstancePtr, ReadBuf);

#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if (((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) \
	|| (XPAR_XISF_FLASH_FAMILY == SST) || 		\
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || 		\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && 	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))

	/*
	 * Check for Intel/STM/Winbond/Spansion Serial Flash.
	 */
	Status = IntelStmFlashInitialize(InstancePtr, ReadBuf);

#endif /* (((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	 || (XPAR_XISF_FLASH_FAMILY == SST) || 		\
	 (XPAR_XISF_FLASH_FAMILY == WINBOND) || 	\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && 	\
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))*/
#if (((XPAR_XISF_FLASH_FAMILY == WINBOND) || 	\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	defined(XPAR_XISF_INTERFACE_PSQSPI))

	Status = SpaMicWinFlashInitialize(InstancePtr, ReadBuf);

#endif /*(((XPAR_XISF_FLASH_FAMILY == WINBOND) ||	\
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) &&	\
	defined(XPAR_XISF_INTERFACE_PSQSPI)) */

	return Status;
}


/*****************************************************************************/
/**
*
* This API sets the configuration of SPI.  This will set the options and
* clock prescaler (if applicable).
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	SpiInstPtr is a pointer to XIsf_Iface instance to be worked on.
* @param	Options contains specified options to be set.
* @param	PreScaler is the value of the clock prescaler to set.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This API can be called before calling XIsf_Initialize()
*		to initialize the SPI interface in other than default options
*		mode. PreScaler is only applicable to PS SPI/QSPI.
*
******************************************************************************/
int XIsf_SetSpiConfiguration(XIsf *InstancePtr, XIsf_Iface *SpiInstPtr,
				u32 Options, u8 PreScaler)
{
	int Status;

	if ((!InstancePtr->RegDone) != 0) {
		XIsf_RegisterInterface(InstancePtr);
		InstancePtr->SpiInstPtr = SpiInstPtr;
		InstancePtr->RegDone = TRUE;
	}

	Status = InstancePtr->XIsf_Iface_SetOptions(InstancePtr->SpiInstPtr,
						Options);
	if (Status != (int)(XST_SUCCESS)){
		return (int)(XST_FAILURE);
	}

	if ((InstancePtr->XIsf_Iface_SetClkPrescaler) != NULL) {
		Status = InstancePtr->XIsf_Iface_SetClkPrescaler(
		    		InstancePtr->SpiInstPtr, PreScaler);
		if (Status != (int)(XST_SUCCESS)){
			return (int)(XST_FAILURE);
		}
	}

	return (int)(XST_SUCCESS);
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
* @note		The contents of the Status Register is stored at second byte
*		pointed by the ReadPtr.
*
******************************************************************************/
int XIsf_GetStatus(XIsf *InstancePtr, u8 *ReadPtr)
{
	int Status;
	u8 Mode;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
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
	/*
	 * Get the Transfer Mode
	 */
	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
		InstancePtr->StatusHandler(InstancePtr,
			XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
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
	u8 Mode;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
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
	/*
	 * Get the Transfer Mode
	 */
	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
			InstancePtr->StatusHandler(InstancePtr,
				XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
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
	u8 Mode;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (ReadPtr == NULL) {
		return (int)(XST_FAILURE);
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
	/*
	 * Get the Transfer Mode
	 */
	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
			InstancePtr->StatusHandler(InstancePtr,
				XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
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
	int Status = (int)(XST_FAILURE);
	u8 Mode;
	u8 WriteEnableBuf[1] = {0};
	u8 * NULLPtr = NULL;
#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) ||  \
    (XPAR_XISF_FLASH_FAMILY == SPANSION) || (XPAR_XISF_FLASH_FAMILY == SST))

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (WriteEnable == XISF_WRITE_ENABLE) {

		WriteEnableBuf[BYTE1] = XISF_CMD_ENABLE_WRITE;

	} else if (WriteEnable == XISF_WRITE_DISABLE) {

		WriteEnableBuf[BYTE1] = XISF_CMD_DISABLE_WRITE;
	} else {

		return Status;
	}

	Xil_AssertNonvoid(NULLPtr == NULL);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, WriteEnableBuf, NULLPtr,
				XISF_CMD_WRITE_ENABLE_DISABLE_BYTES);

	/*
	 * Get the Transfer Mode
	 */
	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
		InstancePtr->StatusHandler(InstancePtr,
			XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}


#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	   (XPAR_XISF_FLASH_FAMILY == WINBOND) ||	\
	   (XPAR_XISF_FLASH_FAMILY == SPANSION) ||	\
	   (XPAR_XISF_FLASH_FAMILY == SST)) */

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
		- XISF_CLEAR_SR_FAIL_FLAGS: Clear Status Register Fail Flags
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
	int Status;
	u8 Mode;
	u8* NULLPtr = NULL;

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) ||  \
    (XPAR_XISF_FLASH_FAMILY == SPANSION) || (XPAR_XISF_FLASH_FAMILY == SST))
	u8 NumBytes;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
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
#endif /* (XPAR_XISF_FLASH_FAMILY == WINBOND) */

		default:
			return (int)(XST_FAILURE);
	}

	Xil_AssertNonvoid(NULLPtr == NULL);

	/*
	 * Initiate the Transfer.
	 */
	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr, NULLPtr,
				NumBytes);
	/*
	 * Get the Transfer Mode
	 */
	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
		InstancePtr->StatusHandler(InstancePtr,
			XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

#else
	Status = (int)(XST_FAILURE);

#endif/* ((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
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
	Status = InstancePtr->XIsf_Iface_SetSlaveSelect(
			InstancePtr->SpiInstPtr,InstancePtr->SpiSlaveSelect);
#else
	Status = InstancePtr->XIsf_Iface_SetSlaveSelect(
			InstancePtr->SpiInstPtr);
#endif
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	/*
	 * Start the transfer.
	 */
#ifdef XPAR_XISF_INTERFACE_AXISPI
	if (InstancePtr->IntrMode == XISF_INTERRUPT_MODE) {
		XIsf_TransferInProgress = TRUE;
	}
	Status = InstancePtr->XIsf_Iface_Transfer(InstancePtr->SpiInstPtr,
			WritePtr, ReadPtr, ByteCount);
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IntrMode == XISF_INTERRUPT_MODE) {
		while (XIsf_TransferInProgress != 0){
			/*NOP*/
		}
	}

	return (int)(XST_SUCCESS);
#endif

	if (InstancePtr->IntrMode == XISF_INTERRUPT_MODE) {
#if defined(XPAR_XISF_INTERFACE_PSQSPI) || defined(XPAR_XISF_INTERFACE_PSSPI)
		XIsf_TransferInProgress = TRUE;
#endif
		Status = InstancePtr->XIsf_Iface_Transfer(
				InstancePtr->SpiInstPtr,
					WritePtr, ReadPtr, ByteCount);
#if defined(XPAR_XISF_INTERFACE_PSQSPI) || defined(XPAR_XISF_INTERFACE_PSSPI)
		while (XIsf_TransferInProgress != 0){
		/*NOP*/
		}

#endif
	} else {
		Status = InstancePtr->XIsf_Iface_PolledTransfer(
				InstancePtr->SpiInstPtr,
					WritePtr, ReadPtr, ByteCount);
	}

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
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
*		- XST_FAILURE if device information doesn't match with Atmel
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
		 * For Atmel Serial Flash the device code is the 3rd byte of
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
		if (Status != (int)(XST_SUCCESS)) {
			return (int)(XST_FAILURE);
		}

		/*
		 * Wait until the transfer is complete.
		 */
		do {
#ifndef XPAR_XISF_INTERFACE_PSQSPI
			Status = InstancePtr->XIsf_Iface_SetSlaveSelect(
					InstancePtr->SpiInstPtr,
						InstancePtr->SpiSlaveSelect);
#else
			Status = InstancePtr->XIsf_Iface_SetSlaveSelect(
					InstancePtr->SpiInstPtr);
#endif
		} while(Status == XST_DEVICE_BUSY);

		if (Status != (int)(XST_SUCCESS)) {
			return (int)(XST_FAILURE);
		}

		StatusRegister = BufferPtr[BYTE2];

		/*
		 * Get the Address mode from the Status Register of Serial
		 * Flash.
		 */
		InstancePtr->AddrMode = StatusRegister &
					XISF_SR_ADDR_MODE_MASK;

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
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
}
#endif /* (XPAR_XISF_FLASH_FAMILY == ATMEL) */

#if (((XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))
/*****************************************************************************/
/**
*
* This function enters the Micron flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	- XST_SUCCESS
*		    - XST_FAILURE
*
* @note		Applicable only for Micron flash devices
*
******************************************************************************/
int XIsf_MicronFlashEnter4BAddMode(XIsf *InstancePtr)
{
	int Status;
	u8* NULLPtr = NULL;
	u8 Mode;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->FourByteAddrMode == TRUE) {
		return (int)(XST_SUCCESS);
	}

	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_ENTER_4BYTE_ADDR_MODE;

	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
			NULLPtr, XISF_CMD_4BYTE_ADDR_ENTER_EXIT_BYTES);

	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
			InstancePtr->StatusHandler(InstancePtr,
				XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	InstancePtr->FourByteAddrMode = TRUE;
	return Status;
}

/*****************************************************************************/
/**
*
* This function exits the Micron flash device from 4 bytes addressing mode.
* As per the Micron spec, before issuing this command a write enable command is
* first issued.
*
* @param	InstancePtr is a pointer to the XIsf instance.
*
* @return	- XST_SUCCESS
*		    - XST_FAILURE
*
* @note		Applicable only for Micron flash devices
*
******************************************************************************/
int XIsf_MicronFlashExit4BAddMode(XIsf *InstancePtr)
{
	int Status;
	u8* NULLPtr = NULL;
	u8 Mode;

	if (InstancePtr == NULL) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->IsReady != TRUE) {
		return (int)(XST_FAILURE);
	}

	if (InstancePtr->FourByteAddrMode == FALSE) {
		return (int)(XST_SUCCESS);
	}

	InstancePtr->WriteBufPtr[BYTE1] = XISF_CMD_EXIT_4BYTE_ADDR_MODE;

	Status = XIsf_Transfer(InstancePtr, InstancePtr->WriteBufPtr,
			NULLPtr, XISF_CMD_4BYTE_ADDR_ENTER_EXIT_BYTES);

	Mode = XIsf_GetTransferMode(InstancePtr);

	if(Mode == XISF_INTERRUPT_MODE){
			InstancePtr->StatusHandler(InstancePtr,
				XIsf_StatusEventInfo, XIsf_ByteCountInfo);
	}

	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}
	InstancePtr->FourByteAddrMode = FALSE;
	return Status;
}

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
*		- XST_FAILURE if device information doesn't match with Intel
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
	 * For STM/Winbond/Spansion Serial Flash the device code is 3rd/4th
	 * byte of the JEDEC info. The Third Byte is Memory Type and the 4th
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
			InstancePtr->ManufacturerID = IntelStmDevices[Index].ManufacturerID;
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
		return (int)(XST_FAILURE);
	}

	return (int)(XST_SUCCESS);
}
#endif /* (((XPAR_XISF_FLASH_FAMILY==INTEL) || (XPAR_XISF_FLASH_FAMILY==STM) \
	|| (XPAR_XISF_FLASH_FAMILY == SST) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	(!defined(XPAR_XISF_INTERFACE_PSQSPI)))*/

#if  (((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	defined(XPAR_XISF_INTERFACE_PSQSPI))
/*****************************************************************************/
/**
*
* This function initializes the instance structure with the device geometry of
* the Spansion/Micron/Winbond Serial Flash if it is an Spansion/Micron/Winbond
* device.
*
* @param	InstancePtr is a pointer to the XIsf instance.
* @param	BufferPtr is a pointer to the memory where the device info of
*		the Serial Flash is present.
*
* @return	- XST_SUCCESS if device information matches the JEDEC
*		  information of Intel or Stm or Winbond Serial Flash.
*		- XST_FAILURE if the device information doesn't match with
*		  Intel or Stm or Winbond Serial Flash.
*
* @note		None
*
******************************************************************************/
static int SpaMicWinFlashInitialize(XIsf *InstancePtr, u8 *BufferPtr)
{
	u32 Index;
	u8 NumOfDevices;
	u8 ManufacturerID;
	unsigned int StartIndex;
	u32 FlashMake;
	u8 * WriteBfrPtr = InstancePtr->WriteBufPtr;
	int Status;

	/*
	 * Read ID in Auto mode.
	 */
	WriteBfrPtr[BYTE1] = READ_ID;
	WriteBfrPtr[BYTE2] = 0x23U;		/* 3 dummy bytes */
	WriteBfrPtr[BYTE3] = 0x08U;
	WriteBfrPtr[BYTE4] = 0x09U;

	Status = XIsf_Transfer(InstancePtr, WriteBfrPtr, BufferPtr,
				RD_ID_SIZE);
	if (Status != (int)(XST_SUCCESS)) {
		return (int)(XST_FAILURE);
	}

	/*
	 * Deduce flash make
	 */
	if(BufferPtr[1] == XISF_MANUFACTURER_ID_MICRON) {
		FlashMake = XISF_MANUFACTURER_ID_MICRON;
		StartIndex = MICRON_INDEX_START;
	}
	else if(BufferPtr[1] == XISF_MANUFACTURER_ID_SPANSION) {
		FlashMake = XISF_MANUFACTURER_ID_SPANSION;
		StartIndex = SPANSION_INDEX_START;
	}
	else if(BufferPtr[1] == XISF_MANUFACTURER_ID_WINBOND) {
		FlashMake = XISF_MANUFACTURER_ID_WINBOND;
		StartIndex = WINBOND_INDEX_START;
	}
	else{
		FlashMake = 0;
		StartIndex = 0;
	}
	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(((FlashMake == XISF_MANUFACTURER_ID_MICRON) ||
		(FlashMake == XISF_MANUFACTURER_ID_SPANSION)||
		(FlashMake == XISF_MANUFACTURER_ID_WINBOND)) &&
		(BufferPtr[3] == XISF_MICRON_ID_BYTE2_128)) {
			switch(InstancePtr->SpiInstPtr->Config.ConnectionMode)
		{
			case XISF_QSPIPS_CONNECTION_MODE_SINGLE:
				XIsf_FCTIndex =
					(u32)FLASH_CFG_TBL_SINGLE_128_SP +
						(u32)StartIndex;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_PARALLEL:
				XIsf_FCTIndex =
					(u32)FLASH_CFG_TBL_PARALLEL_128_SP +
						(u32)StartIndex;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_STACKED:
				XIsf_FCTIndex =
					(u32)FLASH_CFG_TBL_STACKED_128_SP +
						(u32)StartIndex;
				break;
			default:
				XIsf_FCTIndex = 0;
				break;
		}
	}

	/*
	 * 256 and 512Mbit supported only for Micron and Spansion,
	 * not Winbond
	 */
	if(((FlashMake == XISF_MANUFACTURER_ID_MICRON) ||
		(FlashMake == XISF_MANUFACTURER_ID_SPANSION)) &&
		(BufferPtr[3] == XISF_MICRON_ID_BYTE2_256)) {
			switch(InstancePtr->SpiInstPtr->Config.ConnectionMode)
		{
			case XISF_QSPIPS_CONNECTION_MODE_SINGLE:
				XIsf_FCTIndex = FLASH_CFG_TBL_SINGLE_256_SP +
						StartIndex;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_PARALLEL:
				XIsf_FCTIndex = FLASH_CFG_TBL_PARALLEL_256_SP +
						StartIndex;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_STACKED:
				XIsf_FCTIndex = FLASH_CFG_TBL_STACKED_256_SP +
						StartIndex;
				break;
			default:
				XIsf_FCTIndex = 0;
				break;
		}
	}

	if(((FlashMake == XISF_MANUFACTURER_ID_MICRON) ||
		(FlashMake == XISF_MANUFACTURER_ID_SPANSION)) &&
		(BufferPtr[3] == XISF_MICRON_ID_BYTE2_512)) {

			switch(InstancePtr->SpiInstPtr->Config.ConnectionMode)
		{
			case XISF_QSPIPS_CONNECTION_MODE_SINGLE:
				XIsf_FCTIndex = FLASH_CFG_TBL_SINGLE_512_SP +
						StartIndex;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_PARALLEL:
				XIsf_FCTIndex = FLASH_CFG_TBL_PARALLEL_512_SP +
						StartIndex;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_STACKED:
				XIsf_FCTIndex = FLASH_CFG_TBL_STACKED_512_SP +
						StartIndex;
				break;
			default:
				XIsf_FCTIndex = 0;
				break;
		}
	}

	/*
	 * 1Gbit Single connection supported for Spansion.
	 * The ConnectionMode will indicate stacked as this part has 2 SS
	 * The device ID will indicate 512Mbit.
	 * This configuration is handled as the above 512Mbit stacked
	 * configuration.
	 */
	/* 1Gbit single, parallel and stacked supported for Micron */
	if((FlashMake == XISF_MANUFACTURER_ID_MICRON) &&
		(BufferPtr[3] == XISF_MICRON_ID_BYTE2_1G)) {

			switch(InstancePtr->SpiInstPtr->Config.ConnectionMode)
		{
			case XISF_QSPIPS_CONNECTION_MODE_SINGLE:
				XIsf_FCTIndex = FLASH_CFG_TBL_SINGLE_1GB_MC;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_PARALLEL:
				XIsf_FCTIndex = FLASH_CFG_TBL_PARALLEL_1GB_MC;
				break;
			case XISF_QSPIPS_CONNECTION_MODE_STACKED:
				XIsf_FCTIndex = FLASH_CFG_TBL_STACKED_1GB_MC;
				break;
			default:
				XIsf_FCTIndex = 0;
				break;
		}
	}

	/*
	 * Populate the InstancePtr members with the appropriate values
	 * based on the XIsf_FCTIndex
	 */
	InstancePtr->BytesPerPage =
		(u16)(SpaMicWinDevices[XIsf_FCTIndex].PageSize);
	InstancePtr->NumDie = SpaMicWinDevices[XIsf_FCTIndex].NumDie;
	InstancePtr->DeviceIDMemSize =
			SpaMicWinDevices[XIsf_FCTIndex].DeviceIDMemSize;
	InstancePtr->ManufacturerID = FlashMake;
	InstancePtr->SectorSize = SpaMicWinDevices[XIsf_FCTIndex].SectSize;
	InstancePtr->NumSectors = SpaMicWinDevices[XIsf_FCTIndex].NumSect;
	InstancePtr->IsReady = TRUE;

	return (int)(XST_SUCCESS);
}
#endif /*  (((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	   defined(XPAR_XISF_INTERFACE_PSQSPI))*/

/*****************************************************************************/
/**
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
* @param	QspiPtr is a pointer to XIsf_Iface instance to be worked on.
* @param	Address which is to be accessed (for erase, write or read)
*
* @return	RealAddr is the translated address - for single it is unchanged
* 			for stacked, the lower flash size is subtracted
* 			for parallel the address is divided by 2.
*
* @note		None.
*
******************************************************************************/
u32 GetRealAddr(XIsf_Iface *QspiPtr, u32 Address)
{
	u32 LqspiCr;
	u32 RealAddr = {0};
#ifdef XPAR_XISF_INTERFACE_PSQSPI
	switch(QspiPtr->Config.ConnectionMode) {
	case XISF_QSPIPS_CONNECTION_MODE_SINGLE:
		RealAddr = Address;
		break;
	case XISF_QSPIPS_CONNECTION_MODE_STACKED:
		/*
		 * Get the current LQSPI Config reg value
		 */
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);

		/* Select lower or upper Flash based on sector address */
		if(Address &
			SpaMicWinDevices[XIsf_FCTIndex].FlashDeviceSize) {
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Subtract first flash size when accessing second
			 * flash.
			 */
			RealAddr = Address &
			(~SpaMicWinDevices[XIsf_FCTIndex].FlashDeviceSize);

		}
		else{
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
				LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			RealAddr = Address;
		}

		/*
		 * Assert the Flash chip select.
		 */
		(void)XQspiPs_SetSlaveSelect(QspiPtr);
		break;
	case XISF_QSPIPS_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		RealAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case */
	break;

	}
#else
		RealAddr = Address;
#endif
	return(RealAddr);
}

#ifdef XPAR_XISF_INTERFACE_PSQSPI
/*****************************************************************************/
/**
* This functions selects the current bank
*
* @param	InstancePtr is a pointer to the QSPI driver component to use.
* @param	BankSel is the bank to be selected in the flash device(s).
*
* @return	XST_SUCCESS if bank selected, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SendBankSelect(XIsf *InstancePtr, u32 BankSel)
{
	#define EXTADD_REG_WR		0xC5U
	#define EXTADD_REG_RD		0xC8U
	#define BANK_REG_WR		0x17U
	#define BANK_SEL_SIZE		2U	/**< BRWR or EARWR command +
						 * 1 byte bank value */
	u8 WriteBuffer[5] = {0};
	u8* NULLPtr= NULL;
	u8 WriteEnableCmdBuf = { WRITE_ENABLE_CMD };
	u32 FlashMake = InstancePtr->ManufacturerID;
	int Status;
	/*
	 * Bank select commands for Micron and Spansion are different
	 */
	if(FlashMake == XISF_MANUFACTURER_ID_MICRON) {

		Xil_AssertNonvoid(NULLPtr == NULL);

		/*
		 * For Micron command WREN should be sent first
		 * except for some specific feature set
		 */
		Status = XIsf_Transfer(InstancePtr,
				&WriteEnableCmdBuf, NULLPtr,
					(u32)sizeof(WriteEnableCmdBuf));

		if(Status != (int)XST_SUCCESS){
			return (int)XST_FAILURE;
		}

		WriteBuffer[BYTE1] = EXTADD_REG_WR;
		WriteBuffer[BYTE2] = (u8)BankSel;

		Xil_AssertNonvoid(NULLPtr == NULL);

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		Status = XIsf_Transfer(InstancePtr, WriteBuffer, NULLPtr,
				BANK_SEL_SIZE);

		if(Status != (int)XST_SUCCESS){
			return (int)XST_FAILURE;
		}
	}
	if(FlashMake == XISF_MANUFACTURER_ID_SPANSION) {
		WriteBuffer[BYTE1] = BANK_REG_WR;
		WriteBuffer[BYTE2] = (u8)BankSel;

		Xil_AssertNonvoid(NULLPtr == NULL);

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		Status = XIsf_Transfer(InstancePtr, WriteBuffer, NULLPtr,
				BANK_SEL_SIZE);

		if(Status != (int)XST_SUCCESS){
			return (int)XST_FAILURE;
		}
	}

	/* Winbond can be added here */

	return (int)(XST_SUCCESS);
}
#endif

/******************************************************************************
*
* This function is to set the Status Handler when an interrupt is registered
*
* @param	InstancePtr is a pointer to the XIsf Instance.
* @param	QspiInstancePtr is a pointer to the XIsf_Iface instance
*		to be worked on.
* @param	XilIsf_Handler is the status handler for the application.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XIsf_SetStatusHandler(XIsf *InstancePtr, XIsf_Iface *XIfaceInstancePtr,
				XIsf_StatusHandler XilIsf_Handler)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XIfaceInstancePtr != NULL);
	Xil_AssertVoid(XilIsf_Handler != NULL);

	/*
	 * Setup the handler for the QSPI that will be called from the
	 * interrupt context when an QSPI status occurs, specify a pointer to
	 * the QSPI driver instance as the callback reference so the handler
	 * is able to access the instance data
	 */
#ifdef XPAR_XISF_INTERFACE_PSQSPI
	XQspiPs_SetStatusHandler(XIfaceInstancePtr, XIfaceInstancePtr,
			 (XQspiPs_StatusHandler) XIsf_IfaceHandler);
#elif XPAR_XISF_INTERFACE_PSSPI
	XSpiPs_SetStatusHandler(XIfaceInstancePtr, XIfaceInstancePtr,
				 (XSpiPs_StatusHandler) XIsf_IfaceHandler);
#elif XPAR_XISF_INTERFACE_AXISPI
	XSpi_SetStatusHandler(XIfaceInstancePtr, XIfaceInstancePtr,
				 (XSpi_StatusHandler) XIsf_IfaceHandler);
#endif

	InstancePtr->StatusHandler = XilIsf_Handler;
}



/******************************************************************************
*
* This function is the handler which performs processing for the QSPI driver.
* It is called from an interrupt context such that the amount of processing
* performed should be minimized.  It is called when a transfer of QSPI data
* completes or an error occurs.
*
* This handler provides an example of how to handle QSPI interrupts but is
* application specific.
*
* @param	CallBackRef is a reference passed to the handler.
* @param	StatusEvent is the status of the QSPI .
* @param	ByteCount is the number of bytes transferred.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XIsf_IfaceHandler(void *CallBackRef, u32 StatusEvent,
			unsigned int ByteCount)
{

	Xil_AssertVoid(CallBackRef != NULL);

	XIsf_TransferInProgress = FALSE;

	XIsf_StatusEventInfo = StatusEvent;

	XIsf_ByteCountInfo = ByteCount;

}
