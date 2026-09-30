/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs Copyright (C)ChaN, 2016  */
/*-----------------------------------------------------------------------*/
/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
******************************************************************************/

/*****************************************************************************/
/**
*
* @file diskio.c
* @brief Implements the glue layer between the file system and the driver.
*
* Description related to the SD driver:
* To use the file system with SD, select xilffs in the Software Development
* Kit (SDK) when creating a board support package (BSP).
* In SDK, set "fs_interface" to 1 to select the SD interface.
* You can currently use this glue layer with only one SD controller enabled.
* To use an embedded MultiMediaCard (eMMC), set "Enable MMC" to 1 in SDK.
* Otherwise, the glue layer uses SD by default.
*
* Description:
* This glue layer initializes the host controller and SD card in
* disk_initialize. If the SD card supports them, it enables 4-bit mode and
* high speed mode. The default block size is 512 bytes. The disk_read and
* disk_write functions read and write files using Advanced DMA version 2
* (ADMA2) in polled mode.
* The file system can read from and write to an SD card that is already
* formatted as FATFS.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a hk   10/17/13 First release
* 2.0   hk   02/12/14 Corrected status check in disk initialize. CR# 772072.
* 2.1   hk   04/16/14 Move check for ExtCSD high speed bit set inside if
*                     condition for high speed support.
*                     Include xil_types.h irrespective of xsdps.h. CR# 797086.
* 2.2   hk   07/28/14 Make changes to enable use of data cache.
* 3.0	sk	 12/04/14 Added support for micro SD without
* 					  WP/CD. CR# 810655.
*					  Make changes for prototypes of disk_read and
*					  disk_write according to latest version.
*			 12/15/14 Modified the code according to MISRAC 2012 Compliant.
*					  Updated the FatFs to R0.10b
*					  Removed alignment for local buffers as CacheInvalidate
*					  will take care of it.
*		sg   03/03/15 Added card detection check logic
*		     04/28/15 Card detection only in case of card detection signal
* 3.1   sk   06/04/15 Added support for SD1.
* 3.2   sk   11/24/15 Considered the slot type before checking the CD/WP pins.
* 3.3   sk   04/01/15 Added one second delay for checking CD pin.
* 3.4   sk   06/09/16 Added support for mkfs.
* 3.8   mj   07/31/17 Added support for RAM based FATfs.
*       mn   12/04/17 Resolve errors in XilFFS for ARMCC compiler
* 3.9   mn   04/18/18 Resolve build warnings for xilffs library
*       mn   07/06/18 Fix Cppcheck and Doxygen warnings
* 4.2   mn   08/16/19 Initialize Status variables with failure values
*       mn   09/25/19 Check if the SD is powered on or not in disk_status()
* 4.3   mn   02/24/20 Remove unused macro defines
*       mn   04/08/20 Set IsReady to '0' before calling XSdPs_CfgInitialize
* 4.5   sk   03/31/21 Maintain discrete global variables for each controller.
* 4.6   sk   07/20/21 Fixed compilation warning in RAM interface.
* 4.8   sk   05/05/22 Replace standard lib functions with Xilinx functions.
* 5.1   ro   06/12/23 Added support for system device-tree flow.
* 5.2   ap   12/05/23 Add SDT check to fix bug in disk_initialize.
*       ap   01/11/24 Fix Doxygen warnings.
*       sk   07/11/24 Add UFS interface support.
* 5.7   vn   09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/
#include "diskio.h"
#include "ff.h"
#include "xil_types.h"
#include "xstatus.h"

#ifdef SDT
#include "xilffs_config.h"
#endif

#ifdef FILE_SYSTEM_INTERFACE_SD
#ifdef XPAR_XSDPS_NUM_INSTANCES
#include "xsdps.h"		/* SD device driver */
#endif
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
#include "xufspsxc.h"
#endif
#endif
#include "sleep.h"
#include "xil_printf.h"
#include "xil_util.h"

