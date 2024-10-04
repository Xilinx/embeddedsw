/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps.h
* @addtogroup nandps Overview
* @{
* @details
*
* This file implements a driver for the NAND flash controller.
*
* <b>Driver Initialization</b>
*
* The function call XNandPs_CfgInitialize() should be called by the application
* before any other function in the driver. The initialization function takes
* device specific data (like device id, instance id, and base address) and
* initializes the XNandPs instance with the device specific data.
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
*	- 8/16 bit operational mode
*	- Read, Write, and Erase operation
*	- Read, Write cache operation
*	- Read, Write Spare area operation
*	- HW Error Check and Correction (ECC)
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
* <b>Page Cache Write Operation</b>
*
* The page cache write call is same as write call except that it uses cache
* commands to write. This enhances the performance. This operation can't be
* performed on OnDie ECC with internal ECC enabled. There is no way to disable
* internal ECC for OnDie ECC flash parts in current driver. This operation
* is tested with Spansion S34ML04G100TFI00 flash. We have to use this operation
* only on the flash parts which supports program page cache command.
*
* <b>Page Cache Read Operation</b>
*
* The page cache read call is same as read call except that it uses cache
* commands to read. This enhances the performance.
* The read cache random command is used since the HW ECC block doesn't
* support commands without address for starting ECC.
* This operation can't be performed on OnDie ECC with internal ECC enabled.
* There is no way to disable internal ECC for OnDie ECC flash parts in current
* driver. This operation is tested with Spansion S34ML04G100TFI00 flash.
* We have to use this operation only on the flash parts which supports
* read page cache command (random).
*
* <b>Write Spare Bytes Operation</b>
*
* This call writes to user specified buffer into spare bytes of a page.
*
* <b>Read Spare Bytes Operation</b>
*
* This call reads spare bytes of a page into user specified buffer.
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
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* 	nm     29/09/2011  Added support for On-Die ECC NAND and Clean NAND
*                          flash parts.
*                          Added user spare buffer pointer to read/write
*                          API's. Added new API's for reading and writing
*                          spare buffer area.
*                          Changes nand_cycles with ONFI timing mode 0.
*                          Modified ONFI parameter page reading to read 3
*                          mandatory pages.
* 1.01a nm     28/02/2012  Added tcl file to generate xparameters.h.
*                          Added support for 8Gb On-Die ECC NAND flash
*                          parts (CR 648463).
*                          Fixed 16-bit issue with ONFI commands like
*                          read, write and read status command.
* 1.02a nm     20/09/2012  Removed setting of set_cycles and set_opmode
*                          register values as it is now done in FSBL using
*                          the PCW generated files. CR#678949.
* 1.03a nm     10/22/2012  Fixed CR# 683787,673348.
* 1.04a nm     04/15/2013  Fixed CR# 704401. Removed warnings when compiled
* 			   with -Wall and -Wextra option in bsp.
*	       04/25/2013  Implemented PR# 699544. Added page cache read
*			   and program support. Added API's XNandPs_ReadCache
*			   and XNandPs_WriteCache for page cache support.
*			   Added XNandPs_Features structure to XNandPs instance
*			   which contains features handled by driver.
*			   Added function prototypes for Page cache read/write
*			   and spare byte read/write API's.
* 2.0   adk    12/10/13    Updated as per the New Tcl API's
* 2.1   kpc    07/24/13    Fixed CR#808770. Update command register twice only
*                          if flash device requires >= four address cycles.
* 2.2   sb     01/31/2015  Use the address cycles defined in onfi parameter
*			   page than hardcoding this value to 5 for read and
*			   write operations.
*       ms     03/17/17    Added readme.txt file in examples folder for doxygen
*                          generation.
*       ms     04/10/17    Modified Comment lines in examples to follow doxygen
*                          rules.
* 2.3   ms     04/18/17    Modified tcl file to add suffix U for all macros
*                          definitions of nandps in xparameters.h
* 2.4   nsk    06/20/18    Fixed BBT offset overflow in XNandPs_WriteBbt()
*			   and XNandPs_ReadBbt().
* 2.6	akm    09/03/20    Updated the Makefile to support parallel make execution.
* 2.7   sg     03/18/21    Added validation check for parameter page.
* 2.8  akm     07/06/23    Update the driver to support for system device-tree flow.
* 2.9  sb      01/03/24    Add FlashBbt array in nandps structure for partial read/write operations
*                          while reading and writing bbt in flash.
* 2.10 akm     10/04/24    Retrieve the 'reg' property value from smcc node 'ranges'
*
* </pre>
*
******************************************************************************/

#ifndef XNANDPS_H		/* prevent circular inclusions */
#define XNANDPS_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <string.h>	/* For memcpy, memset */
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xnandps_hw.h"
/************************** Constant Definitions *****************************/
#define XNANDPS_MAX_TARGETS		1	/**< Max number of targets
						  supported */
#define XNANDPS_MAX_BLOCKS		32768	/**< Max number of Blocks */
#define XNANDPS_MAX_PAGE_SIZE		16384	/**< Max page size of NAND
						  flash */
#define XNANDPS_MAX_SPARE_SIZE		512	/**< Max spare bytes of a NAND
						  flash page */
#define XNANDPS_MAX_LUNS		8	/**< Max number of LUNs */
#define XNANDPS_MAX_PAGES_PER_BLOCK	512 	/**< Max number pages per block */
#define XNANDPS_ECC_BLOCK_SIZE		512	/**< ECC block size */
#define XNANDPS_ECC_BYTES		3	/**< ECC bytes per ECC block */

#define XNANDPS_PAGE_SIZE_512		512	/**< Page size 512 */
#define XNANDPS_PAGE_SIZE_1024		1024	/**< Page size 1024 */
#define XNANDPS_PAGE_SIZE_2048		2048	/**< Page size 2048 */
#define XNANDPS_PAGE_SIZE_4096		4096	/**< Page size 4096 */
#define XNANDPS_PAGE_SIZE_8192		8192	/**< Page size 8192 */

#define XNANDPS_SPARE_SIZE_8		8	/**< Spare bytes size 8 */
#define XNANDPS_SPARE_SIZE_16		16	/**< Spare bytes size 16 */
#define XNANDPS_SPARE_SIZE_32		32	/**< Spare bytes size 32 */
#define XNANDPS_SPARE_SIZE_64		64	/**< Spare bytes size 64 */
#define XNANDPS_SPARE_SIZE_128		128	/**< Spare bytes size 128 */
#define XNANDPS_SPARE_SIZE_256		256	/**< Spare bytes size 256 */

#define XNANDPS_FLASH_WIDTH_8		8	/**< NAND Flash width 8-bit */
#define XNANDPS_FLASH_WIDTH_16		16	/**< NAND Flash width 16-bit */

/* Macros used for framing SMC AXI command phase and Data phase address */
#define XNANDPS_END_CMD_NONE		0	/**< No End command */
#define XNANDPS_END_CMD_INVALID	0	/**< End command invalid */
#define XNANDPS_CMD_PHASE		1	/**< End command in command
						  phase */
#define XNANDPS_DATA_PHASE		2	/**< End command in data
						  phase */

#define XNANDPS_PAGE_NOT_VALID		-1	/**< Page is not valid in
						  command phase */
#define XNANDPS_COLUMN_NOT_VALID	-1	/**< Column is not valid in
						  command phase */

#define XNANDPS_AXI_DATA_WIDTH		4	/**< AXI Data width for last
						  transaction while reading
						  and writing */

/* Bit shift for AXI Command/Data phase address calculation */
#define XNANDPS_START_CMD_SHIFT	3	/**< Start command shift */
#define XNANDPS_END_CMD_SHIFT		11	/**< End command shift */
#define XNANDPS_END_CMD_VALID_SHIFT	20	/**< End command valid shift */
#define XNANDPS_ADDR_CYCLES_SHIFT	21	/**< Address cycles shift */
#define XNANDPS_CHIP_ADDR_SHIFT	24	/**< Chip address shift */
#define XNANDPS_ECC_LAST_SHIFT		10	/**< Ecc last shift */
#define XNANDPS_CLEAR_CS_SHIFT		21	/**< clear chip select shift */
#define XNANDPS_COMMAND_PHASE_MASK	0x00000000	/**< Command
							 phase mask */
#define XNANDPS_DATA_PHASE_MASK	0x00080000	/**< Data phase mask */

/* Macros used for correcting ECC errors */
#define XNANDPS_ECC_CORRECT_BYTE_MASK	0x1FF	/**< ECC error correction byte
						  position mask, bits[11:3] of
						  error code */
#define XNANDPS_ECC_CORRECT_BIT_MASK	0x7	/**< ECC error correction bit
						  position mask, bits[0:2] of
						  error code */

/* Flash memory controller operating parameters */
#define XNANDPS_CLR_CONFIG	\
	((XNANDPS_MEMC_CLR_CONFIG_INT_DISABLE1_MASK)	| \
	 (XNANDPS_MEMC_CLR_CONFIG_INT_CLR1_MASK)	| \
	 (XNANDPS_MEMC_CLR_CONFIG_ECC_INT_DISABLE1_MASK))
/**< Interrupt settings */

#define XNANDPS_ECC_MEMCFG \
	((0x1 << XNANDPS_ECC_MEMCFG_ECC_MODE_SHIFT) | \
	 (0x1 << XNANDPS_ECC_MEMCFG_ECC_READ_END_SHIFT)  | \
	 (0x0 << XNANDPS_ECC_MEMCFG_ECC_JUMP_SHIFT))
/**< ECC memory configuration settings */

#define XNANDPS_ECC_CMD1 \
	((0x80 << XNANDPS_ECC_MEMCOMMAND1_WR_CMD_SHIFT) | \
	 (0x00 << XNANDPS_ECC_MEMCOMMAND1_RD_CMD_SHIFT)  | \
	 (0x30 << XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_SHIFT) | \
	 (0x1 << XNANDPS_ECC_MEMCOMMAND1_RD_CMD_END_VALID_SHIFT))
/**< ECC command 1 settings */

#define XNANDPS_ECC_CMD2	\
	((0x85 << XNANDPS_ECC_MEMCOMMAND2_WR_COL_CHANGE_SHIFT) | \
	 (0x05 << XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_SHIFT)  | \
	 (0xE0 << XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_SHIFT) | \
	 (0x1 << XNANDPS_ECC_MEMCOMMAND2_RD_COL_CHANGE_END_VALID_SHIFT))
/**< ECC command 2 settings */

#define XNANDPS_CLR_CS		(0x1 << XNANDPS_CLEAR_CS_SHIFT)
/**< set Clear chip select */
#define XNANDPS_ECC_LAST	(0x1 << XNANDPS_ECC_LAST_SHIFT)
/**< set Ecc last */

/**************************** Type Definitions *******************************/
/*
 * This enum contains ECC Mode
 */
typedef enum {
	XNANDPS_ECC_NONE = 0,	/**< No ECC */
	XNANDPS_ECC_SW,	/**< Software ECC */
	XNANDPS_ECC_HW,	/**< Hardware controller ECC */
	XNANDPS_ECC_ONDIE	/**< On-Die ECC */
} XNandPs_EccMode;

/**
 * This typedef contains configuration information for the flash device.
 */
typedef struct {
#ifndef SDT
	u16  DeviceId;		/**< Instance ID of device */
#else
	char *Name;
#endif
	u32  SmcBase;		/**< SMC Base address */
	u32  FlashBase;		/**< NAND base address */
	u32  FlashWidth;	/**< Flash width */
} XNandPs_Config;
/**
 * Flash geometry
 */
typedef struct {
	u32 BytesPerPage;       /**< Bytes per page */
	u16 SpareBytesPerPage;   /**< Size of spare area in bytes */
	u32 PagesPerBlock;       /**< Pages per block */
	u32 BlocksPerLun;       /**< Bocks per LUN */
	u8 NumLun;              /**< Total number of LUN */
	u8 FlashWidth;          /**< Data width of flash device */
	u64 NumPages;           /**< Total number of pages in device */
	u64 NumBlocks;          /**< Total number of blocks in device */
	u64 BlockSize;		/**< Size of a block in bytes */
	u64 DeviceSize;         /**< Total device size in bytes */
	u8 RowAddrCycles;	/**< Row address cycles */
	u8 ColAddrCycles;	/**< Column address cycles */
} XNandPs_Geometry;

/**
 * ONFI Features and Optional commands supported
 * See parameter page byte 6-7 and 8-9
 */
typedef struct {
	int ProgramCache;
	int ReadCache;
} XNandPs_Features;

/**
 * Bad block table descriptor
 */
typedef struct {
	u32 PageOffset;		/**< Page offset where BBT resides */
	u32 SigOffset;		/**< Signature offset in Spare area */
	u32 VerOffset;		/**< Offset of BBT version */
	u32 SigLength;		/**< Length of the signature */
	u32 MaxBlocks;		/**< Max blocks to search for BBT */
	char Signature[4];	/**< BBT signature */
	u8 Version;		/**< BBT version */
	u32 Valid;		/**< BBT descriptor is valid or not */
} XNandPs_BbtDesc;

/**
 * Bad block pattern
 */
typedef struct {
	u32 Options;		/**< Options to search the bad block pattern */
	u32 Offset;		/**< Offset to search for specified pattern */
	u32 Length;		/**< Number of bytes to check the pattern */
	u8 Pattern[2];		/**< Pattern format to search for */
} XNandPs_BadBlockPattern;

/**
 * ECC configuration structure.
 * Contains information related to ECC.
 */
typedef struct {
	u32 NumSteps;		/**< Number of ECC steps for the flash page */
	u32 BlockSize;		/**< ECC block size */
	u32 BytesPerBlock;	/**< Number of ECC bytes for a block */
	u32 TotalBytes;		/**< Total number of ECC bytes for Page */
	u32 EccPos[XNANDPS_MAX_SPARE_SIZE];
	/**< ECC position in the spare area */
} XNandPs_EccConfig;

/**
 * The XNandPs driver instance data. The user is required to allocate a
 * variable of this type for every flash device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct XNandPsTag {
	u32 IsReady;			/**< Device is initialized and ready */
	XNandPs_Config Config;		/**< XNandPs_Config of current
					  device */
	XNandPs_Geometry Geometry;	/**< Part geometry */
	XNandPs_Features Features;	/**< Features and Optional commands */
	u32 CommandPhaseAddr;		/**< NAND command phase address */
	u32 DataPhaseAddr;		/**< NAND Data phase address */
	XNandPs_EccConfig EccConfig;	/**< ECC configuration parameters */
	/* Bad block table definitions */
	XNandPs_BbtDesc BbtDesc;	/**< Bad block table descriptor */
	XNandPs_BbtDesc BbtMirrorDesc;	/**< Mirror BBT descriptor */
	XNandPs_BadBlockPattern BbPattern;	/**< Bad block pattern to
						  search */
	u8 Bbt[XNANDPS_MAX_BLOCKS >> 2];	/**< Bad block table array */
	u8 FlashBbt[XNANDPS_MAX_BLOCKS >> 2];	/**< BBT buffer for partial read/writes */
	u8 DataBuf[XNANDPS_MAX_PAGE_SIZE + XNANDPS_MAX_SPARE_SIZE];
	/**< Data buffer for partial read/writes */
	u8 *SpareBufPtr;		/**< Pointer to store spare buffer */
	u8 EccCalc[XNANDPS_MAX_SPARE_SIZE];	/**< Buffer for calculated
						  ECC */
	u8 EccCode[XNANDPS_MAX_SPARE_SIZE];	/**< Buffer for stored ECC */
	XNandPs_EccMode EccMode;		/**< ECC Mode */
	int (*ReadPage) (struct XNandPsTag *InstancePtr, u8 *DstPtr);
	/**< Read Page routine */
	int (*WritePage) (struct XNandPsTag *InstancePtr, u8 *SrcPtr);
	/**< Write Page routine */
} XNandPs;

