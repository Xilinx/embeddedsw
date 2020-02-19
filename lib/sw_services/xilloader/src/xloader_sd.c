/******************************************************************************
* Copyright (C) 2017-2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_sd.c
*
* This is the file which contains sd related code for the PMC FW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xloader_sd.h"
#include "xplmi_hw.h"
#include "xloader.h"
#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#include "xplmi_util.h"
#include "xparameters.h"
#include "ff.h"
#include "xloader.h"
#include "xplmi_generic.h"
/************************** Constant Definitions *****************************/
#define XLOADER_BASE_FILE_NAME_LEN_SD_0 	8
#define XLOADER_BASE_FILE_NAME_LEN_SD_1 	11
#define XLOADER_NUM_DIGITS_IN_FILE_NAME 	4
#define XLOADER_SD_DRV_NUM_0				0
#define XLOADER_SD_DRV_NUM_1				1
#define XLOADER_SD_DRV_NUM_4				(4U)
#define XLOADER_LOGICAL_DRV_MASK			(0xF0000000U)
#define XLOADER_LOGICAL_DRV_SHIFT			(28U)

/**
 * PMC_GLOBAL Base Address
 */
#define PMC_GLOBAL_BASEADDR      0XF1110000

#define PMC_GLOBAL_PMC_MULTI_BOOT    ( ( PMC_GLOBAL_BASEADDR ) + 0X00000004 )

#define PMC_GLOBAL_PMC_MULTI_BOOT_VALUE_SHIFT   0
#define PMC_GLOBAL_PMC_MULTI_BOOT_VALUE_WIDTH   32
#define PMC_GLOBAL_PMC_MULTI_BOOT_VALUE_MASK    0XFFFFFFFF

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XLoader_MakeSdFileName(char *SdEmmcFileName,
		u32 MultibootReg);
static u32 XLoader_GetDrvNumSD(u32 DeviceFlags);

/************************** Variable Definitions *****************************/

static FIL fil;		/* File object */
static FATFS fatfs;
static u32 XLoader_IsSDRaw;
static XSdPs SdInstance = {0U,};
/*****************************************************************************/
/**
 * This function creates the Boot image name for file system devices based on
 * the multiboot register.
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
static void XLoader_MakeSdFileName(char *SdEmmcFileName,
		u32 MultibootReg)
{

	u32 Index;
	u32 Value;
	u32 FileNameLen;
	u32 MultiBootNum = MultibootReg;

	if (0x0U == MultiBootNum)
	{
		(void)XPlmi_Strcat(SdEmmcFileName, "BOOT.BIN");
	}
	else
	{
			(void)XPlmi_Strcat(SdEmmcFileName, "BOOT0000.BIN");
			FileNameLen = XLOADER_BASE_FILE_NAME_LEN_SD_1;

		/* Update file name (to BOOTXXXX.BIN) based on Multiboot register value */
		for(Index = FileNameLen - 1U;
				Index >= (FileNameLen - XLOADER_NUM_DIGITS_IN_FILE_NAME);
				Index--)
		{
			Value = MultiBootNum % 10U;
			MultiBootNum = MultiBootNum / 10U;
			SdEmmcFileName[Index] += (char)Value;
			if (MultiBootNum == 0U)
			{
				break;
			}
		}
	}

	XLoader_Printf(DEBUG_INFO, "File name is %s\r\n", SdEmmcFileName);
}

/*****************************************************************************/
/**
 * This function is used to obtain drive number based on design and boot mode
 *
 * @param       DeviceFlags contain boot mode information
 *
 * @return      Drive number (0 or 1)
 *
 *****************************************************************************/
static u32 XLoader_GetDrvNumSD(u32 DeviceFlags)
{
	/*
	 * If design has both SD0 and SD1, select drive number based on bootmode
	 * If design has ONLY SD0 or ONLY SD1, drive number should be "0"
	 */
#ifdef XPAR_XSDPS_1_DEVICE_ID
	if ((DeviceFlags == XLOADER_PDI_SRC_SD0) ||
			(DeviceFlags == XLOADER_PDI_SRC_SD0_RAW))
	{
		DeviceFlags = XLOADER_SD_DRV_NUM_0;
	} else {
		/* For XLOADER_SD1_BOOT_MODE or XLOADER_SD1_LS_BOOT_MODE
			or XLOADER_EMMC_BOOT_MODE or XLOADER_SD1_RAW_BOOT_MODE
			or XLOADER_SD1_LS_RAW_BOOT_MODE or XLOADER_EMMC_RAW_BOOT_MODE */
		if((DeviceFlags == XLOADER_PDI_SRC_SD1)
		|| (DeviceFlags == XLOADER_PDI_SRC_SD1_LS)
		|| (DeviceFlags == XLOADER_PDI_SRC_EMMC))
		{
			DeviceFlags = XLOADER_SD_DRV_NUM_4;
		}
		else
		{
			DeviceFlags = XLOADER_SD_DRV_NUM_1;
		}
	}
#else
	DeviceFlags = XLOADER_SD_DRV_NUM_0;
#endif

	return DeviceFlags;
}

