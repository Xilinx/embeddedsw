/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_setluncfg_example.c
*
* This example is used to set the device configuration and
* test read and write transfers on UFS interface.
*
*
* Modify the offset and Size macros to test different UFS memory offset and
* size.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.0 	sk  01/16/24 First release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xufspsxc_control.h"		/* UFS device driver */


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int UfsPsxcRawTest(void);
u32 SetConfigurationDesc(XUfsPsxc *InstancePtr);

/************************** Variable Definitions *****************************/
#define ENABLE_READ_WRITE			0
#define ENABLE_SWITCH_BLUN_TEST		0

#define XPAR_XUFSPSXC_0_DEVICE_ID 0x0

/* Number of SD blocks to test */
#define NUM_BLOCKS 5
/* Sector offset to test */
#define SECTOR_OFFSET 0x0

#ifdef __ICCARM__
#pragma data_alignment = 32
u8 DestinationAddress[10*1024];
#pragma data_alignment = 32
u8 SourceAddress[10*1024];
#else
#if ENABLE_READ_WRITE
u8 SourceAddress[NUM_BLOCKS * 4096] __attribute__ ((aligned(64)));
u8 DestinationAddress[NUM_BLOCKS * 4096] __attribute__ ((aligned(64)));
#endif
#endif

static XUfsPsxc UfsInstance;

static XUfsPsxc_Xfer_CmdDesc CmdDescBuf __attribute__ ((aligned(128))) = {0};
u8 ConfigDesc[256];

#define TEST 0x52

/* Configuration Parameters */
static u8 LunEnable[32] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static u8 BootLunId[32] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static u8 Mem_Type[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static u32 SIZE_MB[32] = {1024, 1024, 4096, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define XUFSPSXC_PRIMARY_LUN		XUFSPSXC_BLUN_A		/* Boot-LUN Enable Attribute ID */
#define XUFSPSXC_SECONDARY_LUN		XUFSPSXC_BLUN_B		/* Boot-LUN Enable Attribute ID */

/*****************************************************************************/
/**
*
* Main function to call the UFS example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("UFS Set LUN configuration Test \r\n");

	Status = UfsPsxcRawTest();
	if (Status != XST_SUCCESS) {
		xil_printf("UFS Set LUN configuration Test failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UFS Set LUN configuration Test \r\n");

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function performs the UFS Raw Read/ Write Test.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int UfsPsxcRawTest(void)
{
	XUfsPsxc_Config *UfsConfig;
	u32 Status;
#if	ENABLE_READ_WRITE
	u32 BuffCnt;
	u32 Index;
	/*
	 * Since block size is 4096 bytes. File Size is 4096*BlockCount.
	 */
	u32 FileSize = (4096*NUM_BLOCKS);
	u32 Sector = SECTOR_OFFSET;

	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		SourceAddress[BuffCnt] = TEST + BuffCnt;
		DestinationAddress[BuffCnt] = 0x0;
	}
#endif

#ifdef IPP
	// For wprot 0 to write into PMC SLCR IOU reg
	Xil_Out32(0xF1060828,0x0);
	// For disabling 49,50,51 pins for UFS testing
	Xil_Out32(0xF10600C4,0x18);
	Xil_Out32(0xF10600C8,0x18);
	Xil_Out32(0xF10600CC,0x18);
	// For enabling 1,2,3 pins for UFS testing
	Xil_Out32(0xF1060004,0x8);
	Xil_Out32(0xF1060008,0x8);
	Xil_Out32(0xF106000C,0x8);
#endif

	/*
	 * Initialize the host controller
	 */
	UfsConfig = XUfsPsxc_LookupConfig(XPAR_XUFSPSXC_0_BASEADDR);
	if (NULL == UfsConfig) {
		return XST_FAILURE;
	}

	XUfsPsxc_CfgInitialize(&UfsInstance, UfsConfig);

	Status = XUfsPsxc_Initialize(&UfsInstance);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = SetConfigurationDesc(&UfsInstance);
	if (Status != XST_SUCCESS) {
		return Status;
	}

#if ENABLE_READ_WRITE
	/* Identify the LunId for the Active Boot LUN */
	for (Index = 0; Index < 32; Index++) {
		if (BootLunId[Index] == XUFSPSXC_PRIMARY_LUN)
			break;
	}

	/*
	 * Read data from UFS device.
	 */
	Status = XUfsPsxc_WritePolled(&UfsInstance, Index, Sector, NUM_BLOCKS,
			SourceAddress);
	if (Status!=XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XUfsPsxc_ReadPolled(&UfsInstance, Index, Sector, NUM_BLOCKS,
				   DestinationAddress);
	if (Status!=XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			return XST_FAILURE;
		}
	}

#if ENABLE_SWITCH_BLUN_TEST
	Status = XUfsPsxc_SwitchBootLUN(&UfsInstance);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	for (Index = 0; Index < 32; Index++) {
		if (BootLunId[Index] == XUFSPSXC_SECONDARY_LUN)
			break;
	}

	/*
	 * Read data from UFS device.
	 */
	Status = XUfsPsxc_WritePolled(&UfsInstance, Index, Sector, NUM_BLOCKS,
			SourceAddress);
	if (Status!=XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XUfsPsxc_ReadPolled(&UfsInstance, Index, Sector, NUM_BLOCKS,
				   DestinationAddress);
	if (Status!=XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			return XST_FAILURE;
		}
	}
#endif
#endif

	return XST_SUCCESS;
}

