/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits,
* goodwill, or any type of  loss or damage suffered as a result of any
* action brought  by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the  possibility
* of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-  safe, or for use
* in any application requiring fail-safe  performance, such as life-support
* or safety devices or  systems, Class III medical devices, nuclear
* facilities,  applications related to the deployment of airbags, or any
* other applications that could lead to death, personal  injury, or severe
* property or environmental damage  (individually and collectively,
* "Critical  Applications"). Customer assumes the sole risk and  liability
* of any use of Xilinx products in Critical  Applications, subject only to
* applicable laws and  regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS  PART
* OF THIS FILE AT ALL TIMES.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps8.h
*
* This file implements a driver to support Arasan NAND controller
* present in Zynq Ultrascale Mp.
*
* <b>Driver Initialization</b>
*
* The function call XNandPs8_CfgInitialize() should be called by the application
* before any other function in the driver. The initialization function takes
* device specific data (like device id, instance id, and base address) and
* initializes the XNandPs8 instance with the device specific data.
*
* <b>Device Geometry</b>
*
* NAND flash device is memory device and it is segmented into areas called
* Logical Unit(s) (LUN) and further in to blocks and pages. A NAND flash device
* can have multiple LUN. LUN is sequential raw of multiple blocks of the same
* size. A block is the smallest erasable unit of data within the Flash array of
* a LUN. The size of each block is based on a power of 2. There is no
* restriction on the number of blocks within the LUN. A block contains a number
* of pages. A page is the smallest addressable unit for read and program
* operations. The arrangement of LUN, blocks, and pages is referred to by this
* module as the part's geometry.
*
* The cells within the part can be programmed from a logic 1 to a logic 0
* and not the other way around. To change a cell back to a logic 1, the
* entire block containing that cell must be erased. When a block is erased
* all bytes contain the value 0xFF. The number of times a block can be
* erased is finite. Eventually the block will wear out and will no longer
* be capable of erasure. As of this writing, the typical flash block can
* be erased 100,000 or more times.
*
* The jobs done by this driver typically are:
*	- 8-bit operational mode
*	- Read, Write, and Erase operation
*
* <b>Write Operation</b>
*
* The write call can be used to write a minimum of one byte and a maximum
* entire flash. If the address offset specified to write is out of flash or if
* the number of bytes specified from the offset exceed flash boundaries
* an error is reported back to the user. The write is blocking in nature in that
* the control is returned back to user only after the write operation is
* completed successfully or an error is reported.
*
* <b>Read Operation</b>
*
* The read call can be used to read a minimum of one byte and maximum of
* entire flash. If the address offset specified to read is out of flash or if
* the number of bytes specified from the offset exceed flash boundaries
* an error is reported back to the user. The read is blocking in nature in that
* the control is returned back to user only after the read operation is
* completed successfully or an error is reported.
*
* <b>Erase Operation</b>
*
* The erase operations are provided to erase a Block in the Flash memory. The
* erase call is blocking in nature in that the control is returned back to user
* only after the erase operation is completed successfully or an error is
* reported.
*
* @note
*
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only. Any needs for dynamic memory management, threads,
* mutual exclusion, virtual memory, cache control, or HW write protection
* management must be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 2.0   sb     11/04/2014  Removed Null checks for Buffer passed
*			   as parameter to Read API's
*			   - XNandPs8_Read()
*			   - XNandPs8_ReadPage
*			   Modified
*			   - XNandPs8_SetFeature()
*			   - XNandPs8_GetFeature()
*			   and made them public.
*			   Removed Failure Return for BCF Error check in
*			   XNandPs8_ReadPage() and added BCH_Error counter
*			   in the instance pointer structure.
* 			   Added XNandPs8_Prepare_Cmd API
*			   Replaced
*			   - XNandPs8_IntrStsEnable
*			   - XNandPs8_IntrStsClear
*			   - XNandPs8_IntrClear
*			   - XNandPs8_SetProgramReg
*			   with XNandPs8_WriteReg call
*			   Modified xnandps8.c file API's with above changes.
* 			   Corrected the program command for Set Feature API.
*			   Modified
*			   - XNandPs8_OnfiReadStatus
*			   - XNandPs8_GetFeature
*			   - XNandPs8_SetFeature
*			   to add support for DDR mode.
*			   Changed Convention for SLC/MLC
*			   SLC --> HAMMING
*			   MLC --> BCH
*			   SlcMlc --> IsBCH
*			   Added support for writing BBT signature and version
*			   in page section by enabling XNANDPS8_BBT_NO_OOB.
*			   Removed extra DMA mode initialization from
*			   the XNandPs8_CfgInitialize API.
*			   Modified
*			   - XNandPs8_SetEccAddrSize
*			   ECC address now is calculated based upon the
*			   size of spare area
* </pre>
*
******************************************************************************/