#ifdef XPAR_XSDPS_NUM_INSTANCES
#define SD_CD_DELAY		10000U		/**< SD card detection delay. */
#endif

#define XSDPS_NUM_INSTANCES	2		/**< Number of SD instances. */

#define XUFSPSXC_START_INDEX	3	/**< Start index of UFS instances. */

#ifdef FILE_SYSTEM_INTERFACE_RAM
#include "xparameters.h"

static char *dataramfs = NULL; /**< Start address of the RAM file system. */

/** @name RAM file system geometry
 * @{
 */
#define BLOCKSIZE       1U /**< Erase block size, in sectors. */
#define SECTORSIZE      512U /**< Sector size, in bytes. */
#define SECTORCNT       (RAMFS_SIZE / SECTORSIZE) /**< Number of sectors. */
/** @} */
#endif

/*--------------------------------------------------------------------------

	Public Functions

---------------------------------------------------------------------------*/

/*
 * Global variables
 */
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
/** Disk status. */
static DSTATUS Stat[XSDPS_NUM_INSTANCES + 33U] =
{ STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT,
  STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT,
  STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT,
  STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT, STA_NOINIT};
#else
/** Disk status. */
static DSTATUS Stat[XSDPS_NUM_INSTANCES] = {STA_NOINIT, STA_NOINIT};
#endif

/** Number of physical drive slots tracked by Stat[]. */
#define XFFS_NUM_DRIVES		((u32)(sizeof(Stat) / sizeof(Stat[0])))

#ifdef FILE_SYSTEM_INTERFACE_SD
#ifdef XPAR_XSDPS_NUM_INSTANCES
/** SD driver instance of each SD controller. */
static XSdPs SdInstance[XSDPS_NUM_INSTANCES];
/** Base address of each SD controller. */
static UINTPTR BaseAddress[XSDPS_NUM_INSTANCES];
/** Card detect setting of each SD controller. */
static u32 CardDetect[XSDPS_NUM_INSTANCES];
/** Write protect setting of each SD controller. */
static u32 WriteProtect[XSDPS_NUM_INSTANCES];
/** Slot type of each SD controller. */
static u32 SlotType[XSDPS_NUM_INSTANCES];
/** Host controller specification version of each SD controller. */
static u8 HostCntrlrVer[XSDPS_NUM_INSTANCES];
#endif

#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
#define XUFS_BLUN_PDRV	2U /**< Physical drive number of the UFS boot LUN. */

static XUfsPsxc UfsInstance; /**< UFS driver instance. */
#endif
#endif

/*-----------------------------------------------------------------------*/
/* Get Disk Status							*/
/*-----------------------------------------------------------------------*/

