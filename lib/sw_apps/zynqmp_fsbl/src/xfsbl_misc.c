/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
#define XFSBL_BASE_FILE_NAME_LEN 8
#define XFSBL_NUM_DIGITS_IN_FILE_NAME 4

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if 0
__inline void XFsbl_Printf(u32 DebugType,char *Format, ...)
{
#ifdef STDOUT_BASEADDRESS
	va_list Args;
	if (((DebugType) & XFsblDbgCurrentTypes) != 0)
	{
		va_start(Args, Format);
		xil_printf(Format, Args);
		va_end(Args);
	}
#endif
}
#endif
/************************** Function Prototypes ******************************/
static void XFsbl_UndefHandler (void);
#ifndef XFSBL_A53
static void XFsbl_SvcHandler (void);
static void XFsbl_PreFetchAbortHandler (void);
#endif
static void XFsbl_DataAbortHandler (void);
static void XFsbl_IrqHandler (void);
static void XFsbl_FiqHandler (void);

/**
 * functions from xfsbl_main.c
 */
extern void XFsbl_ErrorLockDown(u32 ErrorStatus);

/************************** Variable Definitions *****************************/

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
	u32 Index=0U;

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
	while( *Str1Ptr )
	{
		Str1Ptr++;
	}

	while( *Str2Ptr )
	{
		*Str1Ptr = *Str2Ptr;
		Str1Ptr++; Str2Ptr++;
	}

	*Str1Ptr = '\0';
	return --Str1Ptr;
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
void XFsbl_MemSet(void *SrcPtr, u8 Char, u32 Len)
{
	u8 *UsPtr = SrcPtr;

	while (Len != 0)
	{
		*UsPtr = Char;
		UsPtr++;
		Len--;
	}
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
void *XFsbl_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len)
{
	u8 *Dst = DestPtr;
	const u8 *Src = SrcPtr;

	/* Loop and copy.  */
	while (Len != 0)
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
int XFsbl_MemCmp(const void *Str1Ptr, const void *Str2Ptr, u32 Count)
{
	const u8 *S1Ptr = (const u8 *)Str1Ptr;
	const u8 *S2Ptr = (const u8 *)Str2Ptr;
	int Status = 0;

	while (Count--)
	{
	  if (*S1Ptr != *S2Ptr)
	  {
	    Status = ((*S1Ptr < *S2Ptr) ? -1 : 1);
	    break;
    }
	  S1Ptr++;
	  S2Ptr++;
  }

	return Status;
}

/*****************************************************************************/
/**
 *
 *
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
u32 XFsbl_Htonl(u32 Value1)
{
    u32 Value2 = 0;

        Value2 |= (Value1 & 0xFF000000) >> 24;
        Value2 |= (Value1 & 0x00FF0000) >> 8;
        Value2 |= (Value1 & 0x0000FF00) << 8;
        Value2 |= (Value1 & 0x000000FF) << 24;

        return Value2;
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
void XFsbl_MakeSdFileName(char *XFsbl_SdEmmcFileName,
		u32 MultibootReg, u32 DeviceFlags)
{

	u32 Index;
	u32 Value;
	u32 MultiBootNum = MultibootReg;

	if (0x0U == MultiBootNum)
	{
		/* SD file name is BOOT.BIN when Multiboot register value is 0 */
		if ((DeviceFlags == XFSBL_SD0_BOOT_MODE) ||
				(DeviceFlags == XFSBL_EMMC_BOOT_MODE)) {
			(void)XFsbl_Strcpy((char *)XFsbl_SdEmmcFileName, "BOOT.BIN");
		}
		else {
			/* For XFSBL_SD1_BOOT_MODE or XFSBL_SD1_LS_BOOT_MODE */
			(void)XFsbl_Strcpy((char *)XFsbl_SdEmmcFileName, "1:/BOOT.BIN");
		}
	}
	else
	{
		/* set default SD file name as BOOT0000.BIN */
		if ((DeviceFlags == XFSBL_SD0_BOOT_MODE) ||
				(DeviceFlags == XFSBL_EMMC_BOOT_MODE)) {
			(void)XFsbl_Strcpy((char *)XFsbl_SdEmmcFileName, "BOOT0000.BIN");
		}
		else {
			/* For XFSBL_SD1_BOOT_MODE or XFSBL_SD1_LS_BOOT_MODE */
			(void)XFsbl_Strcpy((char *)XFsbl_SdEmmcFileName, "1:/BOOT0000.BIN");
		}

		/* Update file name (to BOOTXXXX.BIN) based on Multiboot register value */
		for(Index = XFSBL_BASE_FILE_NAME_LEN - 1;
				Index >= XFSBL_BASE_FILE_NAME_LEN - XFSBL_NUM_DIGITS_IN_FILE_NAME;
				Index--)
		{
			Value = MultiBootNum % 10;
			MultiBootNum = MultiBootNum / 10;
			XFsbl_SdEmmcFileName[Index] += (s8)Value;
			if (MultiBootNum == 0)
			{
				break;
			}
		}
	}

	XFsbl_Printf(DEBUG_INFO,
			"File name is %s\r\n",XFsbl_SdEmmcFileName);
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

#ifndef XFSBL_A53
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
#ifdef XFSBL_A53
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
	if (XFSBL_PLATFORM != XFSBL_PLATFORM_QEMU)
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
