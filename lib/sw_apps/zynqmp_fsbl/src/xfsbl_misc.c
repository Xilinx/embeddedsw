/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* @file xfsbl_common.c
*
* This is the file which contains common code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*       vns  01/29/17 Added API XFsbl_AdmaCopy to trasfer data using ADMA
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_main.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/
#define XFSBL_BASE_FILE_NAME_LEN_SD_0 8
#define XFSBL_BASE_FILE_NAME_LEN_SD_1 11
#define XFSBL_NUM_DIGITS_IN_FILE_NAME 4

/**************************** Type Definitions *******************************/
typedef struct {
	u32 Id;
	char *Name;
} XFsblPs_ZynqmpDevices;

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
static void XFsbl_UndefHandler (void);
#ifndef ARMA53_64
static void XFsbl_SvcHandler (void);
static void XFsbl_PreFetchAbortHandler (void);
#endif
static void XFsbl_DataAbortHandler (void);
static void XFsbl_IrqHandler (void);
static void XFsbl_FiqHandler (void);

/**
 * functions from xfsbl_main.c
 */
static s32 XFsbl_Strcmp(const char* Str1Ptr,  const char* Str2Ptr);
/************************** Variable Definitions *****************************/
#if defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR) && !defined (ARMR5)
#ifdef ARMA53_64
extern void MMUTableL1(void);
extern void MMUTableL2(void);
#else
extern void MMUTable(void);
#endif
#endif

/****************************************************************************/
/**
* This function is used to print the entire array in bytes as specified by the
* debug type
*
* @param DebugType printing of the array will happen as defined by the
* debug type
*
* @param Buf pointer to the  buffer to be printed
*
* @param Len length of the bytes to be printed
*
* @param Str pointer to the data that is printed along the
* data
*
* @return None
*
* @note
*
*****************************************************************************/
void XFsbl_PrintArray (u32 DebugType, const u8 Buf[], u32 Len, const char *Str)
{
	u32 Index;

	if ((DebugType & XFsblDbgCurrentTypes) != 0U)
	{
		XFsbl_Printf(DebugType, "%s START\r\n", Str);
		for (Index=0U;Index<Len;Index++)
		{
			XFsbl_Printf(DEBUG_INFO, "%02lx ",Buf[Index]);
			if (((Index+1U)%16U) == 0U){
				XFsbl_Printf(DEBUG_INFO, "\r\n");
			}
		}
		XFsbl_Printf(DebugType,"\r\n%s END\r\n",Str);
	}
	return;
}


/*****************************************************************************/
/**
 *
 *
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
char *XFsbl_Strcpy(char *DestPtr, const char *SrcPtr)
{
	u32 Count;

	for (Count=0U; SrcPtr[Count] != '\0'; ++Count)
	{
		DestPtr[Count] = SrcPtr[Count];
	}
	DestPtr[Count] = '\0';

	return DestPtr;
}


/*****************************************************************************/
/**
 *
 *
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
char * XFsbl_Strcat(char* Str1Ptr, const char* Str2Ptr)
{
	while( *Str1Ptr > '\0')
	{
		Str1Ptr++;
	}

	while( *Str2Ptr > '\0')
	{
		*Str1Ptr = *Str2Ptr;
		Str1Ptr++; Str2Ptr++;
	}

	*Str1Ptr = '\0';
	return --Str1Ptr;
}

/*****************************************************************************/
/**
 * This function is used to compare two strings
 *
 *
 * @param	Str1Ptr First string to be compared
 *
 * @param	Str2Ptr Second string to be compared
 *
 * @return	0 if both strings are same, -1/1 if first non matching character has
 * 				lower/greater value in Str1Ptr
 *
 ******************************************************************************/
static s32 XFsbl_Strcmp( const char* Str1Ptr, const char* Str2Ptr)
{
	s32 retVal;

	while (*Str1Ptr == *Str2Ptr) {
		if (*Str1Ptr == '\0') {
			retVal = 0;
			goto END;
		}
		Str1Ptr++;
		Str2Ptr++;
	}

	if( *Str1Ptr < *Str2Ptr) {
		retVal = -1;
	}
	else {
		retVal = 1;
	}

END:
	return retVal;
}