/*****************************************************************************/
/**
*
* @brief Gets the status of the disk.
*
* For SD, this function checks whether the card is present.
*
* @param[in]    pdrv Drive number.
*
* @return
*               - 0 if the status is OK.
*               - STA_NOINIT if the drive is not initialized.
*               - STA_NODISK if there is no medium in the drive.
*               - STA_PROTECT if the drive is write protected.
*
* @note         If the card detect signal is not connected, this function
*               cannot check whether the card is present.
*
******************************************************************************/
DSTATUS disk_status (
	BYTE pdrv	/* Drive number (0) */
)
{
	DSTATUS s;

	if ((u32)pdrv >= XFFS_NUM_DRIVES) {
		return (DSTATUS)(STA_NOINIT | STA_NODISK);
	}

	s = Stat[pdrv];
#ifdef FILE_SYSTEM_INTERFACE_SD
#ifdef XPAR_XSDPS_NUM_INSTANCES
	u32 StatusReg;
	u32 DelayCount = 0;
#endif

	if (pdrv < XSDPS_NUM_INSTANCES) {		/* SD/eMMC interface */
#ifdef XPAR_XSDPS_NUM_INSTANCES
		if (SdInstance[pdrv].Config.BaseAddress == (u32)0) {
			XSdPs_Config *SdConfig;

#ifndef SDT
			SdConfig = XSdPs_LookupConfig((u16)pdrv);
#else
			if (pdrv < XPAR_XSDPS_NUM_INSTANCES) {
				SdConfig = XSdPs_LookupConfig(XSdPs_ConfigTable[pdrv].BaseAddress);
			}
			else {
				SdConfig = NULL;
			}
#endif
			if (NULL == SdConfig) {
				s |= STA_NOINIT;
				return s;
			}

			BaseAddress[pdrv] = SdConfig->BaseAddress;
			CardDetect[pdrv] = SdConfig->CardDetect;
			WriteProtect[pdrv] = SdConfig->WriteProtect;

			HostCntrlrVer[pdrv] = (u8)(XSdPs_ReadReg16(BaseAddress[pdrv],
						   XSDPS_HOST_CTRL_VER_OFFSET) & XSDPS_HC_SPEC_VER_MASK);
			if (HostCntrlrVer[pdrv] == XSDPS_HC_SPEC_V3) {
				SlotType[pdrv] = XSdPs_ReadReg(BaseAddress[pdrv],
								   XSDPS_CAPS_OFFSET) & XSDPS_CAPS_SLOT_TYPE_MASK;
			}
			else {
				SlotType[pdrv] = 0;
			}
		}

		/* If SD is not powered up then mark it as not initialized */
		if ((XSdPs_ReadReg8(BaseAddress[pdrv], XSDPS_POWER_CTRL_OFFSET) &
			 XSDPS_PC_BUS_PWR_MASK) == 0U) {
			s |= STA_NOINIT;
		}

		StatusReg = XSdPs_GetPresentStatusReg(BaseAddress[pdrv]);
		if (SlotType[pdrv] != XSDPS_CAPS_EMB_SLOT) {
			if (CardDetect[pdrv]) {
				while ((StatusReg & XSDPS_PSR_CARD_INSRT_MASK) == 0U) {
					if (DelayCount == 500U) {
						s = STA_NODISK | STA_NOINIT;
						goto Label;
					}
					else {
						/* Wait for 10 msec */
						usleep(SD_CD_DELAY);
						DelayCount++;
						StatusReg = XSdPs_GetPresentStatusReg(BaseAddress[pdrv]);
					}
				}
			}
			s &= ~STA_NODISK;
			if (WriteProtect[pdrv]) {
				if ((StatusReg & XSDPS_PSR_WPS_PL_MASK) == 0U) {
					s |= STA_PROTECT;
					goto Label;
				}
			}
			s &= ~STA_PROTECT;
		}
		else {
			s &= ~STA_NODISK & ~STA_PROTECT;
		}


Label:
		Stat[pdrv] = s;
#endif
	} else {		/* UFS interface */
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
		s &= ~STA_NODISK;
		s &= ~STA_PROTECT;

		Stat[pdrv] = s;
#endif
	}
#endif

	return s;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive						 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* @brief Initializes the drive.
*
* For SD, this function initializes the host controller and the card.
* This function also selects additional settings such as bus width,
* speed, and block size.
*
* @param[in]    pdrv Drive number.
*
* @return       Drive status. A value of 0 or STA_PROTECT alone indicates
*               successful initialization. The status is a bitwise OR of
*               the following flags.
*               - STA_NODISK if the disk is not present.
*               - STA_NOINIT if the drive is not initialized.
*               - STA_PROTECT if the drive is write protected.
*
******************************************************************************/
DSTATUS disk_initialize (
	BYTE pdrv	/* Physical drive number (0) */
)
{
	DSTATUS s;
#ifdef FILE_SYSTEM_INTERFACE_SD
	s32 Status = XST_FAILURE;
#ifdef XPAR_XSDPS_NUM_INSTANCES
	XSdPs_Config *SdConfig;
#endif
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
	XUfsPsxc_Config *UfsConfig;
#endif
#endif

	s = disk_status(pdrv);
	if ((s & STA_NODISK) != 0U) {
		return s;
	}

	/* If disk is already initialized */
	if ((s & STA_NOINIT) == 0U) {
		return s;
	}

#ifdef FILE_SYSTEM_INTERFACE_SD
	if (pdrv < XSDPS_NUM_INSTANCES) {
#ifdef XPAR_XSDPS_NUM_INSTANCES
		if (CardDetect[pdrv]) {
			/*
			 * Card detection check
			 * If the HC detects the No Card State, power will be cleared
			 */
			while (!((XSDPS_PSR_CARD_DPL_MASK |
				  XSDPS_PSR_CARD_STABLE_MASK |
				  XSDPS_PSR_CARD_INSRT_MASK) ==
				 (XSdPs_GetPresentStatusReg(BaseAddress[pdrv]) &
				  (XSDPS_PSR_CARD_DPL_MASK |
				   XSDPS_PSR_CARD_STABLE_MASK |
				   XSDPS_PSR_CARD_INSRT_MASK))));
		}

		/*
		 * Initialize the host controller
		 */
#ifndef SDT
		SdConfig = XSdPs_LookupConfig((u16)pdrv);
#else
		if (pdrv < XPAR_XSDPS_NUM_INSTANCES) {
			SdConfig = XSdPs_LookupConfig(XSdPs_ConfigTable[pdrv].BaseAddress);
		} else {
			SdConfig = NULL;
		}
#endif
		if (NULL == SdConfig) {
			s |= STA_NOINIT;
			return s;
		}

		SdInstance[pdrv].IsReady = 0U;

		Status = XSdPs_CfgInitialize(&SdInstance[pdrv], SdConfig,
						 SdConfig->BaseAddress);
		if (Status != XST_SUCCESS) {
			s |= STA_NOINIT;
			return s;
		}

		Status = XSdPs_CardInitialize(&SdInstance[pdrv]);
		if (Status != XST_SUCCESS) {
			s |= STA_NOINIT;
			return s;
		}
#endif
	} else {
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
		UfsConfig = XUfsPsxc_LookupConfig(XUfsPsxc_ConfigTable[0].BaseAddress);
		if (NULL == UfsConfig) {
			s |= STA_NOINIT;
			return s;
		}

		UfsInstance.IsReady = 0U;

		XUfsPsxc_CfgInitialize(&UfsInstance, UfsConfig);

		Status = XUfsPsxc_Initialize(&UfsInstance);
		if (Status != XST_SUCCESS) {
			s |= STA_NOINIT;
			return s;
		}

		if (pdrv == XUFS_BLUN_PDRV)	{		/* B-LUN drive number used by PLM */
			Status = XUfsPsxc_CheckBootReq(&UfsInstance);
			if (Status != XST_SUCCESS) {
				s |= STA_NOINIT;
				return s;
			}
		}

#endif
	}

	/*
	 * Disk is initialized.
	 * Store the same in Stat.
	 */
	s &= (~STA_NOINIT);

	Stat[pdrv] = s;
#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	/* Assign RAMFS address value from xparameters.h */
	dataramfs = (char *)RAMFS_START_ADDR;

	/* Clearing No init Status for RAM */
	s &= (~STA_NOINIT);
	Stat[pdrv] = s;
#endif

	return s;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)							 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* @brief Reads the drive.
*
* For SD, this function reads the SD card using ADMA2 in polled mode.
*
* @param[in]    pdrv Drive number.
* @param[out]   buff Pointer to the data buffer that stores the read data.
* @param[in]    sector Start sector number.
* @param[in]    count Sector count.
*
* @return
*               - RES_OK if the read is successful.
*               - STA_NOINIT if the drive is not initialized.
*               - RES_ERROR if the read is not successful.
*
******************************************************************************/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DSTATUS s;
#ifdef FILE_SYSTEM_INTERFACE_SD
	s32 Status = XST_FAILURE;
	DWORD LocSector = sector;
#endif

	s = disk_status(pdrv);

	if ((s & STA_NOINIT) != 0U) {
		return RES_NOTRDY;
	}
	if (count == 0U) {
		return RES_PARERR;
	}

#ifdef FILE_SYSTEM_INTERFACE_SD
	if (pdrv < XSDPS_NUM_INSTANCES) {
#ifdef XPAR_XSDPS_NUM_INSTANCES
		/* Convert LBA to byte address if needed */
		if ((SdInstance[pdrv].HCS) == 0U) {
			LocSector *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
		}

		Status  = XSdPs_ReadPolled(&SdInstance[pdrv], (u32)LocSector, count, buff);
		if (Status != XST_SUCCESS) {
			return RES_ERROR;
		}
#endif
	} else {
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
		u32 LunId;

		if (pdrv == XUFS_BLUN_PDRV)	/* Well known B-LUN drive number */
			LunId = XUFSPSXC_BLUN_ID;
		else
			LunId = pdrv - XUFSPSXC_START_INDEX;

		Status  = XUfsPsxc_ReadPolled(&UfsInstance, LunId, (u32)LocSector, count, buff);
		if (Status != XST_SUCCESS) {
			return RES_ERROR;
		}
#endif
	}
#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	/* Reject out-of-range access (subtraction form avoids offset wrap) */
	if ((sector >= (LBA_t)SECTORCNT) || ((LBA_t)count > ((LBA_t)SECTORCNT - sector))) {
		return RES_PARERR;
	}

	if (Xil_SMemCpy(buff, count * SECTORSIZE,
			dataramfs + (sector * SECTORSIZE),
			(u32)(RAMFS_SIZE - (sector * SECTORSIZE)),
			count * SECTORSIZE) != (s32)XST_SUCCESS) {
		return RES_ERROR;
	}
#endif

#if !defined(FILE_SYSTEM_INTERFACE_SD) && !defined(FILE_SYSTEM_INTERFACE_RAM)
	(void)buff;
	(void)sector;
#endif

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions						*/
/*-----------------------------------------------------------------------*/

/*****************************************************************************/
/**
*
* @brief Controls device-specific features and miscellaneous functions.
*
* Lists specific features and performs miscellaneous functions on the device.
* For SD, this function controls device-specific features and miscellaneous
* functions other than generic read and write.
*
* @param[in]     pdrv Drive number.
* @param[in]     cmd Command code.
* @param[in,out] buff Pointer to the parameter, which depends on the command
*                code.
*
* @return
*                - RES_OK if the command is successful.
*                - RES_PARERR if the command is invalid.
*                - RES_NOTRDY if the drive is not initialized.
*                - RES_ERROR if an error occurs.
*
******************************************************************************/
DRESULT disk_ioctl (
	BYTE pdrv,				/* Physical drive number (0) */
	BYTE cmd,				/* Control code */
	void *buff				/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_ERROR;

#ifdef FILE_SYSTEM_INTERFACE_SD
	void *LocBuff = buff;
#ifdef XPAR_XSDPS_NUM_INSTANCES
	DWORD *SendBuff = (DWORD *)(void *)buff;
#endif
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
	u32 Status = XST_FAILURE;
	DWORD PowerMode;
#endif

	if ((disk_status(pdrv) & STA_NOINIT) != 0U) {	/* Check if card is in the socket */
		return RES_NOTRDY;
	}

	switch (cmd) {
		case (BYTE)CTRL_SYNC :	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case (BYTE)GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
			if (pdrv < XSDPS_NUM_INSTANCES) {
#ifdef XPAR_XSDPS_NUM_INSTANCES
				(*((DWORD *)(void *)LocBuff)) = (DWORD)SdInstance[pdrv].SectorCount;
#endif
			} else {
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
				u32 LunId;

				if (pdrv == XUFS_BLUN_PDRV) {	/* B-LUN drive number */
					if (UfsInstance.BootLunEn == XUFSPSXC_BLUN_A) {
						LunId = UfsInstance.BLunALunId;
					} else {
						LunId = UfsInstance.BLunBLunId;
					}
				} else {
					LunId = pdrv - XUFSPSXC_START_INDEX;
				}

				if (!XUfsPsxc_IsValidLun(&UfsInstance, LunId) || (UfsInstance.LUNInfo[LunId].BlockSize == 0U)) {
					return RES_ERROR;
				}

				(*((DWORD *)(void *)LocBuff)) = (DWORD)((UfsInstance.LUNInfo[LunId].LUNSize * 1024U * 1024U) / UfsInstance.LUNInfo[LunId].BlockSize);
#endif
			}
			res = RES_OK;
			break;

		case (BYTE)GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			(*((DWORD *)((void *)LocBuff))) = ((DWORD)128);
			res = RES_OK;
			break;

		case (BYTE)CTRL_TRIM :	/* Erase the data */
			if (pdrv < XSDPS_NUM_INSTANCES) {
#ifdef XPAR_XSDPS_NUM_INSTANCES
				if ((SdInstance[pdrv].HCS) == 0U) {
					SendBuff[0] *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
					SendBuff[1] *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
				}
				(void)XSdPs_Erase(&SdInstance[pdrv], SendBuff[0], SendBuff[1]);
#endif
			}
			res = RES_OK;
			break;

		case (BYTE)GET_SECTOR_SIZE : /* Get sector size on the disk (DWORD) */
			if (pdrv < XSDPS_NUM_INSTANCES) {
#ifdef XPAR_XSDPS_NUM_INSTANCES
				(*((WORD*)((void *)buff))) = ((WORD)512U);
#endif
			} else {
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
				u32 LunId;

				if (pdrv == XUFS_BLUN_PDRV) {	/* B-LUN drive number */
					if (UfsInstance.BootLunEn == XUFSPSXC_BLUN_A) {
						LunId = UfsInstance.BLunALunId;
					} else {
						LunId = UfsInstance.BLunBLunId;
					}
				} else {
					LunId = pdrv - XUFSPSXC_START_INDEX;
				}

				if (!XUfsPsxc_IsValidLun(&UfsInstance, LunId) || (UfsInstance.LUNInfo[LunId].BlockSize == 0U)) {
					return RES_ERROR;
				}

				(*((WORD*)((void *)buff))) = ((WORD)UfsInstance.LUNInfo[LunId].BlockSize);
#endif
			}
			res = RES_OK;
			break;

#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
		case (BYTE)XUFSPSXC_SWITCH_BLUN :
			Status = XUfsPsxc_SwitchBootLUN(&UfsInstance);
			if (Status != XST_SUCCESS) {
				res = RES_ERROR;
				break;
			}

			res = RES_OK;
			break;

		case (BYTE)XUFSPSXC_SPEED_CHANGE :
			PowerMode = *((DWORD *)((void *)buff));
			Status = XUfsPsxc_ConfigureSpeedGear(&UfsInstance, PowerMode);
			if (Status != XST_SUCCESS) {
				res = RES_ERROR;
				break;
			}

			res = RES_OK;
			break;
#endif
		default:
			res = RES_PARERR;
			break;
	}
#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	switch (cmd) {
		case (BYTE)CTRL_SYNC:
			res = RES_OK;
			break;
		case (BYTE)GET_BLOCK_SIZE:
			*(DWORD *)buff = BLOCKSIZE;
			res = RES_OK;
			break;
		case (BYTE)GET_SECTOR_SIZE:
			*(WORD *)buff = SECTORSIZE;
			res = RES_OK;
			break;
		case (BYTE)GET_SECTOR_COUNT:
			*(DWORD *)buff = SECTORCNT;
			res = RES_OK;
			break;
		default:
			res = RES_PARERR;
			break;
	}

	(void)pdrv;
#endif

#if !defined(FILE_SYSTEM_INTERFACE_SD) && !defined(FILE_SYSTEM_INTERFACE_RAM)
	(void)pdrv;
	(void)cmd;
	(void)buff;
#endif

	return res;
}

