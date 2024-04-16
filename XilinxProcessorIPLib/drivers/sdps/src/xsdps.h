/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps.h
* @addtogroup sdps_api SDPS APIs
* @{
* @details
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.00a hk/sg  10/17/13 Initial release
* 2.0   hk      03/07/14 Version number revised.
* 2.1   hk     04/18/14 Increase sleep for eMMC switch command.
*                       Add sleep for microblaze designs. CR# 781117.
* 2.2   hk     07/28/14 Make changes to enable use of data cache.
* 2.3   sk     09/23/14 Send command for relative card address
*                       when re-initialization is done.CR# 819614.
*						Use XSdPs_Change_ClkFreq API whenever changing
*						clock.CR# 816586.
* 2.4	sk	   12/04/14 Added support for micro SD without
* 						WP/CD. CR# 810655.
*						Checked for DAT Inhibit mask instead of CMD
* 						Inhibit mask in Cmd Transfer API.
*						Added Support for SD Card v1.0
* 2.5 	sg		07/09/15 Added SD 3.0 features
*       kvn     07/15/15 Modified the code according to MISRAC-2012.
* 2.6   sk     10/12/15 Added support for SD card v1.0 CR# 840601.
* 2.7   sk     11/24/15 Considered the slot type befoe checking CD/WP pins.
*       sk     12/10/15 Added support for MMC cards.
*              01/08/16 Added workaround for issue in auto tuning mode
*                       of SDR50, SDR104 and HS200.
*       sk     02/16/16 Corrected the Tuning logic.
*       sk     03/01/16 Removed Bus Width check for eMMC. CR# 938311.
* 2.8   sk     04/20/16 Added new workaround for auto tuning.
*              05/03/16 Standard Speed for SD to 19MHz in ZynqMPSoC. CR#951024
* 3.0   sk     06/09/16 Added support for mkfs to calculate sector count.
*       sk     07/16/16 Added support for UHS modes.
*       sk     07/07/16 Used usleep API for both arm and microblaze.
*       sk     07/16/16 Added Tap delays accordingly to different SD/eMMC
*                       operating modes.
*       sk     08/13/16 Removed sleep.h from xsdps.h as a temporary fix for
*                       CR#956899.
* 3.1   mi     09/07/16 Removed compilation warnings with extra compiler flags.
*       sk     10/13/16 Reduced the delay during power cycle to 1ms as per spec
*       sk     10/19/16 Used emmc_hwreset pin to reset eMMC.
*       sk     11/07/16 Enable Rst_n bit in ext_csd reg if not enabled.
*       sk     11/16/16 Issue DLL reset at 31 iteration to load new zero value.
* 3.2   sk     11/30/16 Modified the voltage switching sequence as per spec.
*       sk     02/01/17 Added HSD and DDR mode support for eMMC.
*       sk     02/01/17 Consider bus width parameter from design for switching
*       vns    02/09/17 Added ARMA53_32 support for ZynqMP CR#968397
*       sk     03/20/17 Add support for EL1 non-secure mode.
* 3.3   mn     05/17/17 Add support for 64bit DMA addressing
* 	mn     08/07/17 Modify driver to support 64-bit DMA in arm64 only
*       mn     08/17/17 Enabled CCI support for A53 by adding cache coherency
*                       information.
*       mn     09/06/17 Resolved compilation errors with IAR toolchain
* 3.6   mn     08/01/18 Add support for using 64Bit DMA with 32-Bit Processor
* 3.7   mn     02/01/19 Add support for idling of SDIO
* 3.8   mn     04/12/19 Modified TapDelay code for supporting ZynqMP and Versal
*       mn     09/17/19 Modified ADMA handling API for 32bit and 64bit addresses
* 3.9   mn     03/03/20 Restructured the code for more readability and modularity
*       mn     03/16/20 Move XSdPs_Select_Card API to User APIs
* 3.10  mn     06/05/20 Check Transfer completion separately from XSdPs_Read and
*                       XSdPs_Write APIs
*       mn     06/05/20 Modified code for SD Non-Blocking Read support
* 3.11  sk     12/01/20 Tap programming sequence updates like disable OTAPEN
*                       always, write zero to tap register for zero tap value.
*       sk     12/07/20 Fix eMMC DDR52 mode write/read issue.
*       sk     12/17/20 Removed checking platform specific SD macros and used
*                       Baseaddress instead.
* 3.12  sk     01/28/21 Added support for non-blocking write.
*       sk     02/12/21 Fix the issue in reading CID and CSD.
*       sk     04/08/21 Fixed doxygen warnings in all source files.
*       sk     05/25/21 Fix the compilation issue in Cortex-A72 + EL1_NS by
*                       removing the DLL reset logic (Dead code for Versal).
* 3.13  sk     08/10/21 Limit the SD operating frequency to 19MHz for Versal.
* 3.14  sk     10/22/21 Add support for Erase feature.
*       sk     11/29/21 Fix compilation warnings reported with "-Wundef" flag.
*       sk     01/10/22 Add support to read slot_type parameter.
* 4.0   sk     02/25/22 Add support for eMMC5.1.
*       sk     04/07/22 Add support to read custom tap delay values from design
*                       for SD/eMMC.
*       sk     06/03/22 Fix issue in internal clock divider calculation logic.
* 4.1   sk     11/10/22 Add SD/eMMC Tap delay support for Versal Net.
* 4.1   sa     01/03/23 Report error if Transfer size is greater than 2MB.
* 4.1	sa     12/19/22 Enable eMMC HS400 mode for Versal Net.
* 	sa     01/25/23	Use instance structure to store DMA descriptor tables.
* 4.2   ro     06/12/23 Added support for system device-tree flow.
* 4.2   ap     08/09/23 Reordered XSdPs_FrameCmd XSdPs_Identify_UhsMode functions
* 4.3   ap     10/11/23 Resolved compilation errors with Microblaze RISC-V
* 4.3   ap     11/29/23 Add support for Sanitize feature.
* 4.3   ap     12/22/23 Add support to read custom HS400 tap delay value from design for eMMC.
*
* </pre>
*
******************************************************************************/


#ifndef SDPS_H_
#define SDPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_printf.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "xsdps_hw.h"
#include "xplatform_info.h"
#include "sleep.h"
#include <string.h>
#if defined  (XCLOCKING)
#include "xil_clocking.h"
#endif

/************************** Constant Definitions *****************************/

#define XSDPS_CT_ERROR	0x2L	/**< Command timeout flag */
#define MAX_TUNING_COUNT	40U		/**< Maximum Tuning count */
#define MAX_TIMEOUT		0x1FFFFFFFU		/**< Maximum Timeout */
#define XSDPS_CMD8_VOL_PATTERN	0x1AAU		/**< CMD8 voltage pattern */
#define XSDPS_RESPOCR_READY	0x80000000U	/**< Ready response */
#define XSDPS_ACMD41_HCS	0x40000000U	/**< High Capacity Support */
#define XSDPS_ACMD41_3V3	0x00300000U	/**< 3.3 voltage support */
#define XSDPS_CMD1_HIGH_VOL	0x00FF8000U	/**< CMD1 for High voltage */
#define XSDPS_CMD1_DUAL_VOL	0x00FF8010U	/**< CMD1 for Dual voltage */
#define HIGH_SPEED_SUPPORT	0x2U		/**< High Speed support */
#define UHS_SDR12_SUPPORT	0x1U		/**< SDR12 support */
#define UHS_SDR25_SUPPORT	0x2U		/**< SDR25 support */
#define UHS_SDR50_SUPPORT	0x4U		/**< SDR50 support */
#define UHS_SDR104_SUPPORT	0x8U		/**< SDR104 support */
#define UHS_DDR50_SUPPORT	0x10U		/**< DDR50 support */
#define WIDTH_4_BIT_SUPPORT	0x4U		/**< 4-bit width support */
#define SD_CLK_25_MHZ		25000000U	/**< 25MHz clock */
#define SD_CLK_19_MHZ		19000000U	/**< 19MHz clock */
#define SD_CLK_26_MHZ		26000000U	/**< 26MHz clock */
#define EXT_CSD_DEVICE_TYPE_BYTE	196U	/**< CSD Device Type byte number */
#define EXT_CSD_SEC_COUNT_BYTE1		212U	/**< CSD Sector count byte 1 */
#define EXT_CSD_SEC_COUNT_BYTE2		213U	/**< CSD Sector count byte 2 */
#define EXT_CSD_SEC_COUNT_BYTE3		214U	/**< CSD Sector count byte 3 */
#define EXT_CSD_SEC_COUNT_BYTE4		215U	/**< CSD Sector count byte 4 */
#define EXT_CSD_DEVICE_TYPE_HIGH_SPEED			0x2U	/**< CSD Device type HS */
#define EXT_CSD_DEVICE_TYPE_DDR_1V8_HIGH_SPEED	0x4U	/**< CSD Dev type DDR 1.8v speed */
#define EXT_CSD_DEVICE_TYPE_DDR_1V2_HIGH_SPEED	0x8U	/**< CSD Dev type DDR 1.2v speed */
#define EXT_CSD_DEVICE_TYPE_SDR_1V8_HS200		0x10U	/**< CSD SDR 1.8v HS200 */
#define EXT_CSD_DEVICE_TYPE_SDR_1V2_HS200		0x20U	/**< CSD SDR 1.2v HS200 */
#define EXT_CSD_DEVICE_TYPE_DDR_1V8_HS400		0x40U	/**< CSD SDR 1.8v HS400 */
#define EXT_CSD_DEVICE_TYPE_DDR_1V2_HS400		0x80U	/**< CSD SDR 1.2v HS400 */
#define CSD_SPEC_VER_3		0x3U		/**< CSD card spec ver 3 */
#define SCR_SPEC_VER_3		0x80U		/**< SCR spec ver 3 */
#define ADDRESS_BEYOND_32BIT	0x100000000U	/**< Macro used for beyond 32-bit addr */

#define XSDPS_ZYNQMP_SD0_BASE		0xFF160000U	/**< ZynqMP SD0 Baseaddress */
#define XSDPS_ZYNQMP_SD1_BASE		0xFF170000U	/**< ZynqMP SD1 Baseaddress */
#define XSDPS_VERSAL_SD0_BASE		0xF1040000U	/**< Versal SD0 Baseaddress */
#define XSDPS_VERSAL_SD1_BASE		0xF1050000U	/**< Versal SD1 Baseaddress */

/** @name Block size mask for 512 bytes
 *
 * Block size mask for 512 bytes - This is the default block size.
 * @{
 */

#define XSDPS_BLK_SIZE_512_MASK	0x200U	/**< Blk Size 512 */

/** @} */

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;			/**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;		/**< Base address of the device */
	u32 InputClockHz;		/**< Input clock frequency */
	u32 CardDetect;			/**< Card Detect */
	u32 WriteProtect;			/**< Write Protect */
	u32 BusWidth;			/**< Bus Width */
	u32 BankNumber;			/**< MIO Bank selection for SD */
	u32 HasEMIO;			/**< If SD is connected to EMIO */
	u8 SlotType;			/**< Slot type */
	u8 IsCacheCoherent; 		/**< If SD is Cache Coherent or not */
#if defined  (XCLOCKING) || defined (SDT)
	u32 RefClk;			/**< Input clocks */
#endif
	u32 ITapDly_SDR_Clk50;	/**< Input Tap delay for HSD/SDR25 modes */
	u32 OTapDly_SDR_Clk50;	/**< Output Tap delay for HSD/SDR25 modes */
	u32 ITapDly_DDR_Clk50;	/**< Input Tap delay for DDR50 modes */
	u32 OTapDly_DDR_Clk50;	/**< Output Tap delay for DDR50 modes */
	u32 OTapDly_SDR_Clk100;	/**< Input Tap delay for SDR50 modes */
	u32 OTapDly_SDR_Clk200;	/**< Input Tap delay for SDR104/HS200 modes */
	u32 OTapDly_DDR_Clk200;	/**< Input Tap delay for HS400 modes */
} XSdPs_Config;