#ifndef XNANDPS8_H		/* prevent circular inclusions */
#define XNANDPS8_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include <string.h>
#include "xstatus.h"
#include "xil_assert.h"
#include "xnandps8_hw.h"
#include "xnandps8_onfi.h"
#include "xil_cache.h"
/************************** Constant Definitions *****************************/

#define XNANDPS8_DEBUG
#define RTL_3_1_FIX
#define RTL_3_1_DRIVER_WORKAROUND

#define XNANDPS8_MAX_TARGETS		1U	/**< ce_n0, ce_n1 */
#define XNANDPS8_MAX_PKT_SIZE		0x7FFU	/**< Max packet size */
#define XNANDPS8_MAX_PKT_COUNT		0xFFFU	/**< Max packet count */

#define XNANDPS8_PAGE_SIZE_512		512U	/**< 512 bytes page */
#define XNANDPS8_PAGE_SIZE_2K		2048U	/**< 2K bytes page */
#define XNANDPS8_PAGE_SIZE_4K		4096U	/**< 4K bytes page */
#define XNANDPS8_PAGE_SIZE_8K		8192U	/**< 8K bytes page */
#define XNANDPS8_PAGE_SIZE_16K		16384U	/**< 16K bytes page */
#define XNANDPS8_PAGE_SIZE_1K_16BIT	1024U	/**< 16-bit 2K bytes page */
#define XNANDPS8_MAX_PAGE_SIZE		16384U	/**< Max page size supported */

#define XNANDPS8_BUS_WIDTH_8		0U	/**< 8-bit bus width */
#define XNANDPS8_BUS_WIDTH_16		1U	/**< 16-bit bus width */

#define XNANDPS8_HAMMING			0x1U	/**< Hamming Flash */
#define XNANDPS8_BCH				0x2U	/**< BCH Flash */

#define XNANDPS8_MAX_BLOCKS		32768U	/**< Max number of Blocks */
#define XNANDPS8_MAX_SPARE_SIZE		0x800U	/**< Max spare bytes of a NAND
						  flash page of 16K */

#define XNANDPS8_INTR_POLL_TIMEOUT	10000U

#define SDR_CLK				((u16)100U * (u16)1000U * (u16)1000U)
#define NVDDR_CLK_0			((u16)20U * (u16)1000U * (u16)1000U)
#define NVDDR_CLK_1			((u16)33U * (u16)1000U * (u16)1000U)
#define NVDDR_CLK_2			((u16)50U * (u16)1000U * (u16)1000U)
#define NVDDR_CLK_3			((u16)66U * (u16)1000U * (u16)1000U)
#define NVDDR_CLK_4			((u16)83U * (u16)1000U * (u16)1000U)
#define NVDDR_CLK_5			((u16)100U * (u16)1000U * (u16)1000U)
//#define XNANDPS8_BBT_NO_OOB

/**
 * The XNandPs8_Config structure contains configuration information for NAND
 * controller.
 */
typedef struct {
	u16 DeviceId;		/**< Instance ID of NAND flash controller */
	u32 BaseAddress;	/**< Base address of NAND flash controller */
} XNandPs8_Config;

/**
 * The XNandPs8_DataInterface enum contains flash operating mode.
 */
typedef enum {
	SDR = 0U,		/**< Single Data Rate */
	NVDDR			/**< Double Data Rate */
} XNandPs8_DataInterface;

/**
 * XNandPs8_TimingMode enum contains timing modes.
 */
typedef enum {
	SDR0 = 0U,
	SDR1,
	SDR2,
	SDR3,
	SDR4,
	SDR5,
	NVDDR0,
	NVDDR1,
	NVDDR2,
	NVDDR3,
	NVDDR4,
	NVDDR5
} XNandPs8_TimingMode;

