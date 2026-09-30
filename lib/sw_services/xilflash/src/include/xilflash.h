/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XFLASH_H
#define XFLASH_H

/*****************************************************************************/
/**
*
* @file xilflash.h
* @addtogroup xilflash_apis Library-APIs
* @{
* @brief Defines the XFlash library interface for CFI flash memory devices.
*
* This module implements the functionality for flash memory devices that
* conform to the "Common Flash Interface" (CFI) standard. CFI allows a single
* flash library to be used for an entire family of parts.
*
* This is not a library for a specific device, but for a set of command
* read/write/erase algorithms. CFI allows the library to determine which
* algorithm to use at runtime.
*
* <b>Library Initialization</b>
*
* The application should call XFlash_Initialize() before any other function in
* the library. The initialization function checks for the device family and
* initializes the XFlash instance with the family specific data. It sets up the
* VT table, which contains the function pointers to the family specific APIs,
* and calls the family specific initialization routine.
*
* <b>Device Geometry</b>
*
* The Device geometry varies for different flash device families. The following
* list describes geometry of different flash device families:
*
* 	<b> Intel Flash Device Geometry </b>
*
* 	Flash memory space is segmented into areas called blocks. The size of
* 	each block is based on a power of 2. A region is defined as a contiguous
*	set of blocks of the same size. Some parts have several regions while
*	others have one.
*
*	This module refers to the arrangement of blocks and regions as the
*	part's geometry. Some Intel flash supports multiple banks on the same
*	device. This library supports single and multiple bank flash devices.
*
* 	<b> AMD Flash Device Geometry </b>
*
*	Flash memory space is segmented into areas called banks and further
*	into regions and blocks. The size of each block is based on a power of
*	2. A region is defined as a contiguous set of blocks of the same size.
*	Some parts have several regions while others have one.
*
*	A bank is defined as a contiguous set of blocks. The bank may contain
*	blocks of different size. This module refers to the arrangement of
*	blocks, regions and banks as the part's geometry.
*
* 	The cells within the part can be programmed from a logic 1 to a logic 0
*	and not the other way around. To change a cell back to a logic 1, the
*	entire block containing that cell must be erased. When a block is erased
*	all bytes contain the value 0xFF. The number of times a block can be
*	erased is finite. Eventually the block wears out and is no longer
*	capable of erasure. As of this writing, the typical flash block can
*	be erased 100,000 or more times.
*
* <b>Write Operation</b>
*
* The write call writes a minimum of zero bytes and a maximum of the entire
* flash. An error is reported back to the caller if the Offset Address specified
* to write is out of flash. An error is also reported if the number of bytes
* specified from the Offset address exceeds flash boundaries. The write is
* blocking in nature: control returns to the caller only after the write
* operation completes successfully or an error is reported.
*
* <b>Read Operation</b>
*
* The read call reads a minimum of zero bytes and a maximum of the entire flash.
* If the Offset Address specified to read is out of flash boundary, an error is
* reported back to the caller. The read function reads memory locations beyond
* Flash boundary. Make sure that the Number of Bytes + Offset address is within
* the Flash address boundaries. The read is blocking in nature: control returns
* to the caller only after the read operation completes successfully or an
* error is reported.
*
* <b>Erase Operation</b>
*
* The erase operations erase a Block in the Flash memory. The erase call is
* blocking in nature: control returns to the caller only after the erase
* operation completes successfully or an error is reported.
*
* <b>Sector Protection</b>
*
* The Flash Device is divided into Blocks. Each Block can be protected
* individually from unwarranted writing/erasing.
*
* Use the XFlash_Lock() call to lock blocks. All the memory locations from the
* specified Offset address are locked. Use the XFlash_Unlock() call to unlock
* blocks. All the Blocks which are previously locked are unlocked.
*
* The Lock and Unlock calls are blocking in nature: control returns to the
* caller only after the operation completes successfully or an error is
* reported.
*
* The AMD flash device requires high voltage on the Reset pin to perform lock
* and unlock operations. Provide this high voltage (as defined in the
* datasheet) to the reset pin before calling the lock and unlock APIs for AMD
* flash devices. Lock and Unlock features are not tested for AMD flash devices.
*
* <b>Device Control</b>
*
* Functionalities specific to a Flash Device Family are implemented as Device
* Control.
*
* The following are the Intel specific device control:
*	- Retrieve the last error data.
*	- Get Device geometry.
*	- Get Device properties.
* 	- Set RYBY pin mode.
*	- Set the Configuration register (Platform Flash only).
*
* The following are the AMD specific device control:
*	- Get Device geometry.
*	- Get Device properties.
*	- Erase Resume.
* 	- Erase Suspend.
* 	- Enter Extended Mode.
* 	- Exit Extended Mode.
*	- Get Protection Status of Block Group.
*	- Erase Chip.
*
* @note
* <b>This library needs to know the type of EMC core (AXI or XPS) used to
* access the cfi flash, to map the correct APIs. Use this library with the emc
* driver, v3_01_a and above, so that this information can be automatically
* obtained from the emc driver.</b>
* <br><br>
* This library is intended to be RTOS and processor independent. It works with
* physical addresses only. The layer above this library must satisfy any needs
* for dynamic memory management, threads, mutual exclusion, virtual memory,
* cache control, or HW write protection management.
* <br><br>
* All writes to flash occur in units of bus-width bytes. If more than one part
* exists on the data bus, then the parts are written in parallel. Reads from
* flash are performed in any width up to the width of the data bus. The library
* assumes that the flash bus controller or local bus supports these types of
* accesses.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date      Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/25/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.02a ksu  06/16/09 Added support for multiple banks in Intel flash
*		      Removed unused defines (XFL_MAX_NUM_BANKS, XFL_BANK1,
*		      XFL_BANK2)
*		      Added IOCTL to set configuration register of platform
*		      flash
* 1.03a ksu  10/07/09 Added support for large buffer size flash (CR535564)
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 2.01a ktn  03/31/10 Updated the AMD code to support uniform sector WP modes
* 2.02a sdm  06/30/10 Updated to support AXI EMC with Little Endian Processor
* 2.02a sdm  07/07/10 Updated XFlashAmd_Initialize() to NOT change the erase
*		      region information of a top boot device, when the number
*		      of erase regions is not more than 1.
* 3.00a sdm  03/03/11 Removed static parameters in mld and updated code to
*		      determine these parameters from the CFI data.
* 3.00a sdm  03/23/11 Added two new parameters in mld for flash families. Users
*		      can enable support for either of the flash devices or both
*		      of them.
* 3.01a srt  03/02/12 Added support for Micron G18 Flash device to fix
*		      CRs 648372, 648282.
*		      Modified XFlashIntel_Reset function to reset all the
*		      partitions.
*		      Added DATA_SYNC to fix the CR 644750.
* 3.02a srt  05/30/12 Changed Implementation for Micron G18 Flash, which
*		      fixes the CR 662317.
*		      CR 662317 Description - Xilinx Platform Flash on ML605
*		      fails to work.
* 3.03a srt  11/04/12 Increased AMD maximum erase regions (CR 668730)
* 		      Fixed CR 679937  -
*		      Description: Non-word aligned data write to flash fails
*		      with AXI interface.
* 3.04a srt  02/18/13 Fixed CR 700553.
* 4.1	nsk  06/06/12 Updated WriteBuffer programming for spansion
*		      in xilflash_amd.c and added new definitions
*		      as per AMD Spec in xilflash_amd.h (CR 781697).
* 4.1	nsk  08/06/15 Fixed CR 835008 Modified xilflash_intel.c.
* 4.2   nsk  01/07/15 Add Support to change Flash from Sync to Async Mode.
*                     Updated xilflash_readwrite_example.c file.
*                     Modified FLASH_BASE_ADDRESS in xilflash_readwrite
*                     _example.c to canonical name (CR 808007)
* 4.3   ms   01/17/17 Fixed compilation warnings.
* 4.4   ms   08/03/17 Added doxygen tags.
* 4.5	tjs  08/13/18 Fixed compilation errors for ARMCC (CR#1008306)
* 4.6	akm  01/22/19 Fixed compilation errors (CR#1018603)
* 4.7	akm  07/10/19 Updated XFlashAmd_Write() to use adjusted base address
*                     in write operation(CR-1029074).
* 4.7	akm  07/23/19 Initialized Status variable to XST_FAILURE.
* 4.8	sne  04/23/21 Fixed doxygen warnings.
* 4.10	akm  07/14/23 Added support for system device-tree flow.
* 4.14 vn   08/26/26 Fix Doxygen warnings.
* 4.14  vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** @cond xilflash_internal */
/** @{ */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xilflash_properties.h"
#include "xparameters.h"
#include <string.h>
#ifdef SDT
#include "xilflash_config.h"
#endif