/**
 * ADMA2 32-Bit descriptor table
 */
typedef struct {
	u16 Attribute;		/**< Attributes of descriptor */
	u16 Length;		/**< Length of current dma transfer */
	u32 Address;		/**< Address of current dma transfer */
#ifdef __ICCARM__
#pragma data_alignment = 32
} XSdPs_Adma2Descriptor32;
#else
}
__attribute__((__packed__))XSdPs_Adma2Descriptor32;
#endif

/**
 * ADMA2 64-Bit descriptor table
 */
typedef struct {
	u16 Attribute;		/**< Attributes of descriptor */
	u16 Length;		/**< Length of current dma transfer */
	u64 Address;		/**< Address of current dma transfer */
#ifdef __ICCARM__
#pragma data_alignment = 32
} XSdPs_Adma2Descriptor64;
#else
}  __attribute__((__packed__))XSdPs_Adma2Descriptor64;
#endif

/**
 * The XSdPs driver instance data. The user is required to allocate a
 * variable of this type for every SD device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XSdPs_Config Config;	/**< Configuration structure */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 Host_Caps;		/**< Capabilities of host controller */
	u32 Host_CapsExt;	/**< Extended Capabilities */
	u32 HCS;		/**< High capacity support in card */
	u8  CardType;		/**< Type of card - SD/MMC/eMMC */
	u8  Card_Version;	/**< Card version */
	u8  HC_Version;		/**< Host controller version */
	u8  BusWidth;		/**< Current operating bus width */
	u32 BusSpeed;		/**< Current operating bus speed */
	u8  Switch1v8;		/**< 1.8V Switch support */
	u32 CardID[4];		/**< Card ID Register */
	u32 RelCardAddr;	/**< Relative Card Address */
	u32 CardSpecData[4];	/**< Card Specific Data Register */
	u32 SectorCount;		/**< Sector Count */
	u32 SdCardConfig;	/**< Sd Card Configuration Register */
	u32 Mode;			/**< Bus Speed Mode */
	u32 OTapDelay;		/**< Output Tap Delay */
	u32 ITapDelay;		/**< Input Tap Delay */
	u64 Dma64BitAddr;	/**< 64 Bit DMA Address */
	u16 TransferMode;	/**< Transfer Mode */
	u32 SlcrBaseAddr;	/**< SLCR base address*/
	u8  IsBusy;			/**< Busy Flag*/
	u32 BlkSize;		/**< Block Size*/
	u8  IsTuningDone;	/**< Flag to indicate HS200 tuning complete */