/**
 * The XNandPs8_SWMode enum contains the driver operating mode.
 */
typedef enum {
	POLLING = 0,		/**< Polling */
	INTERRUPT		/**< Interrupt */
} XNandPs8_SWMode;

/**
 * The XNandPs8_DmaMode enum contains the controller MDMA mode.
 */
typedef enum {
	PIO = 0,		/**< PIO Mode */
	SDMA,			/**< SDMA Mode */
	MDMA			/**< MDMA Mode */
} XNandPs8_DmaMode;

/**
 * The XNandPs8_EccMode enum contains ECC functionality.
 */
typedef enum {
	NONE = 0,
	HWECC,
	EZNAND,
	ONDIE
} XNandPs8_EccMode;

typedef struct {
	XNandPs8_DataInterface CurDataIntf;
	XNandPs8_DataInterface NewDataIntf;
	XNandPs8_TimingMode NewTimingMode;
	u32 ClockFreq;
	u32 FeatureVal;
} XNandPs8_TimingModeDesc;

/**
 * Bad block table descriptor
 */
typedef struct {
	u32 PageOffset[XNANDPS8_MAX_TARGETS];
				/**< Page offset where BBT resides */
	u32 SigOffset;		/**< Signature offset in Spare area */
	u32 VerOffset;		/**< Offset of BBT version */
	u32 SigLength;		/**< Length of the signature */
	u32 MaxBlocks;		/**< Max blocks to search for BBT */
	char Signature[4];	/**< BBT signature */
	u8 Version[XNANDPS8_MAX_TARGETS];
				/**< BBT version */
	u32 Valid;		/**< BBT descriptor is valid or not */
} XNandPs8_BbtDesc;

/**
 * Bad block pattern
 */
typedef struct {
	u32 Options;		/**< Options to search the bad block pattern */
	u32 Offset;		/**< Offset to search for specified pattern */
	u32 Length;		/**< Number of bytes to check the pattern */
	u8 Pattern[2];		/**< Pattern format to search for */
} XNandPs8_BadBlockPattern;

/**
 * The XNandPs8_Geometry structure contains the ONFI geometry information.
 */
typedef struct {
	/*
	 * Parameter page information
	 */
	u32 BytesPerPage;	/**< Number of bytes per page */
	u16 SpareBytesPerPage;	/**< Number of spare bytes per page */
	u32 PagesPerBlock;	/**< Number of pages per block */
	u32 BlocksPerLun;	/**< Number of blocks per LUN */
	u8 NumLuns;		/**< Number of LUN's */
	u8 RowAddrCycles;	/**< Row address cycles */
	u8 ColAddrCycles;	/**< Column address cycles */
	u8 NumBitsPerCell;	/**< Number of bits per cell (Hamming/BCH) */
	u8 NumBitsECC;		/**< Number of bits ECC correctability */
	u32 EccCodeWordSize;	/**< ECC codeword size */
	/*
	 * Driver specific information
	 */
	u32 BlockSize;		/**< Block size */
	u32 NumTargetPages;	/**< Total number of pages in a Target */
	u32 NumTargetBlocks;	/**< Total number of blocks in a Target */
	u64 TargetSize;		/**< Target size in bytes */
	u8 NumTargets;		/**< Number of targets present */
	u32 NumPages;		/**< Total number of pages */
	u32 NumBlocks;		/**< Total number of blocks */
	u64 DeviceSize;		/**< Total flash size in bytes */
} XNandPs8_Geometry;

/**
 * The XNandPs8_Features structure contains the ONFI features information.
 */
typedef struct {
	u32 BusWidth;
	u32 NvDdr;
	u32 EzNand;
	u32 OnDie;
	u32 ExtPrmPage;
} XNandPs8_Features;

/**
 * The XNandPs8_EccMatrix structure contains ECC features information.
 */
typedef struct {
	u16 PageSize;
	u16 CodeWordSize;
	u8 NumEccBits;
	u8 IsBCH;
	u16 EccAddr;
	u16 EccSize;
} XNandPs8_EccMatrix;

/**
 * The XNandPs8_EccCfg structure contains ECC configuration.
 */
typedef struct {
	u16 EccAddr;
	u16 EccSize;
	u16 CodeWordSize;
	u8 NumEccBits;
	u8 IsBCH;
} XNandPs8_EccCfg;