u32 SetConfigurationDesc(XUfsPsxc *InstancePtr)
{
	u32 Status;
	u32 Index;
	u32 Tsf_DW0;
	volatile u32 LUIndex;
	u32 NumAllocUnits;
	u32 MemType;

	/* Write bBootLunEn IDN in Attributes */
	memset((void *)&CmdDescBuf, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillAttrUpiu(InstancePtr, &CmdDescBuf, XUFSPSXC_WRITE, XUFSPSXC_BLUNEN_ATTRID, XUFSPSXC_PRIMARY_LUN);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, &CmdDescBuf);
	if (Status != XUFSPSXC_SUCCESS) {
		Status = (XUFSPSXC_QRY_WRITE_ATTR_ERROR << 12U) | Status;
		return Status;
	}
	InstancePtr->BootLunEn = XUFSPSXC_PRIMARY_LUN;

	/* Read Configuration Descriptor */
	for (Index = 0U; Index < InstancePtr->NumOfLuns / 8U; Index++) {
		memset((void *)&CmdDescBuf, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
		Tsf_DW0 = (XUFSPSXC_CONFIG_DESC_IDN << 8U | Index << 16U);
		XUfsPsxc_FillDescUpiu(InstancePtr, &CmdDescBuf, Tsf_DW0, XUFSPSXC_READ, XUFSPSXC_CFG_DESC_LEN(InstancePtr));
		Status = XUfsPsxc_ProcessUpiu(InstancePtr, &CmdDescBuf);
		if (Status != XUFSPSXC_SUCCESS) {
			Status = (XUFSPSXC_QRY_READ_CFG_DESC_ERROR << 12U) | Status;
			return Status;
		}

		memcpy( &ConfigDesc[0], &CmdDescBuf.RespUpiu.QueryRespUpiu.Data[0], XUFSPSXC_CFG_DESC_LEN(InstancePtr));
		memset((void *)&CmdDescBuf, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
		memcpy( &CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[0], &ConfigDesc[0], XUFSPSXC_CFG_DESC_LEN(InstancePtr));

		for (LUIndex = 0U; LUIndex < 8U; LUIndex++) {
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_LU_ENABLE_OFFSET(InstancePtr, LUIndex)] = LunEnable[(Index * 8) + LUIndex];
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_BLUNEN_OFFSET(InstancePtr, LUIndex)] = BootLunId[(Index * 8) + LUIndex];

			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_BLKSZ_OFFSET(InstancePtr, LUIndex)] = 12;	/* 4K Block Size */

			MemType = Mem_Type[(Index * 8) + LUIndex];
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_MEMTYPE_OFFSET(InstancePtr, LUIndex)] = MemType;

			NumAllocUnits = (((u64)SIZE_MB[(Index * 8) + LUIndex] * (u64)1024 * (u64)1024 * (u64)InstancePtr->CapAdjFactor[MemType]) / ((u64)InstancePtr->AllocUnitSize * (u64)InstancePtr->SegmentSize * (u64)512));
			NumAllocUnits = Xil_EndianSwap32(NumAllocUnits);

			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex)] = (u8)NumAllocUnits;
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex) + 1] = (u8)(NumAllocUnits >> 8);
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex) + 2] = (u8)(NumAllocUnits >> 16);
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex) + 3] = (u8)(NumAllocUnits >> 24);
		}

		if (Index == 0)
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[3] = 1;	/* Boot Enable */

		if (Index == 3)
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[2] = 0;
		else
			CmdDescBuf.ReqUpiu.QueryReqUpiu.Data[2] = 1;

		Tsf_DW0 = (XUFSPSXC_CONFIG_DESC_IDN << 8U | Index << 16U);
		XUfsPsxc_FillDescUpiu(InstancePtr, &CmdDescBuf, Tsf_DW0, XUFSPSXC_WRITE, XUFSPSXC_CFG_DESC_LEN(InstancePtr));
		Status = XUfsPsxc_ProcessUpiu(InstancePtr, &CmdDescBuf);
		if (Status != XUFSPSXC_SUCCESS) {
			Status = (XUFSPSXC_QRY_WRITE_CFG_DESC_ERROR << 12U) | Status;
			return Status;
		}
	}

	return XST_SUCCESS;
}