/*****************************************************************************/
/**
 *
 *
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void* XFsbl_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len)
{
	u8 *Dst = DestPtr;
	const u8 *Src = SrcPtr;

	/* Loop and copy.  */
	while (Len != 0U)
	{
		*Dst = *Src;
		Dst++;
		Src++;
		Len--;
	}

	return DestPtr;
}

/*****************************************************************************/
/**
 *
 *
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 *
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_MakeSdFileName(char *XFsbl_SdEmmcFileName,
		u32 MultibootReg, u32 DrvNum)
{

	u32 Index;
	u32 Value;
	u32 FileNameLen;
	u32 MultiBootNum = MultibootReg;

	if (0x0U == MultiBootNum)
	{
		/* SD file name is BOOT.BIN when Multiboot register value is 0 */
		if (DrvNum == XFSBL_SD_DRV_NUM_0) {
			(void)XFsbl_Strcpy(XFsbl_SdEmmcFileName, "BOOT.BIN");
		}
		else {
			/* For second SD instance, include drive number 1 as well */
			(void)XFsbl_Strcpy(XFsbl_SdEmmcFileName, "1:/BOOT.BIN");
		}
	}
	else
	{
		/* set default SD file name as BOOT0000.BIN */
		if (DrvNum == XFSBL_SD_DRV_NUM_0) {
			(void)XFsbl_Strcpy(XFsbl_SdEmmcFileName, "BOOT0000.BIN");
			FileNameLen = XFSBL_BASE_FILE_NAME_LEN_SD_0;
		}
		else {
			/* For second SD instance, include drive number 1 as well */
			(void)XFsbl_Strcpy(XFsbl_SdEmmcFileName, "1:/BOOT0000.BIN");
			FileNameLen = XFSBL_BASE_FILE_NAME_LEN_SD_1;
		}

		/* Update file name (to BOOTXXXX.BIN) based on Multiboot register value */
		for(Index = FileNameLen - 1U;
				Index >= (FileNameLen - XFSBL_NUM_DIGITS_IN_FILE_NAME);
				Index--)
		{
			Value = MultiBootNum % 10U;
			MultiBootNum = MultiBootNum / 10U;
			XFsbl_SdEmmcFileName[Index] += (char)Value;
			if (MultiBootNum == 0U)
			{
				break;
			}
		}
	}

	XFsbl_Printf(DEBUG_INFO,
			"File name is %s\r\n",XFsbl_SdEmmcFileName);
}

/*****************************************************************************/
/**
 * This function is used to obtain drive number based on design and boot mode
 *
 * @param	DeviceFlags contain boot mode information
 *
 * @return	Drive number (0 or 1)
 *
 *****************************************************************************/
u32 XFsbl_GetDrvNumSD(u32 DeviceFlags)
{
	u32 DrvNum;

	/*
	 * If design has both SD0 and SD1, select drive number based on bootmode
	 * If design has ONLY SD0 or ONLY SD1, drive number should be "0"
	 */
#ifdef XPAR_XSDPS_1_DEVICE_ID
	if ((DeviceFlags == XFSBL_SD0_BOOT_MODE) ||
			(DeviceFlags == XFSBL_EMMC_BOOT_MODE)) {
		DrvNum = XFSBL_SD_DRV_NUM_0;
	}
	else {
		/* For XFSBL_SD1_BOOT_MODE or XFSBL_SD1_LS_BOOT_MODE */
		DrvNum = XFSBL_SD_DRV_NUM_1;
	}
#else
	DrvNum = XFSBL_SD_DRV_NUM_0;
#endif

	return DrvNum;
}

/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static void XFsbl_UndefHandler (void)
{
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_UNDEFINED_EXCEPTION\n\r");
	XFsbl_ErrorLockDown(XFSBL_ERROR_UNDEFINED_EXCEPTION);
}

#ifndef ARMA53_64
/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static void XFsbl_SvcHandler (void)
{
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_SVC_EXCEPTION\n\r");
	XFsbl_ErrorLockDown(XFSBL_ERROR_SVC_EXCEPTION);
}

/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*****************************************************************************/
static void XFsbl_PreFetchAbortHandler (void)
{
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_PREFETCH_ABORT_EXCEPTION\n\r");
	XFsbl_ErrorLockDown(XFSBL_ERROR_PREFETCH_ABORT_EXCEPTION);
}
#endif

