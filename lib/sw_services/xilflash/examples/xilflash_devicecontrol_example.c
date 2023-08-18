/******************************************************************************
* Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilflash_devicecontrol_example.c
*
* This file contains a design example using the Generic Flash Library.
* This example displays the Flash device geometry and properties. The geometry
* and properties are fetched using Device Control API.
*
*
* @note		None.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.01a mta  10/09/07 First release
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 3.00a sdm  03/03/11 Updated to pass BaseAddress and Flash Width to _Initialize
*		      API, as required by the new version of the library
* 4.7	akm  07/23/19 Initialized Status variable to XST_FAILURE.
* 4.10	akm  07/14/23 Added support for system device-tree flow.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <xilflash.h>
#include <xil_types.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants define the baseaddress and width the flash memory.
 * These constants map to the XPAR parameters created in the xparameters.h file.
 * They are defined here such that a user can easily change all the needed
 * parameters in one place.
 */
#ifndef SDT
#define FLASH_BASE_ADDRESS	XPAR_EMC_0_S_AXI_MEM0_BASEADDR
#else
#define FLASH_BASE_ADDRESS	XPAR_XEMC_0_BASEADDR
#endif
/*
 * The following constant defines the total byte width of the flash memory. The
 * user needs to update this width based on the flash width in the design/board.
 * The total flash width on some of the Xilinx boards is listed below.
 * -------------------------------
 * Board		Width
 * -------------------------------
 * ML403		4 (32 bit)
 * ML5xx		2 (16 bit)
 * Spartan3S 1600E	2 (16 bit)
 * Spartan-3A DSP	2 (16 bit)
 * Spartan-3A		2 (16 bit)
 * Spartan-3AN		2 (16 bit)
 * ML605		2 (16 bit)
 * SP605		2 (16 bit)
 * SP601		1 (8 bit)
 */
#define FLASH_MEM_WIDTH		2

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int FlashDeviceControlExample(void);

/************************** Variable Definitions *****************************/

XFlash FlashInstance; /* XFlash Instance. */

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
*
* Main function to execute the Flash device control example.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf(" Flash device control Test \r\n");
	Status = FlashDeviceControlExample();
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Flash device control Test \r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function fetches and displays the geometry and properties of the Flash *
* device.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashDeviceControlExample(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	DeviceCtrlParam IoctlParams;

	/*
	 * Initialize the Flash Library.
	 */
	Status = XFlash_Initialize(&FlashInstance, FLASH_BASE_ADDRESS,
				   FLASH_MEM_WIDTH, 0);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Reset the Flash Device. This clears the Status registers and puts
	 * the device in Read mode.
	 */
	Status = XFlash_Reset(&FlashInstance);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Fetch the flash device properties and display.
	 */
	Status = XFlash_DeviceControl(&FlashInstance,
				XFL_DEVCTL_GET_PROPERTIES, &IoctlParams);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("\n\r\t\tFlash Properties\n\r");
	xil_printf("FlashProperties->PartID.ManufacturerID = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						PartID.ManufacturerID);

	xil_printf("FlashProperties->PartID.DeviceID = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
							PartID.DeviceID);

	xil_printf("FlashProperties->PartID.DeviceID = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
							PartID.CommandSet);

	xil_printf("FlashProperties->TimeTypical.WriteSingle_Us = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeTypical.WriteSingle_Us);

	xil_printf("FlashProperties->TimeTypical.WriteBuffer_Us = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeTypical.WriteBuffer_Us);

	xil_printf("FlashProperties->TimeTypical.EraseBlock_Ms = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeTypical.EraseBlock_Ms);

	xil_printf("FlashProperties->TimeTypical.EraseChip_Ms = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeTypical.EraseChip_Ms);

	xil_printf("FlashProperties->TimeMax.WriteSingle_Us = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeMax.WriteSingle_Us);

	xil_printf("FlashProperties->TimeMax.WriteBuffer_Us = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeMax.WriteBuffer_Us);

	xil_printf("FlashProperties->TimeMax.EraseBlock_Ms = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeMax.EraseBlock_Ms);

	xil_printf("FlashProperties->TimeMax.EraseChip_Ms = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						TimeMax.EraseChip_Ms);

	xil_printf("FlashProperties->ProgCap.WriteBufferSize = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						ProgCap.WriteBufferSize);

	xil_printf("FlashProperties->ProgCap.WriteBufferAlignMask = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->ProgCap.
						WriteBufferAlignmentMask);

	xil_printf("FlashProperties->ProgCap.EraseQueueSize = 0x%x\n\r",
			IoctlParams.PropertiesParam.PropertiesPtr->
						ProgCap.EraseQueueSize);

	/*
	 * Fetch the flash device geometry and display.
	 */
	Status = XFlash_DeviceControl(&FlashInstance,
				XFL_DEVCTL_GET_GEOMETRY, &IoctlParams);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("\n\r\n\r\t\tFlash Geometry\n\r");

	xil_printf("FlashGeometry->BaseAddress = 0x%x\n\r",
			IoctlParams.GeometryParam.GeometryPtr->BaseAddress);

	xil_printf("FlashGeometry->MemoryLayout = 0x%x\n\r",
			IoctlParams.GeometryParam.GeometryPtr->MemoryLayout);

	xil_printf("FlashGeometry->DeviceSize = 0x%x\n\r",
			IoctlParams.GeometryParam.GeometryPtr->DeviceSize);

	xil_printf("FlashGeometry->NumEraseRegions = 0x%x\n\r",
			IoctlParams.GeometryParam.GeometryPtr->NumEraseRegions);

	xil_printf("FlashGeometry->NumBlocks = 0x%x\n\r",
			IoctlParams.GeometryParam.GeometryPtr->NumBlocks);

	for(Index = 0; Index < IoctlParams.GeometryParam.GeometryPtr->
					NumEraseRegions; Index++) {
		xil_printf("\tErase region %d\n\r", Index);

		xil_printf("Absolute Offset = 0x%x\n\r",
				IoctlParams.GeometryParam.GeometryPtr->
					EraseRegion[Index].AbsoluteOffset);

		xil_printf("Absolute Block = 0x%x\n\r",
				IoctlParams.GeometryParam.GeometryPtr->
					EraseRegion[Index].AbsoluteBlock);

		xil_printf("Num Of Block = 0x%x\n\r",
				IoctlParams.GeometryParam.GeometryPtr->
						EraseRegion[Index].Number);

		xil_printf("Size Of Block = 0x%x\n\r",
				IoctlParams.GeometryParam.GeometryPtr->
						EraseRegion[Index].Size);
	}

	return XST_SUCCESS;
}