/*****************************************************************************/
/**
*
* @brief Implements the timer function that the application provides to FatFs.
*
* @return       Timestamp packed into a DWORD value.
*
******************************************************************************/
DWORD get_fattime (void)
{
	return	((DWORD)(2010U - 1980U) << 25U)	/* Fixed to Jan. 1, 2010 */
		| ((DWORD)1 << 21)
		| ((DWORD)1 << 16)
		| ((DWORD)0 << 11)
		| ((DWORD)0 << 5)
		| ((DWORD)0 >> 1);
}

/*****************************************************************************/
/**
*
* @brief Writes to the drive.
*
* For SD, this function writes to the SD card using ADMA2 in polled mode.
*
* @param[in]    pdrv Drive number.
* @param[in]    buff Pointer to the data to write.
* @param[in]    sector Sector address.
* @param[in]    count Sector count.
*
* @return
*               - RES_OK if the write is successful.
*               - STA_NOINIT if the drive is not initialized.
*               - RES_ERROR if the write is not successful.
*
******************************************************************************/
#if FF_FS_READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DSTATUS s;
#ifdef FILE_SYSTEM_INTERFACE_SD
	s32 Status = XST_FAILURE;
	DWORD LocSector = sector;
#endif

	s = disk_status(pdrv);
	if ((s & STA_NOINIT) != 0U) {
		return RES_NOTRDY;
	}
	if (count == 0U) {
		return RES_PARERR;
	}