/*****************************************************************************/
/**
 * This function is used to initialize the sd controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SdInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	FRESULT rc;
	char buffer[32]={0};
	char *boot_file = buffer;
	u32 MultiBootOffset;
	u32 PdiSrc = DeviceFlags & XLOADER_PDISRC_FLAGS_MASK;
	u32 DrvNum = XLoader_GetDrvNumSD(PdiSrc);
	XLoader_IsSDRaw = FALSE;

	if((DeviceFlags & XLOADER_SBD_ADDR_SET_MASK) == XLOADER_SBD_ADDR_SET_MASK)
	{
		/** Secondary Boot in FAT filesystem mode */
		MultiBootOffset = (DeviceFlags >> XLOADER_SBD_ADDR_SHIFT);
		DrvNum += (MultiBootOffset & XLOADER_LOGICAL_DRV_MASK)
					>>XLOADER_LOGICAL_DRV_SHIFT;
		MultiBootOffset &= ~(XLOADER_LOGICAL_DRV_MASK);
	}
	else
	{
		/** Primary Boot in FAT filesystem mode */
		MultiBootOffset = Xil_In32(PMC_GLOBAL_PMC_MULTI_BOOT) &
							XLOADER_MULTIBOOT_OFFSET_MASK;
		MultiBootOffset &= XLOADER_MULTIBOOT_OFFSET_MASK;
	}

	/* Set logical drive number */
	/* Register volume work area, initialize device */
	boot_file[0U] = DrvNum + 48U;
	boot_file[1U] = ':';
	boot_file[2U] = '/';
	rc=f_mount(&fatfs, boot_file, 0U);

	XLoader_Printf(DEBUG_INFO,"SD: rc= %.8x\n\r", rc);

	if (rc != FR_OK) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_INIT, rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_INIT\n\r");
		goto END;
	}

	/**
	 * Create boot image name
	 */
	XLoader_MakeSdFileName(boot_file, MultiBootOffset);

	if(boot_file[0]!=0) {
		rc = f_open(&fil, boot_file, (BYTE)FA_READ);
		if (rc!=FR_OK) {
			XLoader_Printf(DEBUG_INFO,
					"SD: Unable to open file %s: %d\n", boot_file, rc);
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_OPEN, rc);
			XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_OPEN\n\r");
			goto END;
		}
	}
	else
	{
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_OPEN, 0x0);
		goto END;
	}

	Status = XLOADER_SUCCESS;
	END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from SD/eMMC to destination
 * address
 *
 * @param SrcAddress is the address of the SD flash where copy should
 * start from
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XLOADER_SUCCESS for successful copy
 * 		- errors as mentioned in xloader_error.h
 *
 *****************************************************************************/
