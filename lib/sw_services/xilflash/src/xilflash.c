/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilflash.c
* @brief Implements the XFlash library APIs for parallel flash devices.
*
* This file contains the library functions to Initialize, Read, Write, Erase,
* Lock and Unlock the Parallel Flash Device.
* The family specific functionalities are implemented in DeviceControl API.
* Refer to xilflash.h for a detailed description.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/20/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.02a ksu  06/16/09 Added support for multiple banks in Intel flash.
*                     Added support for Xilinx Platform Flash XL. If the
*                     user selects to use the Xilinx Platform Flash XL then
*                     it is set to Async mode during the initialization.
*                     If the user wants to use it in Sync mode then it can be
*                     done using the XFlash_DeviceControl API.
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros.
* 3.00a sdm  03/03/11 Removed static parameters in mld and updated code to
*		      determine these parameters from the CFI data.
* 3.01a srt  03/02/12 Added support for Micron G18 Flash device to fix
*		      CRs 648372, 648282.
* 3.02a srt  05/30/12 Changed Implementation for Micron G18 Flash, which
*		      fixes the CR 662317.
*		      CR 662317 Description - Xilinx Platform Flash on ML605
*		      fails to work.
* 4.4   ms   08/03/17 Added tags and modified comment lines style for doxygen.
* 4.10	akm  07/14/23 Added support for system device-tree flow.
* 4.14  vn   08/26/26 Fix Doxygen warnings.
* 4.14  vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
****************************************************************************/

/***************************** Include Files *********************************/

#include "include/xilflash.h"
#include "include/xilflash_cfi.h"
#include "xstatus.h"
#ifdef SDT
#include "xilflash_config.h"
#endif
#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
#include "include/xilflash_intel.h"	/* Intel specific header file */
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */
#ifdef XPAR_XFL_DEVICE_FAMILY_AMD
#include "include/xilflash_amd.h"	/* AMD specific header file */
#endif /* XPAR_XFL_DEVICE_FAMILY_AMD */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int SetVTable(XFlash *InstancePtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief Initializes a specific XFlash instance.
*
* The initialization entails:
* - Check the Device family type.
* - Issuing the CFI query command.
* - Get and translate relevant CFI query information.
* - Set default options for the instance.
* - Setup the VTable.
* - Call the family initialize function of the instance.
* - Initialize the Xilinx Platform Flash XL to Async mode if the application
*   selects to use the Platform Flash XL in the MLD. The Platform Flash XL is
*   an Intel CFI compliant device.
*
* @param[out] InstancePtr Pointer to the XFlash instance.
* @param[in]  BaseAddress Base address of the flash memory.
* @param[in]  BusWidth Total width of the flash memory, in bytes.
* @param[in]  IsPlatformFlash Used to specify if the flash is a platform flash.
*
* @return
*             - XST_SUCCESS if successful.
*             - XFLASH_PART_NOT_SUPPORTED if the command set algorithm or Layout
*               is not supported by any flash family compiled into the system.
*             - XFLASH_CFI_QUERY_ERROR if the device would not enter CFI query
*               mode. Either the devices lack CFI support, the wrong
*               BaseAddress param was used, an unsupported part layout exists,
*               or a hardware problem exists with the part.
*
* @note       BusWidth is not the width of an individual part. It is the total
*             operating width. For example, if there are two 16-bit parts, with
*             one tied to data lines D0-D15 and other tied to D16-D31, BusWidth
*             would be (32 / 8) = 4. If a single 16-bit flash is in 8-bit mode,
*             then BusWidth should be (8 / 8) = 1.
*
******************************************************************************/
int XFlash_Initialize(XFlash *InstancePtr, u32 BaseAddress, u8 BusWidth,
		      int IsPlatformFlash)
{
	int Status = XST_FAILURE;

	/*
	 * Validate parameters.
	 */
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (BusWidth > 8) {
		return XFLASH_PART_NOT_SUPPORTED;
	}

	InstancePtr->IsReady = 0;
	InstancePtr->Geometry.BaseAddress = BaseAddress;
	InstancePtr->IsPlatformFlash = IsPlatformFlash;

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
	if (IsPlatformFlash == 1) {
		/*
		 * Set Async mode for platform flash
		 */
		WRITE_FLASH_16(InstancePtr->Geometry.BaseAddress +
			       XFL_INTEL_CMD_CONFIG_REG_ASYNC_ADDR,
			       XFL_INTEL_CMD_CONFIG_REG_SETUP);
		WRITE_FLASH_16(InstancePtr->Geometry.BaseAddress +
			       XFL_INTEL_CMD_CONFIG_REG_ASYNC_ADDR,
			       XFL_INTEL_CMD_CONFIG_REG_CONFIRM);
	}
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */

	/*
	 * Get CFI data.
	 */
	Status = XFlashCFI_ReadCommon(InstancePtr, BusWidth);
	if (Status != XST_SUCCESS) {
		return (XFLASH_CFI_QUERY_ERROR);
	}

	/*
	 * Set the VTable function pointer based on the command set algorithm
	 * discovered in the CFI query.
	 */
	Status = SetVTable(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Initialize the specific flash family device.
	 * If it initializes successfully set the IsReady flag to indicate the
	 * device is ready.
	 */
	Status = InstancePtr->VTable.Initialize(InstancePtr);
	if (Status == XST_SUCCESS) {
		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief Reads data from the Flash device into the specified buffer.
*
* Reads the data from the Flash device and copies it into the specified
* destination buffer.
*
* The source and destination addresses can be on any alignment supported by the
* processor.
*
* The device is polled until an error or the operation completes successfully.
*
* @param[in,out] InstancePtr Pointer to the XFlash instance.
* @param[in]     Offset Offset into the device address space from which to
*                read.
* @param[in]     Bytes Number of bytes to copy.
* @param[out]    DestPtr Destination address to copy data to.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_ADDRESS_ERROR if the source address does not start
*                  within the addressable areas of the devices.
*
* @note          This function allows the transfer of data past the end of the
*                device's address space. If this occurs, then results are
*                undefined.
*
******************************************************************************/
int XFlash_Read(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *DestPtr)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.Read(InstancePtr, Offset, Bytes, DestPtr));
}

/*****************************************************************************/
/**
* @brief Programs the flash devices with data specified in the source buffer.
*
* The source and destination address must be aligned to the width of the flash's
* data bus.
*
* The device is polled until an error or the operation completes successfully.
*
* @param[in,out] InstancePtr Pointer to the XFlash instance.
* @param[in]     Offset Offset into the device address space from which to
*                begin programming. Must be aligned to the width of the
*                flash's data bus.
* @param[in]     Bytes Number of bytes to program.
* @param[in]     SrcPtr Source address containing data to be programmed. Must
*                be aligned to the width of the flash's data bus. The SrcPtr
*                does not have to be aligned to the flash width if the
*                processor supports unaligned access. But this library is
*                generic, and some processors (for example, MicroBlaze) do not
*                support unaligned access. So this API requires the SrcPtr to
*                be aligned.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_ERROR if a write error occurred. This error is
*                  usually device specific. Use XFlash_DeviceControl() to
*                  retrieve specific error conditions. When this error is
*                  returned, it is possible that the target address range was
*                  only partially programmed.
*
******************************************************************************/
int XFlash_Write(XFlash *InstancePtr, u32 Offset, u32 Bytes, void *SrcPtr)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.Write(InstancePtr, Offset, Bytes, SrcPtr));
}