/************************** Constant Definitions *****************************/

/**
 * @name Vendor command set codes
 *
 * Refer to industry document "CFI publication 100" for the latest list.
 * @{
 */
#define XFL_CMDSET_INTEL_STANDARD	3	/**< Includes Micron/Sharp. */
#define XFL_CMDSET_INTEL_EXTENDED	1	/**< Includes Micron/Sharp. */
/** Includes Fujitsu/STM. */
#define XFL_CMDSET_AMD_STANDARD		2
/** Includes Fujitsu/STM. */
#define XFL_CMDSET_AMD_EXTENDED		4
/** Mitsubishi standard command set. */
#define XFL_CMDSET_MITSUBISHI_STANDARD	256
/** Mitsubishi extended command set. */
#define XFL_CMDSET_MITSUBISHI_EXTENDED	257
#define XFL_CMDSET_INTEL_G18		0x200   /**< Micron G18 Flash. */
/** @} */

/**
 * @name Flash return types
 * @{
 */
/** Flash is erasing or programming. */
#define XFLASH_BUSY			101
#define XFLASH_READY			102 /**< Flash is ready for commands. */
/**
 * Flash has detected an internal error. Use XFlash_DeviceControl to retrieve
 * device specific codes.
 */
#define XFLASH_ERROR			103
/** Flash is in suspended erase state. */
#define XFLASH_ERASE_SUSPENDED		104
/** Flash is in suspended write state. */
#define XFLASH_WRITE_SUSPENDED		105
/** Flash type not supported by library. */
#define XFLASH_PART_NOT_SUPPORTED	106
#define XFLASH_NOT_SUPPORTED		107	/**< Operation not supported. */
/** Too many erase regions. */
#define XFLASH_TOO_MANY_REGIONS		108
/** Programming or erase operation aborted due to a timeout. */
#define XFLASH_TIMEOUT_ERROR		109
/** Accessed flash outside its addressable range. */
#define XFLASH_ADDRESS_ERROR 		110
#define XFLASH_ALIGNMENT_ERROR		111	/**< Write alignment error. */
/**
 * Could not return immediately from write/erase function with
 * XFL_NON_BLOCKING_WRITE/ERASE option cleared.
 */