XStatus XLoader_SdCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	XStatus Status = XST_FAILURE;
	FRESULT rc;	 /* Result code */
	(void) Flags;
	UINT br=0U;

	rc = f_lseek(&fil, SrcAddress);
	if (rc != FR_OK) {
		XLoader_Printf(DEBUG_INFO,
				"SD: Unable to seek to %x\n", SrcAddress);
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_LSEEK, rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_LSEEK\n\r");
		goto END;
	}

	rc = f_read(&fil, (void*)DestAddress, Length, &br);

	if (rc != FR_OK) {
		XLoader_Printf(DEBUG_GENERAL,
				"SD: f_read returned %d\r\n", rc);
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SD_F_READ, rc);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_SD_F_READ\n\r");
		goto END;
	}

	Status = XLOADER_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the sd settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SdRelease(void )
{
	(void )f_close(&fil);

	return XLOADER_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to initialize the sd controller and driver. It is only
 * called in raw boot mode.
 *
 * @param	Drive Number
 *
 * @return	Success or error code
 *
 *****************************************************************************/
int XLoader_RawInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	u32 PdiSrc = DeviceFlags & XLOADER_PDISRC_FLAGS_MASK;
	u32 DrvNum = XLoader_GetDrvNumSD(PdiSrc);
	XLoader_IsSDRaw = TRUE;
	XSdPs_Config *SdConfig;
	/*
	 * Initialize the host controller
	 */
	SdConfig = XSdPs_LookupConfig(DrvNum);
	if (NULL == SdConfig)
	{
		XLoader_Printf(DEBUG_GENERAL,"RAW Lookup config failed\r\n");
		Status = XPLMI_UPDATE_STATUS(Status, XLOADER_ERR_SD_LOOKUP);
		goto END;
	}

	Status = XSdPs_CfgInitialize(&SdInstance, SdConfig,
					SdConfig->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		XLoader_Printf(DEBUG_GENERAL,"RAW Config init failed\r\n");
		Status = XPLMI_UPDATE_STATUS(Status, XLOADER_ERR_SD_CFG);
		goto END;
	}

	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS)
	{
		XLoader_Printf(DEBUG_GENERAL,"RAW SD Card init failed\r\n");
		Status = XPLMI_UPDATE_STATUS(Status, XLOADER_ERR_SD_CARD_INIT);
		goto END;
	}

	if(DeviceFlags == XLOADER_PDI_SRC_EMMC_RAW)
	{
		Status = XSdPs_Set_Mmc_ExtCsd(&SdInstance, XSDPS_MMC_PART_CFG_0_ARG);
	}
	else if(DeviceFlags == XLOADER_PDI_SRC_EMMC_RAW_BP1)
	{
		Status = XSdPs_Set_Mmc_ExtCsd(&SdInstance, XSDPS_MMC_PART_CFG_1_ARG);
	}
	else if(DeviceFlags == XLOADER_PDI_SRC_EMMC_RAW_BP2)
	{
		Status = XSdPs_Set_Mmc_ExtCsd(&SdInstance, XSDPS_MMC_PART_CFG_2_ARG);
	}
	else
	{
		/** MISRA-C compliance */
	}
	if (Status != XST_SUCCESS)
	{
	Status = XPLMI_UPDATE_STATUS(Status, XLOADER_ERR_MMC_PART_CONFIG);
        goto END;
	}

	XLoader_Printf(DEBUG_INFO,"Raw init completed\n\r");
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from SD/eMMC to destination
 * address in raw boot mode only.
 *
 * @param SrcAddress is the address of the SD flash where copy should
 * start from
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XLOADER_SUCCESS for successful copy
 * 		- errors as mentioned in xplmi_status.h
 *
 *****************************************************************************/
XStatus XLoader_RawCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	XStatus Status = XST_FAILURE;
	u32 BlockNumber;
	u32 DataOffset;
	u32 RemainingBytes;
	u8 ReadBuffer[1024U];
	u8*  ReadBuffPtr;
	u32 SectorReadLen;
	u32 NoOfSectors;
	u32 Destination = DestAddress;

	(void) Flags;
	RemainingBytes = Length;
	BlockNumber = SrcAddress/512U;
	DataOffset = SrcAddress%512U;
	/**
	 * Setting the Read len for the first sector partial read
	 */
	SectorReadLen =  512U - DataOffset;


	XLoader_Printf(DEBUG_INFO, "SD Raw Reading Src 0x%08x, Dest 0x%0x%08x, "
		       "Length 0x%0x, Flags 0x%0x\r\n",
			SrcAddress, (u32)(DestAddress>>32U), (u32)DestAddress,
		       Length, Flags);

	do
	{
		/**
		 * Make sure last sector data is read properly
		 */
		if (RemainingBytes < SectorReadLen)
		{
			SectorReadLen = RemainingBytes;
		}

		/**
		 * Read to temparory PRAM address if the length is not equal to 512 bytes
		 */
		if(SectorReadLen != 512U)
		{
			ReadBuffPtr = &ReadBuffer[0U];
			NoOfSectors = 1U;
		}
		else
		{
			ReadBuffPtr = (u8 *)Destination;
			NoOfSectors = RemainingBytes/512U;
			if (NoOfSectors > 128U)
			{
				NoOfSectors = 128U;
			}
		}

		Status  = (u32)XSdPs_ReadPolled(&SdInstance,
				(u32)BlockNumber, NoOfSectors, (u8*)ReadBuffPtr);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}

		/**
		 * Copy the temporary read data to actual destination
		 */
		if (SectorReadLen != 512U)
		{
			(void*)XPlmi_MemCpy((void *)Destination,
						   (ReadBuffPtr + DataOffset),
						   SectorReadLen);
		}
		BlockNumber += NoOfSectors;
		Destination += (NoOfSectors * SectorReadLen);
		RemainingBytes -= (NoOfSectors * SectorReadLen);
		SectorReadLen = 512U;
		DataOffset = 0U;
	} while (RemainingBytes > 0U);

END:
	return Status;
}
#endif /* end of XLOADER_SD_0 */
