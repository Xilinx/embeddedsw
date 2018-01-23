/******************************************************************************
*
* Copyright (C) 2009 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xnandps_onfi.h
* @addtogroup nandps_v2_3
* @{
*
* This file implements ONFI specific commands which are used to get the
* parameter page information.
*
* The following commands are supported currently.
* 	- Reset
* 	- Read ID
* 	- READ Parameter Page
* 	- Read Status
* 	- Change Read Column
* 	- Get Features
* 	- Set Features
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* 1.04a nm     04/25/2013  Implemented PR# 699544. Added page cache read
*			   and program commands to ONFI command list.
* </pre>
*
******************************************************************************/
#ifndef ONFI_H		/* prevent circular inclusions */
#define ONFI_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnandps.h"
/************************** Constant Definitions *****************************/
/*
 * Standard ONFI NAND flash commands
 */

/*
 * Mandatory commands
 */
#define ONFI_CMD_READ1				0x00	/**< ONFI Read command
							  (1st cycle) */
#define ONFI_CMD_READ2				0x30	/**< ONFI Read command
							  (2nd cycle) */
#define ONFI_CMD_CHANGE_READ_COLUMN1		0x05	/**< ONFI Change Read
							  Column command (1st
							  cycle) */
#define ONFI_CMD_CHANGE_READ_COLUMN2		0xE0	/**< ONFI Change Read
							  Column command (2nd
							  cycle) */
#define ONFI_CMD_BLOCK_ERASE1			0x60	/**< ONFI Block Erase
							  (1st cycle) */
#define ONFI_CMD_BLOCK_ERASE2			0xD0	/**< ONFI Block Erase
							  (2nd cycle) */
#define ONFI_CMD_READ_STATUS			0x70	/**< ONFI Read status
							  command */
#define ONFI_CMD_PAGE_PROG1			0x80	/**< ONFI Page Program
							  command (1st cycle)
							  */
#define ONFI_CMD_PAGE_PROG2			0x10	/**< ONFI Page Program
							  command (2nd cycle)
							  */
#define ONFI_CMD_CHANGE_WRITE_COLUMN		0x85	/**< ONFI Change Write
							  Column command */
#define ONFI_CMD_READ_ID			0x90	/**< ONFI Read ID
							  command */
#define ONFI_CMD_READ_PARAM_PAGE		0xEC	/**< ONFI Read
							  Parameter Page
							  command */
#define ONFI_CMD_RESET				0xFF	/**< ONFI Reset
							  command */
/*
 * Optional commands
 */
#define ONFI_CMD_COPYBACK_READ1			0x00	/**< ONFI Copyback Read
							  command (1st cycle)
							  */
#define ONFI_CMD_COPYBACK_READ2			0x35	/**< ONFI Copyback Read
							  command (2nd cycle)
							  */
#define ONFI_CMD_READ_CACHE_ENHANCED1		0x00	/**< ONFI Read cache
							  enhanced command (1st
							  cycle) */
#define ONFI_CMD_READ_CACHE_ENHANCED2		0x31	/**< ONFI Read cache
							  enhanced command (2nd
							  cycle) */
#define ONFI_CMD_READ_CACHE			0x31	/**< ONFI Read cache
							  command */
#define ONFI_CMD_READ_CACHE_END			0x3F	/**< ONFI Read cache
							  end command */
#define ONFI_CMD_BLOCK_ERASE_INTERLEAVED2	0xD1	/**< ONFI Block Erase
							  interleaved command
							  (2nd cycle) */
#define ONFI_CMD_READ_STATUS_ENHANCED		0x78	/**< ONFI Read Status
							  enhanced command */
#define ONFI_CMD_PAGE_PROGRAM_INTERLEAVED2	0x11	/**< ONFI Page Program
							  interleaved command
							  (2nd cycle) */
#define ONFI_CMD_PAGE_CACHE_PROGRAM1		0x80	/**< ONFI Page cache
							  program (1st cycle)
							  */
