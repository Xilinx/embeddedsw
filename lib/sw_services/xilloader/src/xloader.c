/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* @file xloader.c
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
#include "xloader.h"
#include "xillibpm_api.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XilSubsystem SubSystemInfo;

/*****************************************************************************/
#define XLOADER_DEVICEOPS_INIT(DevInit, DevCopy)	\
	{ \
		.DeviceBaseAddr = 0U, \
		.Init = DevInit, \
		.Copy = DevCopy, \
	}

XLoader_DeviceOps DeviceOps[] =
{
	XLOADER_DEVICEOPS_INIT(XLoader_SbiInit, XLoader_SbiCopy),  /* JTAG - 0U */
#ifdef  XLOADER_QSPI
	XLOADER_DEVICEOPS_INIT(XLoader_Qspi24Init, XLoader_Qspi24Copy), /* QSPI24 - 1U */
	XLOADER_DEVICEOPS_INIT(XLoader_Qspi32Init, XLoader_Qspi32Copy), /* QSPI32- 2U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
#endif
#ifdef	XLOADER_SD_0
	XLOADER_DEVICEOPS_INIT(XLoader_SdInit, XLoader_SdCopy), /* SD0 - 3U*/
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL),  /* 4U */
#ifdef  XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT(XLoader_SdInit, XLoader_SdCopy), /* SD1 - 5U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
#endif
#ifdef  XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT(XLoader_SdInit, XLoader_SdCopy), /* EMMC - 6U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL),  /* 7U */
#ifdef  XLOADER_OSPI
	XLOADER_DEVICEOPS_INIT(XLoader_OspiInit, XLoader_OspiCopy), /* OSPI - 8U */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL), /* 9U */
#ifdef XLOADER_SBI
	XLOADER_DEVICEOPS_INIT(XLoader_SbiInit, XLoader_SbiCopy), /* SMAP - 0xA */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
#endif
	XLOADER_DEVICEOPS_INIT(NULL, NULL), /* 0xBU */
	XLOADER_DEVICEOPS_INIT(NULL, NULL), /* 0xCU */
	XLOADER_DEVICEOPS_INIT(NULL, NULL), /* 0xDU */
#ifdef XLOADER_SD_1
	XLOADER_DEVICEOPS_INIT(XLoader_SdInit, XLoader_SdCopy) /* SD1 LS - 0xEU */
#else
	XLOADER_DEVICEOPS_INIT(NULL, NULL),
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
 * @return	returns XLOADER_SUCCESS on success
 *****************************************************************************/
int XSubSys_ReStart(u32 SubsysHd)
{

	return XST_SUCCESS;
}
#endif

XLoader* XLoader_GetLoaderInstancePtr(void)
{
	static XLoader XLoaderInstance={0U, 0U, 0U, 0U, 0U};
	return (XLoader*)&XLoaderInstance;
}

/*****************************************************************************/
/**
 * This function initializes the loader instance and registers loader
 * commands with PLM
 *
 * @param None
 *
 * @return	returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_Init()
{
	int Status;
	XLoader* XLoaderPtr = XLoader_GetLoaderInstancePtr();

	/** Initializes the DMA pointers */
	XPlmi_DmaInit();

	Status = XLoader_CfiInit(XLoaderPtr);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}
END:
	return Status;
}

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
 * @return	returns XLOADER_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_PdiInit(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr)
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
		Status = XLOADER_UNSUPPORTED_BOOT_MODE;
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
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_BOOTHDR, Status);
		goto END;
	}

	Status = XilPdi_ReadAndValidateImgHdrTbl(&PdiPtr->MetaHdr);
	if(Status != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IMGHDR_TBL, Status);
		goto END;
	}

	/*
	 * Read and verify image headers
	 */
	Status = XilPdi_ReadAndVerifyImgHdr(&(PdiPtr->MetaHdr));
	if (XST_SUCCESS != Status)
	{
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_IMGHDR, Status);
		goto END;
	}

	Status = XilPdi_ReadAndVerifyPrtnHdr(&PdiPtr->MetaHdr);
	if(Status != XST_SUCCESS)
	{
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_PRTNHDR, Status);
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to load and start the PDI image. It reads meta header,
 * loads the images as present in the PDI and starts based on hand-off
 * information present in PDI
 *
 * @param PdiPtr Pdi instance pointer
 *
 * @return	returns XLOADER_SUCCESS on success
 *****************************************************************************/
int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr)
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
	u32 ImgNum;

	for (ImgNum = 1U; ImgNum < PdiPtr->MetaHdr.ImgHdrTable.NoOfImgs; ++ImgNum)
	{
		Status = XLoader_LoadImage(PdiPtr, 0xFFFFFFFFU);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XLoader_StartImage(PdiPtr);
		XPlmi_Printf(DEBUG_INFO, "PDI start status: 0x%x\n\r", Status);
		if (Status != XST_SUCCESS)
		{
			goto END;
		}
	}
	SubSystemInfo.PdiPtr = PdiPtr;
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
 * @return	returns XLOADER_SUCCESS on success
 *****************************************************************************/