#define XFLASH_BLOCKING_CALL_ERROR 	112
#define XFLASH_CFI_QUERY_ERROR		113 /**< Failed to query the device. */
#define XFLASH_BLOCK_PROTECTED		114	/**< Block is protected. */
/** @} */

/**
 * @name Supported part arrangements
 *
 * This enumeration defines the supported arrangements of parts on the
 * data-bus. The naming convention for these constants is as follows:
 *
 *	XFL_LAYOUT_Xa_Xb_Xc, where
 *
 * Xa is the part's physical data bus width. Xb is the part's selected data
 * bus width (this field is required because a x16 part can be placed in x8
 * mode). Xc is the number of interleaved parts. For example one part can be
 * tied to D0-D15 and a second to data lines D16-D31.
 *
 * Treat parts arranged in series as separate instances. An example of this
 * layout: Two X16 parts operating in X16 mode. The first part occupies
 * address space FF000000 - FF0FFFFF and a second from FF100000 - FF1FFFFF.
 *
 * These constants are encoded using bit masks defined in the next section.
 * @{
 */
/**
 * One 16-bit part operating in 8-bit mode. Total data bus width is 8-bits.
 * This layout is only supported in AMD flash devices.
 */
#define XFL_LAYOUT_X16_X8_X1  0x02020101
/**
 * One 16-bit part operating in 16-bit mode. Total data bus width is 16-bits.
 * This layout is supported in AMD and Intel flash devices.
 */
#define XFL_LAYOUT_X16_X16_X1 0x02020201
/**
 * Two 16-bit parts operating in 16-bit mode. Total data bus width is 32-bits.
 * This layout is only supported in Intel flash devices.
 */