/*****************************************************************************/
/**
* @brief Erases the specified address range in the flash device.
*
* The number of bytes to erase can be any number as long as it is within the
* bounds of the devices.
*
* The device is polled until an error or the operation completes successfully.
*
* @param[in,out] InstancePtr Pointer to the XFlash instance.
* @param[in]     Offset Offset into the device address space from which to
*                begin erasure.
* @param[in]     Bytes Number of bytes to erase.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_ADDRESS_ERROR if the destination address range is
*                  not completely within the addressable areas of the devices.
*
* @note          Due to flash memory design, the range actually erased may be
*                larger than what was specified by the Offset & Bytes
*                parameters. This occurs if the parameters do not align to
*                block boundaries.
*
******************************************************************************/
int XFlash_Erase(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.Erase(InstancePtr, Offset, Bytes));
}

/*****************************************************************************/
/**
* @brief Locks the blocks in the specified range of the flash devices.
*
* The device is polled until an error or the operation completes successfully.
*
* @param[in,out] InstancePtr Pointer to the XFlash instance.
* @param[in]     Offset Offset into the device address space from which to
*                begin block locking. The first three bytes of every block are
*                reserved for a special purpose. The offset should be at least
*                three bytes from the start of the block.
* @param[in]     Bytes Number of bytes to lock in the block starting from
*                Offset.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_ADDRESS_ERROR if the destination address range is
*                  not completely within the addressable areas of the devices.
*
* @note          Due to flash memory design, the range actually locked may be
*                larger than what was specified by the Offset & Bytes
*                parameters. This occurs if the parameters do not align to
*                block boundaries.
*
******************************************************************************/
int XFlash_Lock(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.Lock(InstancePtr, Offset, Bytes));
}