int XLoader_StartImage(XilPdi *PdiPtr)
{
	int Status;
    u32 Index;
    u32 CpuId;
    u64 HandoffAddr;

	XLoader_Printf(DEBUG_INFO, "XLoader_StartImage enter\r\n");
    /* Handoff to the cpus */
	for (Index = 0U; Index < PdiPtr->NoOfHandoffCpus; Index++)
    {
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings
				& XIH_PH_ATTRB_DSTN_CPU_MASK;
		HandoffAddr = PdiPtr->HandoffParam[Index].HandoffAddr;

		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_A72_0:
			case XIH_PH_ATTRB_DSTN_CPU_A72_1:
			{
				XLoader_Printf(DEBUG_INFO,
						" Request APU wakeup\r\n");
				Status = XPm_RequestWakeUp(XPM_SUBSYSID_PMC,
						XPM_DEVID_ACPU_0, 1, HandoffAddr, 0);
				if (Status != XST_SUCCESS)
				{
					Status = XPLMI_UPDATE_STATUS(
						XLOADER_ERR_WAKEUP_A72, Status);
					goto END;
				}

			}break;

			case XIH_PH_ATTRB_DSTN_CPU_R5_0:
			{
				XLoader_Printf(DEBUG_INFO,
						"Request RPU 0 wakeup\r\n");
				XPm_DevIoctl(XPM_SUBSYSID_PMC, XPM_DEVID_R50_0,
						IOCTL_SET_RPU_OPER_MODE,
						XPM_RPU_MODE_SPLIT, 0, 0);
				Status = XPm_RequestWakeUp(XPM_SUBSYSID_PMC, XPM_DEVID_R50_0,
						1, HandoffAddr, 0);
				if (Status != XST_SUCCESS)
				{
					Status = XPLMI_UPDATE_STATUS(
						XLOADER_ERR_WAKEUP_R5_0, Status);
					goto END;
				}
			}break;

			case XIH_PH_ATTRB_DSTN_CPU_R5_1:
			{
				XLoader_Printf(DEBUG_INFO,
						"Request RPU 1 wakeup\r\n");
				XPm_DevIoctl(XPM_SUBSYSID_PMC, XPM_DEVID_R50_1,
						IOCTL_SET_RPU_OPER_MODE,
						XPM_RPU_MODE_SPLIT, 0, 0);
				Status = XPm_RequestWakeUp(XPM_SUBSYSID_PMC, XPM_DEVID_R50_1,
						1, HandoffAddr, 0);
				if (Status != XST_SUCCESS)
				{
					Status = XPLMI_UPDATE_STATUS(
						XLOADER_ERR_WAKEUP_R5_1, Status);
					goto END;
				}
			}break;

			case XIH_PH_ATTRB_DSTN_CPU_R5_L:
			{
				XLoader_Printf(DEBUG_INFO,
						"Request RPU wakeup\r\n");
				XPm_DevIoctl(XPM_SUBSYSID_PMC, XPM_DEVID_R50_0,
						IOCTL_SET_RPU_OPER_MODE,
						XPM_RPU_MODE_LOCKSTEP, 0, 0);
				Status = XPm_RequestWakeUp(XPM_SUBSYSID_PMC, XPM_DEVID_R50_0,
						1, HandoffAddr, 0);
				if (Status != XST_SUCCESS)
				{
					Status = XPLMI_UPDATE_STATUS(
						XLOADER_ERR_WAKEUP_R5_L, Status);
					goto END;
				}
			}break;

			default:
			{
			}break;
		}
    }

	/*
	 * Make Number of handoff CPUs to zero
	 */
	PdiPtr->NoOfHandoffCpus = 0x0U;
    Status = XLOADER_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used load a image in PDI. PDI can have multiple images
 * present in it. This can be used to load a single image like PL, APU, RPU.
 * This will load all the partitions that are present in that image.
 *
 * @param PdiPtr Pdi instance pointer
 * @param ImageId Id of the image present in PDI
 *
 * @return	returns XLOADER_SUCCESS on success
 *****************************************************************************/
int XLoader_LoadImage(XilPdi *PdiPtr, u32 ImageId)
{
	u32 Index;
	static u32 ImgNum = 1U;
	static u32 PrtnNum = 1U;
	u32 Status;

	if (0xFFFFFFFFU != ImageId)
	{
		for (Index = 0U; Index < SubSystemInfo.Count; Index ++) {
			if (ImageId == SubSystemInfo.SubsystemLut[Index].SubsystemId) {
				ImgNum = SubSystemInfo.SubsystemLut[Index].ImageNum;
				PrtnNum = SubSystemInfo.SubsystemLut[Index].PrtnNum;
				break;
			}
		}
		if (Index == SubSystemInfo.Count) {
			Status = XST_FAILURE;
			goto END;
		}
	} else
	{
		SubSystemInfo.SubsystemLut[SubSystemInfo.Count].SubsystemId =
				PdiPtr->MetaHdr.ImgHdr[ImgNum].ImgID;
		SubSystemInfo.SubsystemLut[SubSystemInfo.Count].ImageNum = ImgNum;
		SubSystemInfo.SubsystemLut[SubSystemInfo.Count++].PrtnNum = PrtnNum;
	}

	Status = XLoader_LoadImagePrtns(PdiPtr, ImgNum, PrtnNum);
	PrtnNum += PdiPtr->MetaHdr.ImgHdr[ImgNum].NoOfPrtns;
	ImgNum++;

END:
	return Status;
}

#if 0
/*****************************************************************************/
/**
 * This function is used to start the PDI image. For processor, reset is
 * released. For PL, end global sequence is done
 *
 * @param PdiPtr Pdi instance pointer
 * @param ImageId Id of the image present in PDI
 *
 * @return	returns XLOADER_SUCCESS on success
 *****************************************************************************/
int XLoader_StartImage(XilPdi *PdiPtr, u32 ImageId)
{


	return XLOADER_SUCCESS;
}
#endif