#define XFL_LAYOUT_X16_X16_X2 0x04020202
/**
 * Four 16-bit parts operating in 16-bit mode. Total data bus width is 64-bits.
 * This layout is only supported in Intel flash devices.
 */
#define XFL_LAYOUT_X16_X16_X4 0x08020204
/** @} */

/**
 * @name Device control commands for all families
 * @{
 */
#define XFL_DEVCTL_GET_LAST_ERROR 	1 /**< Retrieve the last error data. */
/** Get Device geometry. */
#define XFL_DEVCTL_GET_GEOMETRY		2
#define XFL_DEVCTL_GET_PROPERTIES	3	/**< Get Device Properties. */
#define XFL_DEVCTL_SET_RYBY		4	/**< Set RYBY pin mode. */
#define XFL_DEVCTL_ERASE_RESUME		5	/**< Resume Erase. */
#define XFL_DEVCTL_ERASE_SUSPEND	6	/**< Suspend Erase. */
#define XFL_DEVCTL_ENTER_EXT_MODE	7	/**< Enter Extended mode. */
#define XFL_DEVCTL_EXIT_EXT_MODE	8	/**< Exit Extended mode. */
#define XFL_DEVCTL_CHIP_ERASE		9	/**< Erase whole chip. */
/** Check block protection status. */
#define XFL_DEVCTL_PROTECTION_STATUS	10
#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#define XFL_DEVCTL_SET_CONFIG_REG	11 /**< Set config register value. */
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */
/** @} */

/**
 * A block region is defined as a set of consecutive erase blocks of the
 * same size. Most flash devices only have a handful of regions. If a
 * part has more regions than defined by this constant, then the constant
 * must be modified to accommodate the part. The minimum value of this
 * constant is 1 and there is no maximum value. Note that increasing this
 * value also increases the amount of memory used by the geometry structure
 * approximately 12 bytes per increment.
 */
#define XFL_INTEL_MAX_ERASE_REGIONS	20

/** Maximum number of erase regions and banks for the AMD family. */
#define XFL_AMD_MAX_ERASE_REGIONS	10

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
/**
 * @name Family specific limits
 * @{
 */
/** Maximum number of erase regions. */
#define XFL_MAX_ERASE_REGIONS		XFL_INTEL_MAX_ERASE_REGIONS
/** Number of 32-bit integers reserved for vendor data. */
#define XFL_MAX_VENDOR_DATA_LENGTH	20
/** @} */
#else
/**
 * @name Family specific limits
 * @{
 */
/** Maximum number of erase regions. */
#define XFL_MAX_ERASE_REGIONS		XFL_AMD_MAX_ERASE_REGIONS
/** Number of 32-bit integers reserved for vendor data. */
#define XFL_MAX_VENDOR_DATA_LENGTH	10
/** @} */
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */

/**************************** Type Definitions *******************************/

/**
 * @brief Holds the configuration information for the device.
 */
typedef struct {
	u32 BaseAddr;		/**< Base address of array. */
	u8 NumParts;		/**< Number of parts in the array. */
	u8 PartWidth;		/**< Width of each part in bytes. */
	u8 PartMode;		/**< Operation mode of each part in bytes. */
} XFlash_UserInputs;

/**
 * @brief Describes the flash geometry.
 */
typedef struct {
	u32 BaseAddress;	/**< Base address of parts. */
	u32 MemoryLayout;	/**< How multiple parts are connected on the
				 * data bus. Choices are limited to
				 * XFL_LAYOUT_Xa_Xb_Xc constants. */
	u32 DeviceSize;		/**< Total device size in bytes. */
	u32 NumEraseRegions;	/**< Number of erase regions. */
	u16 NumBlocks;		/**< Total number of blocks in device. */
	u8  BootMode;		/**< Boot mode read from the CFI data. */
	struct {
		u32 AbsoluteOffset;	/**< Offset within part where region
					 * begins. */
		u16 AbsoluteBlock;	/**< Block number where region
					 * begins. */
		u16 Number;		/**< Number of blocks in this region. */
		u32 Size;		/**< Size of the block in bytes. */
	} EraseRegion[XFL_MAX_ERASE_REGIONS + 1];
				/**< Erase region definitions. The last entry
				 * is not a real region; it marks one unit
				 * past the addressable region of the part. */

} XFlashGeometry;