/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static void XFsbl_DataAbortHandler (void)
{
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_DATA_ABORT_EXCEPTION\n\r");
	XFsbl_ErrorLockDown(XFSBL_ERROR_DATA_ABORT_EXCEPTION);
}

/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static void XFsbl_IrqHandler (void)
{
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_IRQ_EXCEPTION\n\r");
	XFsbl_ErrorLockDown(XFSBL_ERROR_IRQ_EXCEPTION);
}

/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static void XFsbl_FiqHandler (void)
{
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_FIQ_EXCEPTION\n\r");
	XFsbl_ErrorLockDown(XFSBL_ERROR_FIQ_EXCEPTION);
}

/*****************************************************************************/
/**
*
* This function Registers the Exception Handlers
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XFsbl_RegisterHandlers(void)
{
	Xil_ExceptionInit();

	 /*
	 * Initialize the vector table. Register the stub Handler for each
	 * exception.
	 */
#ifdef ARMA53_64
	 Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
                (Xil_ExceptionHandler)XFsbl_UndefHandler,(void *) 0);
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                (Xil_ExceptionHandler)XFsbl_IrqHandler,(void *) 0);
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
                (Xil_ExceptionHandler)XFsbl_FiqHandler,(void *) 0);
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
                (Xil_ExceptionHandler)XFsbl_DataAbortHandler,(void *) 0);
#else
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT,
		(Xil_ExceptionHandler)XFsbl_UndefHandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT,
		(Xil_ExceptionHandler)XFsbl_SvcHandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT,
		(Xil_ExceptionHandler)XFsbl_PreFetchAbortHandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
		(Xil_ExceptionHandler)XFsbl_DataAbortHandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
		(Xil_ExceptionHandler)XFsbl_IrqHandler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
		(Xil_ExceptionHandler)XFsbl_FiqHandler,(void *) 0);
#endif
}

/*****************************************************************************/
/**
*
* This function checks the power state of one or more power islands and
* powers them up if required.
*
* @param	Mask of Island(s) that need to be powered up
*
* @return	XFSBL_SUCCESS for successful power up or
* 		    XFSBL_FAILURE otherwise.
*
* @note		None.
*
****************************************************************************/
u32 XFsbl_PowerUpIsland(u32 PwrIslandMask)
{

	u32 RegVal;
	u32 Status = XFSBL_SUCCESS;

	/* Skip power-up request for QEMU */
	if (XGet_Zynq_UltraMp_Platform_info() != (u32)XPLAT_ZYNQ_ULTRA_MPQEMU)
	{
		/* There is a single island for both R5_0 and R5_1 */
		if ((PwrIslandMask & PMU_GLOBAL_PWR_STATE_R5_1_MASK) ==
				PMU_GLOBAL_PWR_STATE_R5_1_MASK) {
			PwrIslandMask &= ~(PMU_GLOBAL_PWR_STATE_R5_1_MASK);
			PwrIslandMask |= PMU_GLOBAL_PWR_STATE_R5_0_MASK;
		}

		/* Power up request enable */
		XFsbl_Out32(PMU_GLOBAL_REQ_PWRUP_INT_EN, PwrIslandMask);

		/* Trigger power up request */
		XFsbl_Out32(PMU_GLOBAL_REQ_PWRUP_TRIG, PwrIslandMask);

		/* Poll for Power up complete */
		do {
			RegVal = XFsbl_In32(PMU_GLOBAL_REQ_PWRUP_STATUS) & PwrIslandMask;
		} while (RegVal != 0x0U);
	}

	return Status;
}

/**
*
* This function is used to request isolation restore, through PMU
*
* @param	Mask of the entries for which isolation is to be restored
*
* @return	XFSBL_SUCCESS (for now always returns this)
*
* @note		None.
*
****************************************************************************/
u32 XFsbl_IsolationRestore(u32 IsolationMask)
{

	u32 RegVal;
	u32 Status = XFSBL_SUCCESS;

	/* Skip power-up request for QEMU */
	if (XGet_Zynq_UltraMp_Platform_info() != (u32)XPLAT_ZYNQ_ULTRA_MPQEMU)
	{

		/* Isolation request enable */
		XFsbl_Out32(PMU_GLOBAL_REQ_ISO_INT_EN, IsolationMask);

		/* Trigger Isolation request */
		XFsbl_Out32(PMU_GLOBAL_REQ_ISO_TRIG, IsolationMask);

		/* Poll for Isolation complete */
		do {
			RegVal = XFsbl_In32(PMU_GLOBAL_REQ_ISO_STATUS) & IsolationMask;
		} while (RegVal != 0x0U);
	}

	return Status;
}