/**
 * The XNandPs8 structure contains the driver instance data. The user is
 * required to allocate a variable of this type for the NAND controller.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	u32 IsReady;		/**< Device is initialized and ready */
	XNandPs8_Config Config;
	u8 BCH_Error_Status;
	XNandPs8_DataInterface DataInterface;
	XNandPs8_TimingMode TimingMode;
	XNandPs8_SWMode Mode;		/**< Driver operating mode */
	XNandPs8_DmaMode DmaMode;	/**< MDMA mode enabled/disabled */
	XNandPs8_EccMode EccMode;	/**< ECC Mode */
	XNandPs8_EccCfg EccCfg;		/**< ECC configuration */
	XNandPs8_Geometry Geometry;	/**< Flash geometry */
	XNandPs8_Features Features;	/**< ONFI features */
	u8 PartialDataBuf[XNANDPS8_MAX_PAGE_SIZE] __attribute__ ((aligned(64)));
					/**< Partial read/write buffer */
	/* Bad block table definitions */
	XNandPs8_BbtDesc BbtDesc;	/**< Bad block table descriptor */
	XNandPs8_BbtDesc BbtMirrorDesc;	/**< Mirror BBT descriptor */
	XNandPs8_BadBlockPattern BbPattern;	/**< Bad block pattern to
						  search */
	u8 Bbt[XNANDPS8_MAX_BLOCKS >> 2];	/**< Bad block table array */
} XNandPs8;

/******************* Macro Definitions (Inline Functions) *******************/

/*****************************************************************************/
/**
 * This macro sets the bitmask in the register.
 *
 * @param	InstancePtr is a pointer to the XNandPs8 instance of the
 *		controller.
 * @param	RegOffset is the register offset.
 * @param	BitMask is the bitmask.
 *
 * @note	C-style signature:
 *		void XNandPs8_SetBits(XNandPs8 *InstancePtr, u32 RegOffset,
 *							u32 BitMask)
 *
 *****************************************************************************/
#define XNandPs8_SetBits(InstancePtr, RegOffset, BitMask)		\
	XNandPs8_WriteReg((InstancePtr)->Config.BaseAddress,		\
		(RegOffset),						\
	((u32)(XNandPs8_ReadReg((InstancePtr)->Config.BaseAddress,	\
		(RegOffset)) | (BitMask))))

/*****************************************************************************/
/**
 * This macro clears the bitmask in the register.
 *
 * @param	InstancePtr is a pointer to the XNandPs8 instance of the
 *		controller.
 * @param	RegOffset is the register offset.
 * @param	BitMask is the bitmask.
 *
 * @note	C-style signature:
 *		void XNandPs8_ClrBits(XNandPs8 *InstancePtr, u32 RegOffset,
 *							u32 BitMask)
 *
 *****************************************************************************/
#define XNandPs8_ClrBits(InstancePtr, RegOffset, BitMask)		\
	XNandPs8_WriteReg((InstancePtr)->Config.BaseAddress,		\
		(RegOffset),						\
	((u32)(XNandPs8_ReadReg((InstancePtr)->Config.BaseAddress,	\
		(RegOffset)) & ~(BitMask))))

/*****************************************************************************/
/**
 * This macro clears and updates the bitmask in the register.
 *
 * @param	InstancePtr is a pointer to the XNandPs8 instance of the
 *		controller.
 * @param	RegOffset is the register offset.
 * @param	Mask is the bitmask.
 * @param	Value is the register value to write.
 *
 * @note	C-style signature:
 *		void XNandPs8_ReadModifyWrite(XNandPs8 *InstancePtr,
 *					u32 RegOffset, u32 Mask, u32 Val)
 *
 *****************************************************************************/
#define XNandPs8_ReadModifyWrite(InstancePtr, RegOffset, Mask, Value)	\
	XNandPs8_WriteReg((InstancePtr)->Config.BaseAddress,		\
		(RegOffset),						\
	((u32)((u32)(XNandPs8_ReadReg((InstancePtr)->Config.BaseAddress,\
		(u32)(RegOffset)) & (u32)(~(Mask))) | (u32)(Value))))