/**
 * @brief Holds part specific data maintained by the part component.
 *
 * Within the component, a new type is defined that overlays this type.
 */
typedef u32 XFlashVendorData[XFL_MAX_VENDOR_DATA_LENGTH];

/**
 * @brief Describes the argument list of the various Device Control operations.
 *
 * The union contains one structure for each operation.
 */
typedef union {

	struct {
		XFlashGeometry *GeometryPtr; /**< Pointer to the geometry. */
	} GeometryParam; /**< Arguments of XFL_DEVCTL_GET_GEOMETRY. */

	struct {
		XFlashProperties *PropertiesPtr;	/**< Pointer to the
							 * flash properties. */
	} PropertiesParam; /**< Arguments of XFL_DEVCTL_GET_PROPERTIES. */

	struct {
		u32 Param; /**< Mode to set the RYBY signal to. */
	} RyByParam; /**< Arguments of XFL_DEVCTL_SET_RYBY. */

	struct {
		u32 Error; /**< Last error data. */
	} LastErrorParam; /**< Arguments of XFL_DEVCTL_GET_LAST_ERROR. */

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
	struct {
		u32 Value; /**< Configuration register value. */
	} ConfigRegParam; /**< Arguments of XFL_DEVCTL_SET_CONFIG_REG. */
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */
} DeviceCtrlParam;

/**
 * @brief Stores the XFlash library instance data.
 *
 * Allocate a variable of this type for every flash device in the system.
 * Pass a pointer to this variable to the library API functions.
 */