#if defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR) && !defined (ARMR5)
/*****************************************************************************
*
* Set the memory attributes for a section, in the translation table.
*
* @param	addr is the address for which attributes are to be set.
* @param	attrib specifies the attributes for that memory region.
*
* @return	None.
*
* @note		The MMU and D-cache need not be disabled before changing an
*			translation table attribute.
*
******************************************************************************/
void XFsbl_SetTlbAttributes(INTPTR Addr, UINTPTR attrib)
{
	void (*Funcptr)(void);
#ifdef ARMA53_64
	INTPTR *ptr;
	INTPTR section;
	u64 block_size;
	/* if region is less than 4GB MMUTable level 2 need to be modified */
	if(Addr < ADDRESS_LIMIT_4GB){
		/* block size is 2MB for addressed < 4GB*/
		block_size = BLOCK_SIZE_2MB;
		section = Addr / block_size;
		Funcptr = &MMUTableL2;
		ptr = (INTPTR*)Funcptr + section;
	}
	/* if region is greater than 4GB MMUTable level 1 need to be modified */
	else{
		/* block size is 1GB for addressed > 4GB */
		block_size = BLOCK_SIZE_1GB;
		section = Addr / block_size;
		Funcptr = &MMUTableL1;
		ptr = (INTPTR*)Funcptr + section;
	}
	*ptr = (Addr & (~(block_size-1))) | attrib;

	mtcptlbi(ALLE3);

	dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
#else
	u32 *ptr;
	u32 section;

	section = Addr / 0x100000U;
	Funcptr = &MMUTable;
	ptr = (u32*)Funcptr + section;

	if(ptr != NULL) {
		*ptr = (Addr & 0xFFF00000U) | attrib;
	}

	mtcp(XREG_CP15_INVAL_UTLB_UNLOCKED, 0U);
	/* Invalidate all branch predictors */
	mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0U);

	dsb(); /* ensure completion of the BP and TLB invalidation */
	isb(); /* synchronize context on this processor */
#endif
}
#endif

/*****************************************************************************
*
* This function looksup for the Device-SVD Id and returns corresponding
* DeviceId Name
*
* @param	none
*
* @return	string containing Device Id Name or "UNKN" if none found
*
******************************************************************************/
const char *XFsbl_GetSiliconIdName(void)
{
	/* Lookup table for Device-SVD Id and DeviceId Name */
	static XFsblPs_ZynqmpDevices ZynqmpDevices[] = {
		{0x10U, "3",},
		{0x11U, "2",},
		{0x20U, "5",},
		{0x21U, "4",},
		{0x30U, "7",},
		{0x38U, "9",},
		{0x39U, "6",},
		{0x40U, "11",},
		{0x50U, "15",},
		{0x58U, "19",},
		{0x59U, "17",},
	};
	u32 DevSvdId;
	u32 Index;

	DevSvdId = XFsbl_In32(CSU_IDCODE);

	DevSvdId &= CSU_IDCODE_DEVICE_CODE_MASK | CSU_IDCODE_SVD_MASK;
	DevSvdId >>= CSU_IDCODE_SVD_SHIFT;

	for (Index = 0U; Index < ARRAY_SIZE(ZynqmpDevices); Index++) {
		if (ZynqmpDevices[Index].Id == DevSvdId) {
			return ZynqmpDevices[Index].Name;
		}
	}

	return "UNKN";
}

