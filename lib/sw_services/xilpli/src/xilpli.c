/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilpli.c
*
* This file contains the code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/25/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xilpli.h"
#include "xillibpm_api.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************/
#define XILPLI_DEVICEOPS_INIT(DevInit, DevCopy)	\
	{ \
		.DeviceBaseAddr = 0U, \
		.Init = DevInit, \
		.Copy = DevCopy, \
	}

XilPli_DeviceOps DeviceOps[] =
{
	XILPLI_DEVICEOPS_INIT(XPli_SbiInit, XPli_SbiCopy),  /* JTAG - 0U */
#ifdef  XILPLI_QSPI
	XILPLI_DEVICEOPS_INIT(XPli_Qspi24Init, XPli_Qspi24Copy), /* QSPI24 - 1U */
	XILPLI_DEVICEOPS_INIT(XPli_Qspi32Init, XPli_Qspi32Copy), /* QSPI32- 2U */
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
#ifdef	XILPLI_SD_0
	XILPLI_DEVICEOPS_INIT(XPli_SdInit, XPli_SdCopy), /* SD0 - 3U*/
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
	XILPLI_DEVICEOPS_INIT(NULL, NULL),  /* 4U */
#ifdef  XILPLI_SD_0
	XILPLI_DEVICEOPS_INIT(XPli_SdInit, XPli_SdCopy), /* eMMC0 - 5U */
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
#ifdef  XILPLI_SD_1
	XILPLI_DEVICEOPS_INIT(XPli_SdInit, XPli_SdCopy), /* SD1 - 6U */
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
	XILPLI_DEVICEOPS_INIT(NULL, NULL),  /* 7U */
#ifdef  XILPLI_OSPI
	XILPLI_DEVICEOPS_INIT(XPli_OspiInit, XPli_OspiCopy), /* OSPI - 8U */
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
	XILPLI_DEVICEOPS_INIT(NULL, NULL), /* 9U */
#ifdef XILPLI_SMAP
	XILPLI_DEVICEOPS_INIT(XPli_SbiInit, XPli_SbiCopy), /* SMAP - 0xA */
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
	XILPLI_DEVICEOPS_INIT(NULL, NULL), /* 0xBU */
	XILPLI_DEVICEOPS_INIT(NULL, NULL), /* 0xCU */
	XILPLI_DEVICEOPS_INIT(NULL, NULL), /* 0xDU */
#ifdef XILPLI_SD_1
	XILPLI_DEVICEOPS_INIT(XPli_SdInit, XPli_SdCopy) /* SD1 LS - 0xEU */
#else
	XILPLI_DEVICEOPS_INIT(NULL, NULL),
#endif
};

#if 0
/*****************************************************************************/
/**
 * This function initializes the subsystem with its CDO file.
 * CDO file can contain the system topologies, subsystem policies and
 * system set requirements
 *
 * @param CdoBuf is the pointer to the CDO contents
 *
 * @return	returns SUCCESS on success
 *
 *****************************************************************************/
