/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpmcfw_util.c
*
* This file which contains the code which interfaces with the CRP
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_util.h"
#include "xpmcfw_debug.h"
#include "xpmcfw_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

void XPmcFw_UtilRMW(u32 RegAddr, u32 Mask, u32 Value)
{
	u32 l_Val;

	l_Val = Xil_In32(RegAddr);
	l_Val = (l_Val & (~Mask)) | (Mask & Value);

	Xil_Out32(RegAddr, l_Val);
}

XStatus XPmcFw_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutCount)
{
	u32 l_RegValue;
	u32 TimeOut = TimeOutCount;
	/**
	 * Read the Register value
	 */
	l_RegValue = Xil_In32(RegAddr);
	/**
	 * Loop while the MAsk is not set or we timeout
	 */
	while(((l_RegValue & Mask) != Mask) && (TimeOut > 0U)){
		/**
		 * Latch up the Register value again
		 */
		l_RegValue = Xil_In32(RegAddr);
		/**
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);
}

XStatus XPmcFw_UtilPollForZero(u32 RegAddr, u32 Mask, u32 TimeOutCount)
{
	u32 l_RegValue;
	u32 TimeOut = TimeOutCount;

	l_RegValue = Xil_In32(RegAddr);
	/**
	 * Loop until all bits defined by mask are cleared
	 * or we time out
	 */
	while (((l_RegValue & Mask) != 0U) && (TimeOut > 0U)) {
		/**
		 * Latch up the reg value again
		 */
		l_RegValue = Xil_In32(RegAddr);
		/**
		 * Decrement the timeout count
		 */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);
}

void XPmcFw_UtilWait(u32 TimeOutCount)
{
	u32 TimeOut = TimeOutCount;
	while (TimeOut > 0U) {
		TimeOut--;
	}
}
/*****************************************************************************/
/**
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param	Mask is the bit field to be updated
 * @param	Value is value to be updated
 * @param       TimeOutCount is delay time in ms
 *
 * @return      None
 *
 ******************************************************************************/
XStatus XPmcFw_UtilPollForMask64(u32 HighAddr, u32 LowAddr, u32 Mask,
				u32 TimeOutInMs)
{
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	u32 ReadValue;
    u32 TimeOut = TimeOutInMs*10000; //TBD: remove multiplication

    /**
	 * Read the Register value
	 */
    ReadValue = XPmcFw_In64(Addr);

    /**
	 * Loop while the Mask is not set or we timeout
	 */
    while(((ReadValue & Mask) != Mask) && (TimeOut > 0U)){
		/**
		 * Latch up the value again
		 */
		ReadValue = XPmcFw_In64(Addr);

		/**
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);

}

/*****************************************************************************/
/**
 * @param       HighAddr is higher 32-bits of 64-bit address
 * @param	LowAaddr is lower 32-bits of 64-bit address
 * @param       Mask is the bit field to be updated
 * @param       Value is value to be updated
 *
 * @return      None
 *
 *******************************************************************************/
void XPmcFw_UtilRMW64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 Value)
{
	u64 Addr = (((u64)HighAddr << 32) | LowAddr);
    u32 ReadVal;

    ReadVal = XPmcFw_In64(Addr);
    ReadVal = (ReadVal & (~Mask)) | (Mask & Value);

    XPmcFw_Out64(Addr, ReadVal);
}

/*****************************************************************************/
/**
 * @param       HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param       Value is value to be updated
 *
 * @return      None
 *
 *******************************************************************************/
void XPmcFw_Write64(u32 HighAddr, u32 LowAddr, u32 Value)
{
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	XPmcFw_Out64(Addr, Value);
}

/****************************************************************************/
/**
* This function is used to print the entire array in bytes as specified by the
* debug type
*
* @param DebugType printing of the array will happen as defined
*	 by the debug type
* @param Buf pointer to the  buffer to be printed
* @param Len length of the bytes to be printed
* @param Str pointer to the data that is printed along the data
*
* @return None
*
* @note
*
*****************************************************************************/
void XPmcFw_PrintArray (u32 DebugType, const u8 Buf[], u32 Len, const char *Str)
{
	u32 Index;

	if ((DebugType & XPmcFwDbgCurrentTypes) != 0U)
	{
		XPmcFw_Printf(DebugType, "%s START\r\n", Str);
		for (Index=0U;Index<Len;Index++)
		{
			XPmcFw_Printf(DebugType, "%02lx ", Buf[Index]);
			if (((Index+1U)%16U) == 0U){
				XPmcFw_Printf(DebugType, "\r\n");
			}
		}
		XPmcFw_Printf(DebugType, "\r\n%s END\r\n", Str);
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
char *XPmcFw_Strcpy(char *DestPtr, const char *SrcPtr)
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
char * XPmcFw_Strcat(char* Str1Ptr, const char* Str2Ptr)
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
 * @param	Str1Ptr First string to be compared
 * @param	Str2Ptr Second string to be compared
 * @return	0 if both strings are same,
 *		-1/1 if first non matching character has
 *		lower/greater value in Str1Ptr
 *
 ******************************************************************************/
s32 XPmcFw_Strcmp( const char* Str1Ptr, const char* Str2Ptr)
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
void* XPmcFw_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len)
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