/*****************************************************************************/
/**
* @brief Unlocks the blocks in the specified range of the flash devices.
*
* The device is polled until an error or the operation completes successfully.
*
* @param[in,out] InstancePtr Pointer to the XFlash instance.
* @param[in]     Offset Offset into the device address space from which to
*                begin block UnLocking. The first three bytes of every block
*                are reserved for a special purpose. The offset should be at
*                least three bytes from the start of the block.
* @param[in]     Bytes Number of bytes to unlock in the block starting from
*                Offset.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_ADDRESS_ERROR if the destination address range is
*                  not completely within the addressable areas of the devices.
*
******************************************************************************/
int XFlash_Unlock(XFlash *InstancePtr, u32 Offset, u32 Bytes)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.Unlock(InstancePtr, Offset, Bytes));
}

/*****************************************************************************/
/**
* @brief Resets the flash device and places it in read mode.
*
* @param[in,out] InstancePtr Pointer to the XFlash instance.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_BUSY if the flash devices were in the middle of an
*                  operation and could not be reset.
*                - XFLASH_ERROR if the devices have experienced an internal
*                  error during the operation. XFlash_DeviceControl() must be
*                  used to access the cause of the device specific error
*                  condition.
*
******************************************************************************/
int XFlash_Reset(XFlash *InstancePtr)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.Reset(InstancePtr));
}

/*****************************************************************************/
/**
* @brief Executes device specific commands.
*
* For a list of device specific commands, see xilflash.h.
*
* @param[in]     InstancePtr Pointer to the XFlash instance.
* @param[in]     Command Device specific command to issue.
* @param[in,out] Parameters Specifies the arguments passed to the device control
*                function.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_NOT_SUPPORTED if the command is not
*                  recognized/supported by the devices.
*
******************************************************************************/
int XFlash_DeviceControl(XFlash *InstancePtr, u32 Command,
			 DeviceCtrlParam *Parameters)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		return XST_FAILURE;
	}

	return (InstancePtr->VTable.DeviceControl(InstancePtr, Command,
			Parameters));
}

/*****************************************************************************/
/**
* @brief Checks whether the device has been successfully initialized.
*
* Checks the readiness of the device, which means it has been successfully
* initialized.
*
* @param[in] InstancePtr Pointer to the XFlash instance.
*
* @return    TRUE if the device has been initialized (but not necessarily
*            started), and FALSE otherwise.
*
******************************************************************************/
int XFlash_IsReady(XFlash *InstancePtr)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	return (InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
}