#ifdef FILE_SYSTEM_INTERFACE_SD
	if (pdrv < XSDPS_NUM_INSTANCES) {
#ifdef XPAR_XSDPS_NUM_INSTANCES
		/* Convert LBA to byte address if needed */
		if ((SdInstance[pdrv].HCS) == 0U) {
			LocSector *= (DWORD)XSDPS_BLK_SIZE_512_MASK;
		}

		Status  = XSdPs_WritePolled(&SdInstance[pdrv], (u32)LocSector, count, buff);
		if (Status != XST_SUCCESS) {
			return RES_ERROR;
		}
#endif
	} else {
#ifdef XPAR_XUFSPSXC_NUM_INSTANCES
		u32 LunId;

		if (pdrv == XUFS_BLUN_PDRV) {	/* Well known B-LUN drive number */
			LunId = XUFSPSXC_BLUN_ID;
		} else {
			LunId = pdrv - XUFSPSXC_START_INDEX;
		}

		Status  = XUfsPsxc_WritePolled(&UfsInstance, LunId, (u32)LocSector, count, buff);
		if (Status != XST_SUCCESS) {
			return RES_ERROR;
		}
#endif
	}

#endif

#ifdef FILE_SYSTEM_INTERFACE_RAM
	/* Reject out-of-range access */
	if ((sector >= (LBA_t)SECTORCNT) || ((LBA_t)count > ((LBA_t)SECTORCNT - sector))) {
		return RES_PARERR;
	}

	if (Xil_SMemCpy(dataramfs + (sector * SECTORSIZE),
			(u32)(RAMFS_SIZE - (sector * SECTORSIZE)),
			buff, count * SECTORSIZE,
			count * SECTORSIZE) != (s32)XST_SUCCESS) {
		return RES_ERROR;
	}
#endif

#if !defined(FILE_SYSTEM_INTERFACE_SD) && !defined(FILE_SYSTEM_INTERFACE_RAM)
	(void)buff;
	(void)sector;
#endif

	return RES_OK;
}
#endif