#define ONFI_CMD_PAGE_CACHE_PROGRAM2		0x15	/**< ONFI Page cache
							  program (2nd cycle)
							  */
#define ONFI_CMD_COPYBACK_PROGRAM1		0x85	/**< ONFI Copyback
							  program command (1st
							  cycle) */
#define ONFI_CMD_COPYBACK_PROGRAM2		0x10	/**< ONFI Copyback
							  program command (2nd
							  cycle) */
#define ONFI_CMD_COPYBACK_PROGRAM_INTERLEAVED2	0x11	/**< ONFI Copyback
							  program interleaved
							  command (2nd cycle)
							  */
#define ONFI_CMD_READ_UNIQUEID			0xED	/**< ONFI Read Unique
							  ID command */
#define ONFI_CMD_GET_FEATURES			0xEE	/**< ONFI Get features
							  command */
#define ONFI_CMD_SET_FEATURES			0xEF	/**< ONFI Set features
							  command */

/*
 * ONFI Status Register bit offsets
 */
#define ONFI_STATUS_FAIL			0x01	/**< ONFI Status
							Register : FAIL */
#define ONFI_STATUS_FAILC			0x02	/**< ONFI Status
							Register : FAILC */
#define ONFI_STATUS_ARDY			0x20	/**< ONFI Status
							Register : ARDY */
#define ONFI_STATUS_RDY				0x40	/**< ONFI Status
							Register : RDY */
#define ONFI_STATUS_WP				0x80	/**< ONFI Status
							Register : WR */
/*
 * ONFI constants
 */
#define ONFI_ID_LEN		4		/**< ONFI ID Length */
#define ONFI_CRC_INIT		0x4F4E		/**< ONFI CRC16
						  Inititialization constant */
#define ONFI_CRC_POLYNOM	0x8005		/**< ONFI CRC16 polynomial */
#define ONFI_CRC_ORDER		16		/**< ONFI CRC16 order */
#define ONFI_PARAM_PAGE_LEN	256		/**< ONFI Parameter page length
							*/
#define ONFI_CRC_LEN		254		/**< ONFI CRC16 length */
#define ONFI_SIGNATURE_LEN	4		/**< ONFI Signature Length */


/**
 * This enum defines the onfi commands.
 */
enum OnfiCommandsEnum {
	READ=0,			/**< ONFI Read */
	CHANGE_READ_COLUMN,	/**< ONFI Change Read Column */
	BLOCK_ERASE,		/**< ONFI Block Erase */
	READ_STATUS,		/**< ONFI Read Status */
	PAGE_PROGRAM,		/**< ONFI Page Program */
	CHANGE_WRITE_COLUMN,	/**< ONFI Change Write Column */
	READ_ID,		/**< ONFI Read ID */
	READ_PARAM_PAGE,	/**< ONFI Read Parameter Page */
	RESET,			/**< ONFI Reset */
	GET_FEATURES,		/**< ONFI Get Features */
	SET_FEATURES,		/**< ONFI Set Features */
	READ_CACHE_RANDOM,	/**< ONFI Read page cache random */
	READ_CACHE_END_SEQ,	/**< ONFI Read page cache end */
	PAGE_CACHE_PROGRAM	/**< ONFI Program page cache */
};

/**************************** Type Definitions *******************************/
/**
 * ONFI 1.0 support
 */