/*****************************************************************************/
/**
*
* @brief Converts block coordinates to a part offset.
*
* Region, Block, & BlockOffset are converted to PartOffset.
*
* @param[in]  InstancePtr Pointer to the XFlashGeometry instance.
* @param[in]  Region Erase region the physical address appears in.
* @param[in]  Block Block within Region the physical address appears in.
* @param[in]  BlockOffset Offset within Block where the physical address
*             appears.
* @param[out] AbsoluteOffsetPtr Returned offset value.
*
* @return
*             - XST_SUCCESS if successful.
*             - XFLASH_ADDRESS_ERROR if the block coordinates are invalid.
*
******************************************************************************/
int XFlashGeometry_ToAbsolute(XFlashGeometry *InstancePtr,
			      u16 Region, u16 Block, u32 BlockOffset,
			      u32 *AbsoluteOffsetPtr)
{
	/*
	 * Verify inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (AbsoluteOffsetPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Validate Region, Block, and BlockOffset parameters.
	 */
	if (!XFL_GEOMETRY_IS_BLOCK_VALID(InstancePtr, Region, Block,
					 BlockOffset)) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Calculate offset.
	 */
	*AbsoluteOffsetPtr = InstancePtr->EraseRegion[Region].AbsoluteOffset +
			     (InstancePtr->EraseRegion[Region].Size * Block) + BlockOffset;

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* @brief Converts a part offset to block coordinates.
*
* PartOffset is converted to Region, Block & BlockOffset.
*
* @param[in]  InstancePtr Pointer to the XFlashGeometry.
* @param[in]  AbsoluteOffset Offset within part to find block coordinates for.
* @param[out] RegionPtr Region that corresponds to AbsoluteOffset. This is a
*             return parameter.
* @param[out] BlockPtr Block within Region that corresponds to AbsoluteOffset.
*             This is a return parameter.
* @param[out] BlockOffsetPtr Offset within Block that corresponds to
*             AbsoluteOffset. This is a return parameter.
*
* @return
*             - XST_SUCCESS if successful.
*             - XFLASH_ADDRESS_ERROR if the block coordinates are invalid.
*
******************************************************************************/
int XFlashGeometry_ToBlock(XFlashGeometry *InstancePtr,
			   u32 AbsoluteOffset, u16 *RegionPtr,
			   u16 *BlockPtr, u32 *BlockOffsetPtr)
{
	u32 Index;
	u32 RemainderOffset;

	/*
	 * Verify inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	if (RegionPtr == NULL) {
		return XST_FAILURE;
	}

	if (BlockPtr == NULL) {
		return XST_FAILURE;
	}

	if (BlockOffsetPtr == NULL) {
		return XST_FAILURE;
	}

	/*
	 * Determine if the offset exceeds the device's size.
	 */
	if (!XFL_GEOMETRY_IS_ABSOLUTE_VALID(InstancePtr, AbsoluteOffset)) {
		return (XFLASH_ADDRESS_ERROR);
	}

	/*
	 * Calculate the region. When the loop breaks, Index will index one past
	 * the region that the offset appears in.
	 */
	for (Index = 1; Index < XFL_MAX_ERASE_REGIONS; Index++) {
		if (AbsoluteOffset <
		    InstancePtr->EraseRegion[Index].AbsoluteOffset) {
			break;
		}
	}

	Index--;

	/*
	 * Calculate the remaining offset from which block and block offset
	 * are calculated.
	 */
	RemainderOffset = AbsoluteOffset -
			  InstancePtr->EraseRegion[Index].AbsoluteOffset;

	/*
	 * Calculate block & offset, assign return values and return.
	 */
	*RegionPtr = Index;
	*BlockPtr = RemainderOffset / InstancePtr->EraseRegion[Index].Size;
	*BlockOffsetPtr = RemainderOffset -
			  (*BlockPtr * InstancePtr->EraseRegion[Index].Size);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* @brief Sets up the VTable function pointers for the instance.
*
* The VTable is set up based on the instance's Properties->CommandSet
* attribute.
*
* @param[in,out] InstancePtr Pointer to the Flash instance.
*
* @return
*                - XST_SUCCESS if successful.
*                - XFLASH_NOT_SUPPORTED if no matching part is found.
*
******************************************************************************/
static int SetVTable(XFlash *InstancePtr)
{


	switch (InstancePtr->CommandSet) {

#ifdef XPAR_XFL_DEVICE_FAMILY_INTEL
		case XFL_CMDSET_INTEL_EXTENDED:
		case XFL_CMDSET_INTEL_STANDARD:
		case XFL_CMDSET_INTEL_G18:
			InstancePtr->VTable.Read = XFlashIntel_Read;
			InstancePtr->VTable.Write = XFlashIntel_Write;
			InstancePtr->VTable.Erase = XFlashIntel_Erase;
			InstancePtr->VTable.Lock = XFlashIntel_Lock;
			InstancePtr->VTable.Unlock = XFlashIntel_Unlock;
			InstancePtr->VTable.Initialize = XFlashIntel_Initialize;
			InstancePtr->VTable.Reset = XFlashIntel_Reset;
			InstancePtr->VTable.DeviceControl =
				XFlashIntel_DeviceControl;
			break;
#endif /* XPAR_XFL_DEVICE_FAMILY_INTEL */

#ifdef XPAR_XFL_DEVICE_FAMILY_AMD
		case XFL_CMDSET_AMD_EXTENDED:
		case XFL_CMDSET_AMD_STANDARD:
			InstancePtr->VTable.Read = XFlashAmd_Read;
			InstancePtr->VTable.Write = XFlashAmd_Write;
			InstancePtr->VTable.Erase = XFlashAmd_Erase;
			InstancePtr->VTable.Lock = XFlashAmd_Lock;
			InstancePtr->VTable.Unlock = XFlashAmd_Unlock;
			InstancePtr->VTable.EraseChip = XFlashAmd_EraseChip;
			InstancePtr->VTable.Initialize = XFlashAmd_Initialize;
			InstancePtr->VTable.Reset = XFlashAmd_Reset;
			InstancePtr->VTable.DeviceControl =
				XFlashAmd_DeviceControl;
			break;
#endif /* XPAR_XFL_DEVICE_FAMILY_AMD */

		default:
			return (XFLASH_PART_NOT_SUPPORTED);
	}

	return (XST_SUCCESS);
}
