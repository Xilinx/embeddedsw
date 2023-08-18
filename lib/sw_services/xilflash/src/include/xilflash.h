/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilflash.h
* @addtogroup xilflash_apis Library-APIs
* @{
* @cond xilflash_internal
* @{
*
* This module implements the functionality for flash memory devices that
* conform to the "Common Flash Interface" (CFI) standard. CFI allows a single
* flash library to be used for an entire family of parts.
*
* This is not a library for a specific device, but for a set of command
* read/write/erase algorithms. CFI allows us to determine which algorithm to
* utilize at runtime.
*
* <b>Library Initialization</b>
*
* The function call XFlash_Initialize() should be called by the application
* before any other function in the library. The initialization function checks
* for the device family and initializes the XFlash instance with the family
* specific data. The VT table (Contains the function pointers to family specific
* API's) is setup and family specific initialization routine is called.
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
*	others have one. The arrangement of blocks and regions is referred to by
*	this module as the part's geometry. Some Intel flash supports multiple
*	banks on the same device. This library supports single and multiple bank
*	flash devices.
*
* 	<b> AMD Flash Device Geometry </b>
*
* 	Flash memory space is segmented into areas called banks and further in
*	to regions and blocks. The size of each block is based on a power of 2.
*	A region is defined as a contiguous set of blocks of the same size. Some
*	parts have several regions while others have one. A bank is defined as a
*	contiguous set of blocks. The bank may contain blocks of different size.
*	The arrangement of blocks, regions and banks is referred to by this
*	module as the part's geometry.
*
* 	The cells within the part can be programmed from a logic 1 to a logic 0
*	and not the other way around. To change a cell back to a logic 1, the
*	entire block containing that cell must be erased. When a block is erased
*	all bytes contain the value 0xFF. The number of times a block can be
*	erased is finite. Eventually the block will wear out and will no longer
*	be capable of erasure. As of this writing, the typical flash block can
*	be erased 100,000 or more times.
*
* <b>Write Operation</b>
*
* The write call can be used to write a minimum of zero bytes and a maximum
* entire flash. If the Offset Address specified to write is out of flash or if
* the number of bytes specified from the Offset address exceed flash boundaries
* an error is reported back to the user. The write is blocking in nature in that
* the control is returned back to user only after the write operation is
* completed successfully or an error is reported.
*
* <b>Read Operation</b>
*
* The read call can be used to read a minimum of zero bytes and maximum of
* entire flash. If the Offset Address specified to write is out of flash
* boundary an error is reported back to the user. The read function reads memory
* locations beyond Flash boundary. Care should be taken by the user to make sure
* that the Number of Bytes + Offset address is within the Flash address
* boundaries. The write is blocking in nature in that the control is returned
* back to user only after the read operation is completed successfully or an
* error is reported.
*
* <b>Erase Operation</b>
*
* The erase operations are provided to erase a Block in the Flash memory. The
* erase call is blocking in nature in that the control is returned back to user
* only after the erase operation is completed successfully or an error is
* reported.
*
* <b>Sector Protection</b>
*
* The Flash Device is divided into Blocks. Each Block can be protected
* individually from unwarranted writing/erasing. The Block locking can be
* achieved using XFlash_Lock() lock. All the memory locations from the Offset
* address specified will be locked. The block can be unlocked using
* XFlash_UnLock() call. All the Blocks which are previously locked will be
* unlocked. The Lock and Unlock calls are blocking in nature in that the control
* is returned back to user only after the operation is completed successfully or
* an error is reported.
* The AMD flash device requires high voltage on Reset pin to perform lock and
* unlock operation. User must provide this high voltage (As defined in
* datasheet) to reset pin before calling lock and unlock API for AMD flash
* devices. Lock and Unlock features are not tested for AMD flash device.
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
*
* <b>This library needs to know the type of EMC core (AXI or XPS) used to
* access the cfi flash, to map the correct APIs. This library should be used
* with the emc driver, v3_01_a and above, so that this information can be
* automatically obtained from the emc driver.</b>
*
* This library is intended to be RTOS and processor independent. It works with
* physical addresses only. Any needs for dynamic memory management, threads,
* mutual exclusion, virtual memory, cache control, or HW write protection
* management must be satisfied by the layer above this library.
* <br><br>
* All writes to flash occur in units of bus-width bytes. If more than one part
* exists on the data bus, then the parts are written in parallel. Reads from
* flash are performed in any width up to the width of the data bus. It is
* assumed that the flash bus controller or local bus supports these types of
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
* </pre>
*
***************************************************************************/

#ifndef XFLASH_H		/* prevent circular inclusions */
#define XFLASH_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

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
 * Vendor command set codes.
 * Refer to industry document "CFI publication 100" for the latest list
 */
#define XFL_CMDSET_INTEL_STANDARD	3	/**< Includes Micron/Sharp */
#define XFL_CMDSET_INTEL_EXTENDED	1	/**< Includes Micron/Sharp */
#define XFL_CMDSET_AMD_STANDARD		2	/**< Includes Fujitsu/STM */
#define XFL_CMDSET_AMD_EXTENDED		4	/**< Includes Fujitsu/STM */
#define XFL_CMDSET_MITSUBISHI_STANDARD	256
#define XFL_CMDSET_MITSUBISHI_EXTENDED	257
#define XFL_CMDSET_INTEL_G18		0x200   /**< Micron G18 Flash */

/*
 * Flash return types.
 */
#define XFLASH_BUSY			101	/**< Flash is erasing or
						 * programming
						 */
#define XFLASH_READY			102	/**< Flash is ready for
						 * commands
						 */
#define XFLASH_ERROR			103	/**< Flash had detected an
						 * internal error. Use
						 * XFlash_DeviceControl
						 * to retrieve device specific
						 * codes
						 */
#define XFLASH_ERASE_SUSPENDED		104	/**< Flash is in suspended erase
						 * state
						 */
#define XFLASH_WRITE_SUSPENDED		105	/**< Flash is in suspended write
						 * state
						 */
#define XFLASH_PART_NOT_SUPPORTED	106	/**< Flash type not supported by
						 * library
						 */
#define XFLASH_NOT_SUPPORTED		107	/**< Operation not supported */
#define XFLASH_TOO_MANY_REGIONS		108	/**< Too many erase regions */
#define XFLASH_TIMEOUT_ERROR		109	/**< Programming or erase
						 * operation aborted due to a
						 * timeout
						 */
#define XFLASH_ADDRESS_ERROR 		110	/**< Accessed flash outside its
						 * addressable range
						 */
#define XFLASH_ALIGNMENT_ERROR		111	/**< Write alignment error */
#define XFLASH_BLOCKING_CALL_ERROR 	112	/**< Couldn't return immediately
						 * from write/erase function
						 * with
						 * XFL_NON_BLOCKING_WRITE/ERASE
						 * option cleared
						 */
#define XFLASH_CFI_QUERY_ERROR		113	/**< Failed to query the device
						 */
#define XFLASH_BLOCK_PROTECTED		114	/**< Block is protected */

/**
 * Supported part arrangements.
 * This enumeration defines the supported arrangements of parts on the
 * data-bus. The naming convention for these constants is as follows:
 *
 *	XFL_LAYOUT_Xa_Xb_Xc, where
 *
 * Xa is the part's physical data bus width. Xb is the is the part's selected
 * data bus width (this field is required because a x16 part can be placed in
 * x8 mode). Xc is the number of interleaved parts. For example one part can
 * be tied to D0-D15 and a second to data lines D15-D31.
 *
 * Parts arranged in series should be treated as separate instances. An example
 * of this layout: Two X16 parts operating in X16 mode. The first part occupies
 * address space FF000000 - FF0FFFFF and a second from FF100000 - FF1FFFFF.
 *
 * These constants are encoded using bit masks defined in the next section.
 */
#define XFL_LAYOUT_X16_X8_X1  0x02020101	/**< One 16-bit part operating
						 * in 8-bit mode. Total data bus
						 * width is 8-bits. This layout
						 * is only supported in AMD
						 * flash devices
						 */
#define XFL_LAYOUT_X16_X16_X1 0x02020201	/**< One 16-bit part operating
						 * in 16-bit mode. Total data
						 * bus width is 16-bits. This
						 * layout is supported in AMD
						 * and Intel flash devices
						 */
#define XFL_LAYOUT_X16_X16_X2 0x04020202	/**< Two 16-bit parts operating
						 * in 16-bit mode. Total data
						 * bus width is 32-bits. This
						 * layout is only supported in
						 * Intel flash devices
						 */
#define XFL_LAYOUT_X16_X16_X4 0x08020204	/**< Four 16-bit parts operating
						 * in 16-bit mode. Total data
						 * bus width is 64-bits. This
						 * layout is only supported in
						 * Intel flash devices
						 */

/*
 * DeviceControl list for all family.
 */
#define XFL_DEVCTL_GET_LAST_ERROR 	1	/**< Retrieve the last error
						 * data */
#define XFL_DEVCTL_GET_GEOMETRY		2	/**< Get Device geometry */
#define XFL_DEVCTL_GET_PROPERTIES	3	/**< Get Device Properties */
#define XFL_DEVCTL_SET_RYBY		4	/**< Set RYBY pin mode */
#define XFL_DEVCTL_ERASE_RESUME		5	/**< Resume Erase */
#define XFL_DEVCTL_ERASE_SUSPEND	6	/**< Suspend Erase */
#define XFL_DEVCTL_ENTER_EXT_MODE	7	/**< Enter Extended mode */
#define XFL_DEVCTL_EXIT_EXT_MODE	8	/**< Exit Extended mode */
#define XFL_DEVCTL_CHIP_ERASE		9	/**< Erase whole chip */
#define XFL_DEVCTL_PROTECTION_STATUS	10	/**< Check block protection
						 * status */
#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#define XFL_DEVCTL_SET_CONFIG_REG	11	/**< Set config register value*/
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */

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

/*
 * Maximum number of erase region and banks for AMD family.
 */
#define XFL_AMD_MAX_ERASE_REGIONS	10

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#define XFL_MAX_ERASE_REGIONS		XFL_INTEL_MAX_ERASE_REGIONS
#define XFL_MAX_VENDOR_DATA_LENGTH	20	/* Number of 32-bit integers
						 * reserved for vendor data
						 */
#else
#define XFL_MAX_ERASE_REGIONS		XFL_AMD_MAX_ERASE_REGIONS
#define XFL_MAX_VENDOR_DATA_LENGTH	10	/* Number of 32-bit integers
						 * reserved for vendor data
						 */
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u32 BaseAddr;	  	/**< Base address of array */
	u8 NumParts;	  	/**< Number of parts in the array */
	u8 PartWidth;	  	/**< Width of each part in bytes */
	u8 PartMode;	  	/**< Operation mode of each part in bytes */
} XFlash_UserInputs;

/**
 * Flash geometry
 */
typedef struct {
	u32 BaseAddress;	/**< Base address of part(s) */
	u32 MemoryLayout;	/**< How multiple parts are connected on
				 * the data bus. Choices are limited to
				 * XFL_LAYOUT_Xa_Xb_Xc constants */
	u32 DeviceSize;		/**< Total device size in bytes */
	u32 NumEraseRegions;	/**< Number of erase regions */
	u16 NumBlocks;		/**< Total number of blocks in device */
	u8  BootMode;
	struct {
		u32 AbsoluteOffset; /**< Offset within part where
				     * region begins */
		u16 AbsoluteBlock;
		/**< Block number where region begins */
		u16 Number;	/**< Number of blocks in this region */
		u32 Size;	/**< Size of the block in bytes */
	} EraseRegion[XFL_MAX_ERASE_REGIONS + 1];

} XFlashGeometry;

/*
 * Define a type to contain part specific data to be maintained by
 * the part component. Within the component, a new type is defined that
 * overlays this type.
 */
typedef u32 XFlashVendorData[XFL_MAX_VENDOR_DATA_LENGTH];

/*
 * Define a union that contains structures which represent the argument list of
 * of various Device Control operations.
 */
typedef union {

	struct {
		XFlashGeometry *GeometryPtr;
	} GeometryParam;

	struct {
		XFlashProperties *PropertiesPtr;
	} PropertiesParam;

	struct {
		u32 Param;
	} RyByParam;

	struct {
		u32 Error;
	} LastErrorParam;

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
	struct {
		u32 Value;
	} ConfigRegParam;
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */
} DeviceCtrlParam;

/**
 * The XFlash library instance data. The user is required to allocate a
 * variable of this type for every flash device in the system. A pointer
 * to a variable of this type is then passed to the library API functions.
 */
typedef struct XFlashTag {
	u32 Options;		/* Current device options */
	int IsReady;		/* Device is initialized and ready */
	int IsPlatformFlash;	/* Indicates whether this a platform flash */
	u16 CommandSet;		/* Command algorithm used by part. Choices
				 * are defined in XFL_CMDSET constants
				 */
	XFlashGeometry Geometry;	/* Part geometry */
	XFlashProperties Properties;	/* Part timing, programming, &
					 * identification properties
					 */
	XFlashVendorData VendorData;	/* Part specific data */


	struct {
		int (*Read) (struct XFlashTag *InstancePtr, u32 Offset,
			     u32 Bytes, void *DestPtr);

		int (*Write) (struct XFlashTag *InstancePtr, u32 Offset,
			      u32 Bytes, void *SrcPtr);

		int (*Erase) (struct XFlashTag *InstancePtr, u32 Offset,
			      u32 Bytes);

		int (*Lock) (struct XFlashTag *InstancePtr, u32 Offset, u32
			     Bytes);

		int (*Unlock) (struct XFlashTag *InstancePtr, u32 Offset, u32
			       Bytes);

		int (*EraseChip) (struct XFlashTag *InstancePtr);

		int (*Initialize) (struct XFlashTag *Initialize);

		int (*Reset) (struct XFlashTag *InstancePtr);
		int (*DeviceControl) (struct XFlashTag *InstancePtr,
				      u32 Command, DeviceCtrlParam
				      *Parameters);
	} VTable;
	XFlashCommandSet Command;	/* Flash Specific Commands */
} XFlash;

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * The following macros implement flash I/O primitives. All flash components
 * use these macros to access the flash devices. The only exceptions are read
 * functions which simply copy data using memcpy or equivalent utility funcs.
 * The 8, 16, 32, 64 signify the access width.
 */
#define READ_FLASH_8(Address) (*(volatile u8*)(Address))

#define READ_FLASH_16(Address) (*(volatile u16*)(Address))

#define READ_FLASH_32(Address) (*(volatile u32*)(Address))

#define READ_FLASH_64(Address, Data) \
	(XUINT64_MSW(Data) = *(volatile u32*)(Address)); \
	(XUINT64_LSW(Data) = *(volatile u32*)(((u32)(Address) + 4)))

#define WRITE_FLASH_8(Address, Data) (*(volatile u8*)(Address) = (u8)(Data))

#define WRITE_FLASH_16(Address, Data) (*(volatile u16*)(Address) = (u16)(Data))

#define WRITE_FLASH_32(Address, Data) (*(volatile u32*)(Address) = (u32)(Data))

#define WRITE_FLASH_64(Address, Data) \
	(*(volatile u32*)(Address) = XUINT64_MSW(Data)); \
	(*(volatile u32*)((u32)(Address) + 4) = XUINT64_LSW(Data))

#define WRITE_FLASH_64x2(Address, Data1, Data2) \
	(*(volatile u32*)(Address) = Data1); \
	(*(volatile u32*)((u32)(Address) + 4) = Data2)

/*****************************************************************************/
/**
*
* Increments the given Region and Block to the next block address.
*
* @param	GeometryPtr is the geometry instance that defines flash
*		addressing.
* @param	Region is the starting region.
* @param	Block is the starting block.
*
* @return	Region parameter is incremented if the next block starts in a
*		new region. Block parameter is set to zero if the next block
*		starts in a new region, otherwise it is incremented by one.
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
* Calculates the number of blocks between the given start and end coordinates.
*
* @param	GeometryPtr is the geometry instance that defines flash
*		addressing.
* @param	StartRegion is the starting region.
* @param	StartBlock is the starting block.
* @param	EndRegion is the ending region.
* @param	EndBlock is the ending block.
*
* @return	The number of blocks between start Region/Block and end
*		Region/Block(inclusive).
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
* Tests the given absolute Offset to verify it lies within the bounds of the
* address space defined by a geometry instance.
*
* @param	GeometryPtr is the geometry instance that defines flash
*		addressing.
* @param	Offset is the offset to test.
*
* @return
* 		- 0 if Offset do not lie within the address space described by
* 		  GeometryPtr.
* 		- 1 if Offset are within the address space.
*
*****************************************************************************/
#define XFL_GEOMETRY_IS_ABSOLUTE_VALID(GeometryPtr, Offset)  \
	((Offset) < (GeometryPtr)->DeviceSize)

/*****************************************************************************/
/**
*
* Tests the given Region, Block, and Offset to verify they lie within the
* address space defined by a geometry instance.
*
* @param	GeometryPtr is the geometry instance that defines flash
*		addressing
* @param	Region is the region to test
* @param	Block is the block to test
* @param	BlockOffset is the offset within block
*
* @return
*		- 0 if Region, Block, & BlockOffset do not lie within the
*		  address space described by GeometryPtr.
*		- 1 if Region, Block, & BlockOffset are within the address space
*
*****************************************************************************/
#define XFL_GEOMETRY_IS_BLOCK_VALID(GeometryPtr, Region, Block, BlockOffset) \
	(((Region) < ( GeometryPtr)->NumEraseRegions) &&		     \
	 ((Block) < (GeometryPtr)->EraseRegion[Region].Number) &&	     \
	 ((BlockOffset) < (GeometryPtr)->EraseRegion[Region].Size))

/**
@}
@endcond */

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

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/* @} */