/*****************************************************************************/
/**
 * This macro enables bitmask in Interrupt Signal Enable register.
 *
 * @param	InstancePtr is a pointer to the XNandPs8 instance of the
 *		controller.
 * @param	Mask is the bitmask.
 *
 * @note	C-style signature:
 *		void XNandPs8_IntrSigEnable(XNandPs8 *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define XNandPs8_IntrSigEnable(InstancePtr, Mask)			\
		XNandPs8_SetBits((InstancePtr),				\
			XNANDPS8_INTR_SIG_EN_OFFSET,			\
			(Mask))

/*****************************************************************************/
/**
 * This macro clears bitmask in Interrupt Signal Enable register.
 *
 * @param	InstancePtr is a pointer to the XNandPs8 instance of the
 *		controller.
 * @param	Mask is the bitmask.
 *
 * @note	C-style signature:
 *		void XNandPs8_IntrSigClear(XNandPs8 *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define XNandPs8_IntrSigClear(InstancePtr, Mask)			\
		XNandPs8_ClrBits((InstancePtr),				\
			XNANDPS8_INTR_SIG_EN_OFFSET,			\
			(Mask))

/*****************************************************************************/
/**
 * This macro enables bitmask in Interrupt Status Enable register.
 *
 * @param	InstancePtr is a pointer to the XNandPs8 instance of the
 *		controller.
 * @param	Mask is the bitmask.
 *
 * @note	C-style signature:
 *		void XNandPs8_IntrStsEnable(XNandPs8 *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define XNandPs8_IntrStsEnable(InstancePtr, Mask)			\
		XNandPs8_SetBits((InstancePtr),				\
			XNANDPS8_INTR_STS_EN_OFFSET,			\
			(Mask))

/*****************************************************************************/
/**
 * This macro checks for the ONFI ID.
 *
 * @param	Buff is the buffer holding ONFI ID
 *
 * @note	none.
 *
 *****************************************************************************/
#define IS_ONFI(Buff)					\
	(Buff[0] == (u8)'O') && (Buff[1] == (u8)'N') &&	\
	(Buff[2] == (u8)'F') && (Buff[3] == (u8)'I')

/************************** Function Prototypes *****************************/

s32 XNandPs8_CfgInitialize(XNandPs8 *InstancePtr, XNandPs8_Config *ConfigPtr,
				u32 EffectiveAddr);

s32 XNandPs8_Erase(XNandPs8 *InstancePtr, u64 Offset, u64 Length);

s32 XNandPs8_Write(XNandPs8 *InstancePtr, u64 Offset, u64 Length,
							u8 *SrcBuf);

s32 XNandPs8_Read(XNandPs8 *InstancePtr, u64 Offset, u64 Length,
							u8 *DestBuf);

s32 XNandPs8_EraseBlock(XNandPs8 *InstancePtr, u32 Target, u32 Block);

s32 XNandPs8_WriteSpareBytes(XNandPs8 *InstancePtr, u32 Page, u8 *Buf);

s32 XNandPs8_ReadSpareBytes(XNandPs8 *InstancePtr, u32 Page, u8 *Buf);

s32 XNandPs8_ChangeTimingMode(XNandPs8 *InstancePtr,
				XNandPs8_DataInterface NewIntf,
				XNandPs8_TimingMode NewMode);

s32 XNandPs8_GetFeature(XNandPs8 *InstancePtr, u32 Target, u8 Feature,
								u8 *Buf);

s32 XNandPs8_SetFeature(XNandPs8 *InstancePtr, u32 Target, u8 Feature,
								u8 *Buf);
void XNandPs8_EnableDmaMode(XNandPs8 *InstancePtr);

void XNandPs8_DisableDmaMode(XNandPs8 *InstancePtr);

void XNandPs8_EnableEccMode(XNandPs8 *InstancePtr);

void XNandPs8_DisableEccMode(XNandPs8 *InstancePtr);

void XNandPs8_Prepare_Cmd(XNandPs8 *InstancePtr, u8 Cmd1, u8 Cmd2, u8 EccState,
			u8 DmaMode, u8 AddrCycles);

/*
 * XNandPs8_LookupConfig in xnandps8_sinit.c
 */
XNandPs8_Config *XNandPs8_LookupConfig(u16 DeviceID);


#ifdef __cplusplus
}
#endif

#endif /* XNANDPS8_H end of protection macro */
