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
* @file xplmi_generic.c
*
* This is the file which contains general commands.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_generic.h"
#include "xstatus.h"
#include "xplmi_util.h"
#include "xplmi_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

static int XPlmi_Reserved(XPlmi_Cmd * Cmd)
{
	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides 32 bit mask poll command execution
 *  Command payload parameters are
 *	* Address
 *	* Mask
 *	* Expected Value
 *	* Timeout in ms
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS on successful poll
 *****************************************************************************/
static int XPlmi_MaskPoll(XPlmi_Cmd * Cmd)
{
	u32 Addr = Cmd->Payload[0];
	u32 Mask = Cmd->Payload[1];
	u32 ExpectedValue = Cmd->Payload[2];
	u32 TimeOutInMs = Cmd->Payload[3];
	int Status;

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Mask 0x%0x, ExpVal: 0x%0x, Timeout: %d\n\r",
		__func__, Addr, Mask, ExpectedValue, TimeOutInMs);

	Status = XPlmi_UtilPoll(Addr, Mask, ExpectedValue, TimeOutInMs);
	if (Status != XST_SUCCESS)
	{
		Status = XPLMI_ERR_MASKPOLL;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides 32 bit mask write command execution
 *  Command payload parameters are
 *	* Address
 *	* Mask
 *	* Value
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *****************************************************************************/
static int XPlmi_MaskWrite(XPlmi_Cmd * Cmd)
{
	u32 Addr = Cmd->Payload[0];
	u32 Mask = Cmd->Payload[1];
	u32 Value = Cmd->Payload[2];

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Mask 0x%0x, Value: 0x%0x\n\r",
		__func__, Addr, Mask, Value);

	XPlmi_UtilRMW(Addr, Mask, Value);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides 32 bit Write command execution
 *  Command payload parameters are
 *	* Address
 *	* Value
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *****************************************************************************/
static int XPlmi_Write(XPlmi_Cmd * Cmd)
{
	u32 Addr = Cmd->Payload[0];
	u32 Value = Cmd->Payload[1];

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Val: 0x%0x\n\r",
		__func__, Addr, Value);

	Xil_Out32(Addr, Value);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides delay command execution
 *  Command payload parameters are
 *	* Delay in ms
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *****************************************************************************/
static int XPlmi_Delay(XPlmi_Cmd * Cmd)
{
	u32 Delay;

	/** TODO implement timer based delay */
	Delay = Cmd->Payload[0] * 1000;

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Delay: %d\n\r",
		__func__, Delay);

	XPlmi_UtilWait(Delay);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides DMA write command execution
 *  Command payload parameters are
 *	* High Dest Addr
 *	* Low Dest Addr
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of DMA Xfer API
 *****************************************************************************/
static int XPlmi_DmaWrite(XPlmi_Cmd * Cmd)
{
	int Status;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Flags;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U)
	{
		/** store the destination address in resume data */
		Cmd->ResumeData[0] = Cmd->Payload[0];
		Cmd->ResumeData[1] = Cmd->Payload[1];
		SrcAddr = (u64 )(UINTPTR) &Cmd->Payload[2];
		Len -= 2U;
	} else {
		SrcAddr = (u64 )(UINTPTR) &Cmd->Payload[0];
	}

	DestAddr = (u64) Cmd->ResumeData[0];
	DestAddr = ((u64 )Cmd->ResumeData[1] |
			(DestAddr<<32));
	DestAddr += (Cmd->ProcessedLen * 4U);

	/** Set DMA flags to DMA0 and INCR */
	Flags = XPLMI_PMCDMA_0;

	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, Flags);
	if(Status != XST_SUCCESS)
	{
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Failed\n\r");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides 64 bit mask poll command execution
 *  Command payload parameters are
 *	* High Address
 *	* Low Address
 *	* Mask
 *	* Expected Value
 *	* Timeout in ms
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS on successful poll
 *****************************************************************************/
static int XPlmi_MaskPoll64(XPlmi_Cmd * Cmd)
{
	int Status;
	u64 Addr = (((u64)Cmd->Payload[0] << 32U) | Cmd->Payload[1U]);
	u32 Mask = Cmd->Payload[2U];
	u32 ExpectedValue = Cmd->Payload[3U];
	u32 TimeOutInMs = Cmd->Payload[4U];

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Mask 0x%0x, ExpVal: 0x%0x, Timeout: %d\n\r",
		__func__, (u32)Addr, Mask, ExpectedValue, TimeOutInMs);

	Status = XPlmi_UtilPoll64(Addr, Mask, ExpectedValue, TimeOutInMs);
	if (Status != XST_SUCCESS)
	{
		Status = XPLMI_ERR_MASKPOLL64;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides 64 bit mask write command execution
 *  Command payload parameters are
 *	* High Address
 *	* Low Address
 *	* Mask
 *	* Value
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *****************************************************************************/
static int XPlmi_MaskWrite64(XPlmi_Cmd * Cmd)
{
	u64 Addr = (((u64)Cmd->Payload[0] << 32U) | Cmd->Payload[1U]);
	u32 Mask = Cmd->Payload[2U];
	u32 Value = Cmd->Payload[3U];
	u32 ReadVal;

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Mask 0x%0x, Val: 0x%0x\n\r",
		__func__, (u32)Addr, Mask, Value);

	/**
	 * Read the Register value
	 */
	ReadVal = XPlmi_In64(Addr);
	ReadVal = (ReadVal & (~Mask)) | (Mask & Value);

	XPlmi_Out64(Addr, ReadVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides 64 bit write command execution
 *  Command payload parameters are
 *	* High Address
 *	* Low Address
 *	* Value
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *****************************************************************************/
static int XPlmi_Write64(XPlmi_Cmd * Cmd)
{
	u64 Addr = (((u64)Cmd->Payload[0] << 32U) | Cmd->Payload[1U]);
	u32 Value = Cmd->Payload[2U];

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Val: 0x%0x\n\r",
		__func__, Addr, Value);

	XPlmi_Out64(Addr, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function provides DMA Xfer command execution
 *  Command payload parameters are
 *	* High Src Addr
 *	* Low Src Addr
 *	* High Dest Addr
 *	* Low Dest Addr
 *	* Params - AXI burst type (Fixed/INCR)
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of DMA Xfer API
 *****************************************************************************/
static int XPlmi_DmaXfer(XPlmi_Cmd * Cmd)
{
	int Status;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len;
	u32 Flags;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U)
	{
		/** store the command fields in resume data */
		Cmd->ResumeData[0] = Cmd->Payload[0];
		Cmd->ResumeData[1] = Cmd->Payload[1];
		Cmd->ResumeData[2] = Cmd->Payload[2];
		Cmd->ResumeData[3] = Cmd->Payload[3];
		Cmd->ResumeData[4] = Cmd->Payload[4];
		Cmd->ResumeData[5] = Cmd->Payload[5];
	}

	SrcAddr = (u64 )Cmd->ResumeData[0];
	SrcAddr = ((u64 )Cmd->ResumeData[1] |
		    (SrcAddr << 32));
	DestAddr = (u64) Cmd->ResumeData[2];
	DestAddr = ((u64 )Cmd->ResumeData[3] |
			(DestAddr<<32));
	DestAddr += (Cmd->ProcessedLen * 4U);
	Len = Cmd->ResumeData[4];

	/** Set DMA flags to DMA0 */
	Flags = Cmd->ResumeData[5] | XPLMI_PMCDMA_0;

	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, Flags);
	if(Status != XST_SUCCESS)
	{
		XPlmi_Printf(DEBUG_GENERAL, "DMA XFER Failed\n\r");
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief contains the array of PLM generic commands
 *
 *****************************************************************************/
static XPlmi_ModuleCmd XPlmi_GenericCmds[] =
{
	XPLMI_MODULE_COMMAND(XPlmi_Reserved),
	XPLMI_MODULE_COMMAND(XPlmi_MaskPoll),
	XPLMI_MODULE_COMMAND(XPlmi_MaskWrite),
	XPLMI_MODULE_COMMAND(XPlmi_Write),
	XPLMI_MODULE_COMMAND(XPlmi_Delay),
	XPLMI_MODULE_COMMAND(XPlmi_DmaWrite),
	XPLMI_MODULE_COMMAND(XPlmi_MaskPoll64),
	XPLMI_MODULE_COMMAND(XPlmi_MaskWrite64),
	XPLMI_MODULE_COMMAND(XPlmi_Write64),
	XPLMI_MODULE_COMMAND(XPlmi_DmaXfer)
};

/*****************************************************************************/
/**
 * @brief Contains the module ID and PLM generic commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Generic =
{
	XPLMI_MODULE_GENERIC_ID,
	XPlmi_GenericCmds,
	sizeof XPlmi_GenericCmds / sizeof *XPlmi_GenericCmds,
};

/*****************************************************************************/
/**
 * @brief This function registers the PLM generic commands to the PLMI
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
void XPlmi_GenericInit(void)
{
	XPlmi_ModuleRegister(&XPlmi_Generic);
}