typedef struct XFlashTag {
	u32 Options;		/**< Current device options. */
	int IsReady;		/**< Device is initialized and ready. */
	int IsPlatformFlash;	/**< Indicates whether this is a platform
				 * flash. */
	u16 CommandSet;		/**< Command algorithm used by part. Choices are
				 * defined in XFL_CMDSET constants. */
	XFlashGeometry Geometry;	/**< Part geometry. */
	XFlashProperties Properties;	/**< Part timing, programming, and
					 * identification properties. */
	XFlashVendorData VendorData;	/**< Part specific data. */


	struct {
		int (*Read) (struct XFlashTag *InstancePtr, u32 Offset,
			     u32 Bytes, void *DestPtr);
				/**< Family specific read function. */

		int (*Write) (struct XFlashTag *InstancePtr, u32 Offset,
			      u32 Bytes, void *SrcPtr);
				/**< Family specific write function. */

		int (*Erase) (struct XFlashTag *InstancePtr, u32 Offset,
			      u32 Bytes);
				/**< Family specific erase function. */

		int (*Lock) (struct XFlashTag *InstancePtr, u32 Offset, u32
			     Bytes);
				/**< Family specific lock function. */

		int (*Unlock) (struct XFlashTag *InstancePtr, u32 Offset, u32
			       Bytes);
				/**< Family specific unlock function. */

		int (*EraseChip) (struct XFlashTag *InstancePtr);
				/**< Family specific chip erase function. */

		int (*Initialize) (struct XFlashTag *Initialize);
				/**< Family specific initialization function. */

		int (*Reset) (struct XFlashTag *InstancePtr);
				/**< Family specific reset function. */
		int (*DeviceControl) (struct XFlashTag *InstancePtr,
				      u32 Command, DeviceCtrlParam
				      *Parameters);
				/**< Family specific device control function. */
	} VTable;			/**< VTable of function pointers to the
					 * family specific APIs. */
	XFlashCommandSet Command;	/**< Flash Specific Commands. */
} XFlash;

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * @name Flash I/O primitives
 *
 * These macros implement flash I/O primitives. All flash components use these
 * macros to access the flash devices. The only exceptions are read functions,
 * which simply copy data using memcpy or equivalent utility functions. The 8,
 * 16, 32, and 64 suffixes signify the access width.
 * @{
 */
/**
 * @brief Reads an 8-bit value from a flash address.
 *
 * @param[in] Address Flash address to read.
 *
 * @return    8-bit value read from Address.
 *
 * @note      C-style signature:
 *            u8 READ_FLASH_8(u32 Address)
 */
#define READ_FLASH_8(Address) (*(volatile u8*)(Address))

/**
 * @brief Reads a 16-bit value from a flash address.
 *
 * @param[in] Address Flash address to read.
 *
 * @return    16-bit value read from Address.
 *
 * @note      C-style signature:
 *            u16 READ_FLASH_16(u32 Address)
 */
#define READ_FLASH_16(Address) (*(volatile u16*)(Address))

/**
 * @brief Reads a 32-bit value from a flash address.
 *
 * @param[in] Address Flash address to read.
 *
 * @return    32-bit value read from Address.
 *
 * @note      C-style signature:
 *            u32 READ_FLASH_32(u32 Address)
 */
#define READ_FLASH_32(Address) (*(volatile u32*)(Address))

/**
 * @brief Reads a 64-bit value from a flash address as two 32-bit reads.
 *
 * @param[in]  Address Flash address to read.
 * @param[out] Data 64-bit variable that receives the value read.
 *
 * @note       C-style signature:
 *             void READ_FLASH_64(u32 Address, Xuint64 Data)
 */
#define READ_FLASH_64(Address, Data) \
	(XUINT64_MSW(Data) = *(volatile u32*)(Address)); \
	(XUINT64_LSW(Data) = *(volatile u32*)(((u32)(Address) + 4)))

/**
 * @brief Writes an 8-bit value to a flash address.
 *
 * @param[in] Address Flash address to write.
 * @param[in] Data Value to write.
 *
 * @note      C-style signature:
 *            void WRITE_FLASH_8(u32 Address, u8 Data)
 */
#define WRITE_FLASH_8(Address, Data) (*(volatile u8*)(Address) = (u8)(Data))

/**
 * @brief Writes a 16-bit value to a flash address.
 *
 * @param[in] Address Flash address to write.
 * @param[in] Data Value to write.
 *
 * @note      C-style signature:
 *            void WRITE_FLASH_16(u32 Address, u16 Data)
 */
#define WRITE_FLASH_16(Address, Data) (*(volatile u16*)(Address) = (u16)(Data))

/**
 * @brief Writes a 32-bit value to a flash address.
 *
 * @param[in] Address Flash address to write.
 * @param[in] Data Value to write.
 *
 * @note      C-style signature:
 *            void WRITE_FLASH_32(u32 Address, u32 Data)
 */
#define WRITE_FLASH_32(Address, Data) (*(volatile u32*)(Address) = (u32)(Data))

/**
 * @brief Writes a 64-bit value to a flash address as two 32-bit writes.
 *
 * @param[in] Address Flash address to write.
 * @param[in] Data 64-bit value to write.
 *
 * @note      C-style signature:
 *            void WRITE_FLASH_64(u32 Address, Xuint64 Data)
 */
#define WRITE_FLASH_64(Address, Data) \
	(*(volatile u32*)(Address) = XUINT64_MSW(Data)); \
	(*(volatile u32*)((u32)(Address) + 4) = XUINT64_LSW(Data))

/**
 * @brief Writes two 32-bit values to consecutive words at a flash address.
 *
 * @param[in] Address Flash address to write.
 * @param[in] Data1 Value to write at Address.
 * @param[in] Data2 Value to write at Address + 4.
 *
 * @note      C-style signature:
 *            void WRITE_FLASH_64x2(u32 Address, u32 Data1, u32 Data2)
 */
#define WRITE_FLASH_64x2(Address, Data1, Data2) \
	(*(volatile u32*)(Address) = Data1); \
	(*(volatile u32*)((u32)(Address) + 4) = Data2)
/** @} */

/*****************************************************************************/
/**
*
* @brief Increments the given Region and Block to the next block address.
*
* Region is incremented if the next block starts in a new region. Block is set
* to zero if the next block starts in a new region, otherwise it is incremented
* by one.
*
* @param[in]     GeometryPtr Geometry instance that defines flash addressing.
* @param[in,out] Region Starting region.
* @param[in,out] Block Starting block.
*
* @note          C-style signature:
*                void XFL_GEOMETRY_INCREMENT(XFlashGeometry *GeometryPtr,
*                                            u16 Region, u16 Block)
*
*****************************************************************************/
#define XFL_GEOMETRY_INCREMENT(GeometryPtr, Region, Block)		\
	{									\
		if ((GeometryPtr)->EraseRegion[Region].Number <= ++(Block))	\
		{								\
			(Region)++; 						\
			(Block) = 0;						\
		}								\
	}

/*****************************************************************************/
/**
*
* @brief Calculates the number of blocks between two block coordinates.
*
* Calculates the number of blocks between the given start and end coordinates.
*
* @param[in] GeometryPtr Geometry instance that defines flash addressing.
* @param[in] StartRegion Starting region.
* @param[in] StartBlock Starting block.
* @param[in] EndRegion Ending region.
* @param[in] EndBlock Ending block.
*
* @return    The number of blocks between start Region/Block and end
*            Region/Block (inclusive).
*
* @note      C-style signature:
*            u32 XFL_GEOMETRY_BLOCK_DIFF(XFlashGeometry *GeometryPtr,
*                                        u16 StartRegion, u16 StartBlock,
*                                        u16 EndRegion, u16 EndBlock)
*
*****************************************************************************/
#define XFL_GEOMETRY_BLOCK_DIFF(GeometryPtr, StartRegion, StartBlock, \
				EndRegion,EndBlock) \