/*
 * Parameter page structure of ONFI 1.0 specification.
 * Enhanced this sturcture to include ONFI 2.3 information for EZ NAND support.
 */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	/*
	 * Revision information and features block
	 */
	u8 Signature[4];	/**< Parameter page signature */
	u16 Revision;		/**< Revision Number */
	u16 Features;		/**< Features supported */
	u16 OptionalCmds;	/**< Optional commands supported */
	u8 Reserved0[2];	/**< ONFI 2.3: Reserved */
	u16 ExtParamPageLen;	/**< ONFI 2.3: extended parameter page
					length */
	u8 NumOfParamPages;	/**< ONFI 2.3: No of parameter pages */
	u8 Reserved1[17];	/**< Reserved */
	/*
	 * Manufacturer information block
	 */
	u8 DeviceManufacturer[12];	/**< Device manufacturer */
	u8 DeviceModel[20];		/**< Device model */
	u8 JedecManufacturerId;		/**< JEDEC Manufacturer ID */
	u8 DateCode[2];			/**< Date code */
	u8 Reserved2[13];		/**< Reserved */
	/*
	 * Memory organization block
	*/
	u32 BytesPerPage;		/**< Number of data bytes per page */
	u16 SpareBytesPerPage;		/**< Number of spare bytes per page */
	u32 BytesPerPartialPage;	/**< Number of data bytes per partial
					  page */
	u16 SpareBytesPerPartialPage;	/**< Number of spare bytes per partial
					  page */
	u32 PagesPerBlock;		/**< Number of pages per block */
	u32 BlocksPerLun;		/**< Number of blocks per logical unit
					  (LUN) */
	u8 NumLuns;			/**< Number of LUN's */
	u8 AddrCycles;			/**< Number of address cycles */
	u8 BitsPerCell;			/**< Number of bits per cell */
	u16 MaxBadBlocksPerLun;		/**< Bad blocks maximum per LUN */
	u16 BlockEndurance;		/**< Block endurance */
	u8 GuaranteedValidBlock;	/**< Guaranteed valid blocks at
					  beginning of target */
	u16 BlockEnduranceGvb;		/**< Block endurance for guaranteed
					  valid block */
	u8 ProgramsPerPage;		/**< Number of programs per page */
	u8 PartialProgAttr;		/**< Partial programming attributes */
	u8 EccBits;			/**< Number of bits ECC
					  correctability */
	u8 InterleavedAddrBits;		/**< Number of interleaved address
					  bits */
	u8 InterleavedOperation;	/**< Interleaved operation
					  attributes */
	u8 EzNandSupport;		/**< ONFI 2.3: EZ NAND support
						parameters */
	u8 Reserved3[12];		/**< Reserved */
	/*
	 * Electrical parameters block
	*/
	u8 IOPinCapacitance;		/**< I/O pin capacitance */
	u16 TimingMode;			/**< Timing mode support */
	u16 PagecacheTimingMode;	/**< Program cache timing mode */
	u16 TProg;			/**< Maximum page program time */
	u16 TBers;			/**< Maximum block erase time */
	u16 TR;				/**< Maximum page read time */
	u16 TCcs;			/**< Maximum change column setup
					  time */
	u16 SynTimingMode;		/**< ONFI 2.3: Source synchronous
						timing mode support */
	u8 SynFeatures;			/**< ONFI 2.3: Source synchronous
						features */
	u16 ClkInputPinCap;		/**< ONFI 2.3: CLK input pin
						capacitance */
	u16 IOPinCap;			/**< ONFI 2.3: I/O pin capacitance */
	u16 InputPinCap;		/**< ONFI 2.3: Input pin capacitance
						typical */
	u8 InputPinCapMax;		/**< ONFI 2.3: Input pin capacitance
						maximum */
	u8 DrvStrength;			/**< ONFI 2.3: Driver strength
						support */
	u16 TMr;			/**< ONFI 2.3: Maximum multi-plane
						read time */
	u16 TAdl;			/**< ONFI 2.3: Program page register
						clear enhancement value */
	u16 TEr;			/**< ONFI 2.3: Typical page read time
						for EZ NAND */
	u8 Reserved4[6];		/**< Reserved */
	/*
	 * Vendor block
	*/
	u16 VendorRevisionNum;		/**< Vendor specific revision
					  number */
	u8 VendorSpecific[88];		/**< Vendor specific */
	u16 Crc;			/**< Integrity CRC */
#ifdef __ICCARM__
} OnfiNand_Geometry;
#pragma pack(pop)
#else
}__attribute__((packed))OnfiNand_Geometry;
#endif

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