/*****************************************************************************
*
* This function determines and returns the Processor System Id and Engine Type
* Currently it checks between CG/EG and returns accordingly.
*
* @param	none
*
* @return	"CG" or "EG" based on IPDISABLE register
*
******************************************************************************/
const char *XFsbl_GetProcEng(void)
{
	u32 DevSvdId = XFsbl_In32(CSU_IDCODE);
	DevSvdId &= CSU_IDCODE_DEVICE_CODE_MASK | CSU_IDCODE_SVD_MASK;
	DevSvdId >>= CSU_IDCODE_SVD_SHIFT;

	if ((XFsbl_In32(EFUSE_IPDISABLE) & EFUSE_IPDISABLE_CG_MASK) ==
			EFUSE_IPDISABLE_CG_MASK) {
		return "CG";
	}
	else if((DevSvdId == 0x20U) || (DevSvdId == 0x21U) || (DevSvdId == 0x30U))
	{
		if ((XFsbl_In32(EFUSE_IPDISABLE) & EFUSE_IPDISABLE_VCU_DIS_MASK) ==
				EFUSE_IPDISABLE_VCU_DIS_MASK) {
			return "EG";
		}
		else
		{
			return "EV";
		}

	}
	else
	{
		return "EG";
	}

}

/*****************************************************************************
*
* This function checks if a given CPU is supported by this variant of Silicon
* Currently it checks if it is CG part and disallows handoff to A53_2/3 cores
*
* @param	none
*
* @return	XFSBL_SUCCESS if supported CPU, XFSBL_FAILURE if not.
*
******************************************************************************/
u32 XFsbl_CheckSupportedCpu(u32 CpuId)
{
	u32 Status;

	if ((0 == XFsbl_Strcmp(XFsbl_GetProcEng(), "CG")) &&
			((CpuId == XIH_PH_ATTRB_DEST_CPU_A53_2) ||
			(CpuId == XIH_PH_ATTRB_DEST_CPU_A53_3))){
		Status = XFSBL_FAILURE;
		goto END;
	}

	/* Add code to check for support of other CPUs/cores in future */
	Status = XFSBL_SUCCESS;

END:
	return Status;
}

/******************************************************************************
*
* This function copies data memory to memory using ADMA.
*
* @param	DestPtr is a pointer to destination buffer to which data needs
*		to be copied.
* @param	SrcPtr is a pointer to the source buffer.
* @param	size holds the size of the data to be transfered.
*
* @return
*		Success on successful copy
*		Error on failure.
*
* @note		Cache invalidation and flushing should be taken care by user
*		Before calling this API ADMA also should be configured to
*		simple DMA.
*
******************************************************************************/
u32 XFsbl_AdmaCopy(void * DestPtr, void * SrcPtr, u32 Size)
{
	u32 RegVal;
	u64 SrcAddr = (UINTPTR)SrcPtr;
	u64 DstAddr = (UINTPTR)DestPtr;
	u32 Status = XFSBL_SUCCESS;

	/* Wait until the DMA is in idle state */
	do {
		RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_STATUS);
		RegVal &= ADMA_CH0_ZDMA_CH_STATUS_STATE_MASK;
	} while ((RegVal != ADMA_CH0_ZDMA_CH_STATUS_STATE_DONE) &&
			(RegVal != ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR));

	/* Write source Address */
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD0,
			(SrcAddr & ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0_LSB_MASK));
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD1,
		(((u64)SrcAddr >> 32U) &
				ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1_MSB_MASK));

	/* Write Destination Address */
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0,
		(u32)(DstAddr & ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0_LSB_MASK));
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1,
			(u32)((DstAddr >> 32U) &
			ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1_MSB_MASK));

	/* Size to be Transferred. Recommended to set both src and dest sizes */
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2, Size);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2, Size);


	/* coherence enable */
	RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3);
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3, RegVal | 0x1U);

	RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3);
		XFsbl_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3, RegVal | 0x1U);

	/* DMA Enable */
	RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_CTRL2);
	RegVal |= ADMA_CH0_ZDMA_CH_CTRL2_EN_MASK;
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL2, RegVal);

	/* Check the status of the transfer by polling on DMA Done */
	do {
		RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_ISR);
		RegVal &= ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK;
	} while (RegVal != ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK);

	/* Clear DMA status */
	RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_ISR);
	RegVal |= ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK;
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_ISR, ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK);

	/* Read the channel status for errors */
	RegVal = XFsbl_In32(ADMA_CH0_ZDMA_CH_STATUS);
	if (RegVal == ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR) {
		Status = XFSBL_FAILURE;
	}

	/* Clear the TOTAL BYTE COUNT register to avoid the BYTE_CNT_OVRFLW
	*interupt from being set
	*/
	XFsbl_Out32(ADMA_CH0_ZDMA_CH_CTRL0_TOTAL_BYTE_COUNT,0x00000000U);

	return Status;

}