(((GeometryPtr)->EraseRegion[EndRegion].AbsoluteBlock + (EndBlock)) - \
 ((GeometryPtr)->EraseRegion[StartRegion].AbsoluteBlock + 	      \
  (StartBlock)) + 1)

/*****************************************************************************/
/**
*
* @brief Tests whether an absolute offset lies within a geometry address space.
*
* Tests the given absolute Offset to verify it lies within the bounds of the
* address space defined by a geometry instance.
*
* @param[in] GeometryPtr Geometry instance that defines flash addressing.
* @param[in] Offset Offset to test.
*
* @return
*            - 0 if Offset does not lie within the address space described by
*              GeometryPtr.
*            - 1 if Offset is within the address space.
*
* @note      C-style signature:
*            int XFL_GEOMETRY_IS_ABSOLUTE_VALID(XFlashGeometry *GeometryPtr,
*                                               u32 Offset)
*
*****************************************************************************/
#define XFL_GEOMETRY_IS_ABSOLUTE_VALID(GeometryPtr, Offset)  \
	((Offset) < (GeometryPtr)->DeviceSize)

/*****************************************************************************/
/**
*
* @brief Tests whether block coordinates lie within a geometry address space.
*
* Tests the given Region, Block, and Offset to verify they lie within the
* address space defined by a geometry instance.
*
* @param[in] GeometryPtr Geometry instance that defines flash addressing.
* @param[in] Region Region to test.
* @param[in] Block Block to test.
* @param[in] BlockOffset Offset within block.
*
* @return
*            - 0 if Region, Block, & BlockOffset do not lie within the address
*              space described by GeometryPtr.
*            - 1 if Region, Block, & BlockOffset are within the address space.
*
* @note      C-style signature:
*            int XFL_GEOMETRY_IS_BLOCK_VALID(XFlashGeometry *GeometryPtr,
*                                            u16 Region, u16 Block,
*                                            u32 BlockOffset)
*
*****************************************************************************/
#define XFL_GEOMETRY_IS_BLOCK_VALID(GeometryPtr, Region, Block, BlockOffset) \
	(((Region) < ( GeometryPtr)->NumEraseRegions) &&		     \
	 ((Block) < (GeometryPtr)->EraseRegion[Region].Number) &&	     \
	 ((BlockOffset) < (GeometryPtr)->EraseRegion[Region].Size))

/** @} */
/**@endcond */

/************************** Function Prototypes ******************************/

/*
 * Initialization, configuration, & control Functions.
 */
int XFlash_Initialize(XFlash *InstancePtr, u32 BaseAddress, u8 BusWidth,
		      int IsPlatformFlash);
int XFlash_Reset(XFlash *InstancePtr);
int XFlash_DeviceControl(XFlash *InstancePtr, u32 Command,
			 DeviceCtrlParam *Parameters);
int XFlash_Read(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *DestPtr);
int XFlash_Write(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *SrcPtr);
int XFlash_Erase(XFlash *InstancePtr, u32 Offset, u32 Bytes);
int XFlash_Lock(XFlash *InstancePtr, u32 Offset, u32 Bytes);
int XFlash_Unlock(XFlash *InstancePtr, u32 Offset, u32 Bytes);
int XFlash_IsReady(XFlash *InstancePtr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