#ifdef __ICCARM__
#pragma data_alignment = 32
	XSdPs_Adma2Descriptor32 Adma2_DescrTbl32[32];		/**< ADMA descriptor table 32 Bit */
	XSdPs_Adma2Descriptor64 Adma2_DescrTbl64[32];		/**< ADMA descriptor table 64 Bit */
#else
	XSdPs_Adma2Descriptor32 Adma2_DescrTbl32[32] __attribute__ ((aligned(32)));	/**< ADMA descriptor table 32 Bit */
	XSdPs_Adma2Descriptor64 Adma2_DescrTbl64[32] __attribute__ ((aligned(32)));	/**< ADMA descriptor table 64 Bit */
#endif
} XSdPs;

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name SD High Speed mode configuration options
 * @{
 */
/**
 * User configuration option to enable or disable SD HS mode.
 * By default SD HS mode is disabled for Versal and enabled for
 * other platforms.
 */
#ifdef versal
#define SD_HS_MODE_ENABLE	0
#else
#define SD_HS_MODE_ENABLE	1
#endif
/** @} */

/**
 * Enable eMMC HS400 mode for Versal Net platform
 */
#define ENABLE_HS400_MODE

/************************** Variable Definitions *****************************/
/**
 * XSdPs Configuration Table
 */
#ifndef SDT
extern XSdPs_Config XSdPs_ConfigTable[XPAR_XSDPS_NUM_INSTANCES];
#else
extern XSdPs_Config XSdPs_ConfigTable[];
#endif

/************************** Function Prototypes ******************************/
#ifndef SDT
XSdPs_Config *XSdPs_LookupConfig(u16 DeviceId);
#else
XSdPs_Config *XSdPs_LookupConfig(u32 BaseAddress);
#endif
s32 XSdPs_CfgInitialize(XSdPs *InstancePtr, XSdPs_Config *ConfigPtr,
			UINTPTR EffectiveAddr);
s32 XSdPs_CardInitialize(XSdPs *InstancePtr);
s32 XSdPs_ReadPolled(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, u8 *Buff);
s32 XSdPs_WritePolled(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, const u8 *Buff);
s32 XSdPs_Idle(XSdPs *InstancePtr);

s32 XSdPs_Change_BusSpeed(XSdPs *InstancePtr);
s32 XSdPs_Change_ClkFreq(XSdPs *InstancePtr, u32 SelFreq);
s32 XSdPs_Pullup(XSdPs *InstancePtr);
s32 XSdPs_Get_BusWidth(XSdPs *InstancePtr, u8 *ReadBuff);
s32 XSdPs_Change_BusWidth(XSdPs *InstancePtr);
s32 XSdPs_Get_BusSpeed(XSdPs *InstancePtr, u8 *ReadBuff);
s32 XSdPs_Get_Mmc_ExtCsd(XSdPs *InstancePtr, u8 *ReadBuff);
s32 XSdPs_Set_Mmc_ExtCsd(XSdPs *InstancePtr, u32 Arg);
s32 XSdPs_SetBlkSize(XSdPs *InstancePtr, u16 BlkSize);
s32 XSdPs_Get_Status(XSdPs *InstancePtr, u8 *SdStatReg);
s32 XSdPs_Select_Card(XSdPs *InstancePtr);
s32 XSdPs_StartReadTransfer(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, u8 *Buff);
s32 XSdPs_CheckReadTransfer(XSdPs *InstancePtr);
s32 XSdPs_StartWriteTransfer(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, u8 *Buff);
s32 XSdPs_CheckWriteTransfer(XSdPs *InstancePtr);
s32 XSdPs_Erase(XSdPs *InstancePtr, u32 StartAddr, u32 EndAddr);
s32 XSdPs_Sanitize(XSdPs *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* SD_H_ */
/** @} */