int XSubSys_Init(u32 * CdoBuf)
{

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to load the PDI file and configure different
 * subsystems present in it.
 * It stores the subsystem handle and corresponding PDI images for later
 * usage
 *
 * @param PdiSrc is source of PDI. It can be in Boot Device, DDR
 * @param PdiAddr is the address at PDI is located in the PDI source
 *        mentioned
 *
 * @return	returns SUCCESS on success
 *
 *****************************************************************************/
int XSubSys_LoadPdi(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr)
{

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to copy the PDI image to a new address like DDR so
 * that it can be loaded and started runtime without reading from boot device
 * After copying the image it updates the PDI source and address fields
 *
 * @param PdiSrc is source of PDI. It can be in Boot Device, DDR
 * @param SrcAddr is the address at which PDI is located in the PDI source
 *        mentioned
 * @param DestAddr Address where PDI has to be copied to
 * @param PdiLen is length of the PDI to be copied
 *
 * @return	returns SUCCESS on success
 *****************************************************************************/
int XSubSys_CopyPdi(u32 PdiSrc, u64 SrcAddr, u64 DestAddr, u32 PdiLen)
{

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to restart a subsystem that is already loaded.
 * Using subsystem ID, corresponding PDI image will be reloaded.
 *
 * @param SubsystemId is the subsystem handle returned by libPM or Master ID
 * of processor
 *
 * @return	returns XILPLI_SUCCESS on success
 *****************************************************************************/
int XSubSys_ReStart(u32 SubsysHd)
{

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * This function initializes the PDI instance with required details and read
 * the meta header
 *
 * @param Pdi instance pointer where PDI details are stored
 * @param PdiSrc is source of PDI. It can be in Boot Device, DDR
 * @param PdiAddr is the address at PDI is located in the PDI source
 *        mentioned
 *
 * @return	returns XILPLI_SUCCESS on success
 *
 *****************************************************************************/
int XPli_PdiInit(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr)
{
	int Status;

	/**
	 * Update PDI Ptr with source, addr, meta header
	 */
	PdiPtr->PdiSrc = PdiSrc;
	PdiPtr->PdiAddr = PdiAddr;
	PdiPtr->PdiType = 1;

	if(DeviceOps[PdiSrc].Init==NULL)
	{
		Status = XILPLI_UNSUPPORTED_BOOT_MODE;
		goto END;
	}

	Status = DeviceOps[PdiSrc].Init(PdiSrc);
	if(Status != XST_SUCCESS)
        {
                goto END;
        }

	PdiPtr->DeviceCopy =  DeviceOps[PdiSrc].Copy;
	PdiPtr->MetaHdr.DeviceCopy = PdiPtr->DeviceCopy;
	PdiPtr->MetaHdr.FlashOfstAddr = 0U;
	/**
	 * Read meta header from PDI source
	 */
	Status = XilPdi_ReadBootHdr(&PdiPtr->MetaHdr);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	Status = XilPdi_ReadAndValidateImgHdrTbl(&PdiPtr->MetaHdr);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	Status = XilPdi_ReadAndVerifyPrtnHdr(&PdiPtr->MetaHdr);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to load the PDI image. It reads meta header and start
 * loading the images as present in the PDI
 *
 * @param PdiPtr Pdi instance pointer
 *
 * @return	returns XILPLI_SUCCESS on success
 *****************************************************************************/
int XPli_LoadSubSystemPdi(XilPdi *PdiPtr)
{

	/**
	 * From the meta header present in PDI pointer, read the subsystem
	 * image details and load, start all the images
	 *
	 * For every image,
	 *   1. Read the CDO file if present
	 *   2. Send the CDO file to cdo parser which directs
	 *      CDO commands to Xilpm, and other components
	 *   3. Load partitions to respective memories
	 */
	int Status;
	for(u32 PrtnNum=1U;
	    PrtnNum < PdiPtr->MetaHdr.ImgHdrTable.NoOfPrtns; ++PrtnNum)
	{
		Status = XPli_PrtnLoad(PdiPtr, PrtnNum);
		if(Status != XST_SUCCESS)
		goto END;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to start the subsystems in the PDI.
 *
 * @param PdiPtr Pdi instance pointer
 *
 * @return	returns XILPLI_SUCCESS on success
 *****************************************************************************/
int XPli_StartSubSystemPdi(XilPdi *PdiPtr)
{
	int Status;
        u32 Index;
        u32 CpuId;
        u64 HandoffAddr;

	XPli_Printf(DEBUG_INFO, "XPli_StartSubSystemPdi enter\r\n");
        /* Handoff to the cpus */
        for (Index=0U;Index<PdiPtr->NoOfHandoffCpus;Index++)
        {
                CpuId = PdiPtr->HandoffParam[Index].CpuSettings
                        & XIH_PH_ATTRB_DSTN_CPU_MASK;
                HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;

                switch (CpuId)
                {
                        case XIH_PH_ATTRB_DSTN_CPU_A72_0:
                        case XIH_PH_ATTRB_DSTN_CPU_A72_1:
                        {
				XPli_Printf(DEBUG_INFO,
				    " Request APU wakeup\r\n");
				Status = XPm_RequestWakeUp(
					XPM_DEVID_ACPU_0, 1, HandoffAddr);
                        }break;
                        case XIH_PH_ATTRB_DSTN_CPU_R5_0:
                        case XIH_PH_ATTRB_DSTN_CPU_R5_1:
                        case XIH_PH_ATTRB_DSTN_CPU_R5_L:
                        default:
                        {
                        }break;
                }

        }
        Status = XILPLI_SUCCESS;
	return Status;
}

#if 0
/*****************************************************************************/
/**
 * This function is used load a image in PDI. PDI can have multiple images
 * present in it. This can be used to load a single image like PL, APU, RPU.
 * This will load all the partitions that are present in that image.
 *
 * @param PdiPtr Pdi instance pointer
 * @param ImageId Id of the image present in PDI
 *
 * @return	returns XILPLI_SUCCESS on success
 *****************************************************************************/
int XPli_LoadImage(XilPdi *PdiPtr, u32 ImageId)
{


	return XILPLI_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to start the PDI image. For processor, reset is
 * released. For PL, end global sequence is done
 *
 * @param PdiPtr Pdi instance pointer
 * @param ImageId Id of the image present in PDI
 *
 * @return	returns XILPLI_SUCCESS on success
 *****************************************************************************/
int XPli_StartImage(XilPdi *PdiPtr, u32 ImageId)
{


	return XILPLI_SUCCESS;
}
#endif