/**
 * NAND Command format structures
 */
typedef struct {
	int StartCmd;	/**< Start command */
	int EndCmd;	/**< End command */
	u8 AddrCycles;	/**< Number of address cycles */
	u8 EndCmdValid;	/**< End command valid */
} XNandPs_CommandFormat;
/***************** Macros (Inline Functions) Definitions *********************/

/**
 * OneHot is used to check if one and only one bit is set.
 * This Macro returns 1 if the value passed is OneHot.
 */
#define OneHot(Value)	(!((Value) & (Value - 1)))

/************************** Function Prototypes ******************************/

/*
 * Functions in xnandps_sinit.c
 */
#ifndef SDT
XNandPs_Config *XNandPs_LookupConfig(u16 DeviceId);
#else
XNandPs_Config *XNandPs_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * Functions in xnandps.c
 */
/*
 * Initialization, read, write and erase functions.
 */
int XNandPs_CfgInitialize(XNandPs *InstancePtr, XNandPs_Config *ConfigPtr,
			  u32 SmcBaseAddr, u32 FlashBaseAddr);
int XNandPs_Read(XNandPs *InstancePtr, u64 Offset, u32 Bytes, void *DestPtr,
		 u8 *UserSparePtr);
int XNandPs_ReadCache(XNandPs *InstancePtr, u64 Offset, u32 Bytes,
		      void *SrcPtr, u8 *UserSparePtr);
int XNandPs_Write(XNandPs *InstancePtr, u64 Offset, u32 Bytes, void *SrcPtr,
		  u8 *UserSparePtr);
int XNandPs_WriteCache(XNandPs *InstancePtr, u64 Offset, u32 Length,
		       void *SrcPtr, u8 *UserSparePtr);
int XNandPs_ReadSpareBytes(XNandPs *InstancePtr, u32 Page, u8 *Buf);
int XNandPs_WriteSpareBytes(XNandPs *InstancePtr, u32 Page, u8 *Buf);
int XNandPs_EraseBlock(XNandPs *InstancePtr, u32 BlockNum);
#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
