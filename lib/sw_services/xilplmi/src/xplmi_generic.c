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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
#include "xcfupmc.h"
#include "sleep.h"
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
 *	* Timeout in us
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
	u32 TimeOutInUs = Cmd->Payload[3];
	int Status;
	/** HACK  **/
	TimeOutInUs = 1000000;

	Status = XPlmi_UtilPoll(Addr, Mask, ExpectedValue, TimeOutInUs);
	if (Status != XST_SUCCESS)
	{
		XPlmi_Printf(DEBUG_GENERAL,
		"%s: Addr: 0x%0x,  Mask: 0x%0x, ExpVal: 0x%0x, "
		"Timeout: %d ...ERROR\r\n",  __func__,
		Addr, Mask, ExpectedValue, TimeOutInUs);
		/** HACK -- proceed even if we fail */
		Status = XST_SUCCESS;
	}
	else {
		XPlmi_Printf(DEBUG_INFO,
		"%s: Addr: 0x%0x,  Mask: 0x%0x, ExpVal: 0x%0x, "
		"Timeout: %d ...Done\r\n",  __func__,
		Addr, Mask, ExpectedValue, TimeOutInUs);
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
 *	* Delay in us
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *****************************************************************************/
static int XPlmi_Delay(XPlmi_Cmd * Cmd)
{
	u32 Delay;

	Delay = Cmd->Payload[0];
	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Delay: %d\n\r",
		__func__, Delay);

	usleep(Delay);
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
	u32 DestOffset = 0U;

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
		/** decrement the destination offset by CMD params */
		DestOffset = 2U;
	}

	DestAddr = (u64) Cmd->ResumeData[0];
	DestAddr = ((u64 )Cmd->ResumeData[1] |
			(DestAddr<<32));
	DestAddr += ((Cmd->ProcessedLen - DestOffset) * 4U);

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
 *	* Timeout in us
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
	u32 TimeOutInUs = Cmd->Payload[4U];

	XPlmi_Printf(DEBUG_DETAILED,
	    "%s, Addr: 0x%0x%08x,  Mask 0x%0x, ExpVal: 0x%0x, Timeout: %d\n\r",
	    __func__, (u32)(Addr>>32), (u32)Addr, Mask, ExpectedValue, TimeOutInUs);

	Status = XPlmi_UtilPoll64(Addr, Mask, ExpectedValue, TimeOutInUs);
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
		"%s, Addr: 0x%0x%08x,  Mask 0x%0x, Val: 0x%0x\n\r",
		__func__, (u32)(Addr>>32), (u32)Addr, Mask, Value);

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
		"%s, Addr: 0x%0x%08x,  Val: 0x%0x\n\r",
		__func__, (u32)(Addr>>32), (u32)Addr, Value);

	XPlmi_Out64(Addr, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @param	Npi Src Address
 *
 * @param   Destination Address
 *
 * @param	Len is number of words to be read
 *
 * @return	returns success or the error codes described in xilcdo.h
 *
 ******************************************************************************/
int XPlmi_NpiRead(u64 SrcAddr, u64 DestAddr, u32 Len)
{
	u64 RegVal;
	u32 Count = 0U;
	int Status;
	u32 Offset;
	u32 ProcWords=0U;

	/** For NPI READ command, the source address needs to be
	 * 16 byte aligned. Use XPlmi_Out64 till the destination address
	 *becomes 16 byte aligned. */
	while((ProcWords<Len)&&(((SrcAddr+Count)&(0xFU)) != 0U))
	{
		RegVal = XPlmi_In64(SrcAddr + Count);
		XPlmi_Out64(DestAddr + (ProcWords<<2U), RegVal);
		Count+=4U;
		++ProcWords;
	}

	Status = XPlmi_DmaXfr((u64)(SrcAddr+Count),
		(u64)(DestAddr+Count), (Len - ProcWords)&(~(0x3U)), XPLMI_PMCDMA_0);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	/** For NPI_READ command, Offset variable should
	 *  be updated with the unaligned bytes. */
	ProcWords = ProcWords + ((Len - ProcWords)&(~(0x3U)));
	Offset = (ProcWords<<2U);

	while(ProcWords < Len)
	{
		xil_printf("\n %0x %0x \n\r", SrcAddr+Offset, DestAddr+Offset);
		RegVal = XPlmi_In64(SrcAddr + Offset);
		XPlmi_Out64(DestAddr + Offset, RegVal);
		Offset += 4U;
		ProcWords++;
	}

END:
	return Status;
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

	if(DestAddr == XPLMI_SBI_DEST_ADDR)
	{
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK);

		Status = XPlmi_DmaSbiXfer(SrcAddr, Len, XPLMI_PMCDMA_1);
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK,0U);
	}
	else if((Flags & XPLMI_DMA_SRC_NPI) == XPLMI_DMA_SRC_NPI)
	{
		Status = XPlmi_NpiRead(SrcAddr,DestAddr,Len);
	}
	else
	{
		Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, Flags);
	}

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
 * @brief This function provides INIT_SEQ command execution
 *  Command payload parameters are
 *	* DATA
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status
 *****************************************************************************/
static int XPlmi_InitSeq(XPlmi_Cmd * Cmd)
{
	/** for MISRA C */
	(void )Cmd;

	return XPLMI_ERR_CMD_NOT_SUPPORTED;
}

/*****************************************************************************/
/**
 * @brief This function provides CFI READ command execution
 *  Command payload parameters are
 *	* Params - SMPA/JTAG/DDR
 *	* High Dest Addr
 *	* Low Dest Addr
 *	* Read Length in number of words to be read from CFU
 *	* DATA (CFU READ Packets)
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status
 *****************************************************************************/
int XPlmi_CfiRead(XPlmi_Cmd * Cmd)
{
	int Status;
	u32 SrcType = Cmd->Payload[0];
	u64 DestAddrHigh;
	u32 DestAddrLow;
	u32 Len = Cmd->Payload[3];
	u32 SrcAddr = CFU_FDRO_ADDR;
	u64 DestAddr;

	XPlmi_SetMaxOutCmds(1U);
	if(SrcType == XPLMI_READBK_INTF_TYPE_DDR)
	{
		DestAddrHigh = Cmd->Payload[1];
		DestAddrLow =  Cmd->Payload[2];
		DestAddr = (DestAddrHigh<<32U) | DestAddrLow;

		Status = XPlmi_DmaXfr(CFU_FDRO_ADDR, DestAddr, Len,
			XPLMI_PMCDMA_1 | XPLMI_SRC_CH_AXI_FIXED |
						XPLMI_DMA_SRC_NONBLK);
	}
	else
	{
		if(SrcType == XPLMI_READBK_INTF_TYPE_JTAG)
		{
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
                               SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
                               XPLMI_SBI_CTRL_INTERFACE_JTAG);
		}
		else if(SrcType == XPLMI_READBK_INTF_TYPE_SMAP)
		{
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
                               SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
                               XPLMI_SBI_CTRL_INTERFACE_SMAP);
		}
		else
		{
			/** Do Nothing */
		}

		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK);

		Status = XPlmi_DmaSbiXfer(CFU_FDRO_ADDR, Len, XPLMI_PMCDMA_1
				 | XPLMI_SRC_CH_AXI_FIXED | XPLMI_DMA_SRC_NONBLK);
	}

	if(Status != XST_SUCCESS)
	{
		goto END;
	}
	XPlmi_SetMaxOutCmds(8);
	SrcAddr = (u32)(&Cmd->Payload[4]);

	Status = XPlmi_DmaXfr(SrcAddr, (u64)CFU_STREAM_ADDR,
				Cmd->PayloadLen-4, XPLMI_PMCDMA_0);

	if(SrcType == XPLMI_READBK_INTF_TYPE_DDR)
	{
		XPlmi_WaitForNonBlkDma();
		goto END;
	}
	else
	{
		XPlmi_WaitForNonBlkSrcDma();
	}

	Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK,
			0x1000, XPLMI_TIME_OUT_DEFAULT);

	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	if(SrcType == XPLMI_READBK_INTF_TYPE_SMAP)
	{
		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			       SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK,
				0x800000, XPLMI_TIME_OUT_DEFAULT);
	}
	else
	{
		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
                               SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
                                0x80000000, XPLMI_TIME_OUT_DEFAULT);
	}

	if(Status != XST_SUCCESS)
        {
                goto END;
        }

END:
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides SET command execution
 *  Command payload parameters are
 *	* High Dest Addr
 *	* Low Dest Addr
 *	* Length (Length of words to set to value)
 *	* Value
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status
 *****************************************************************************/
static int XPlmi_Set(XPlmi_Cmd * Cmd)
{
	/** for MISRA C */
	(void )Cmd;

	return XPLMI_ERR_CMD_NOT_SUPPORTED;
}

/*****************************************************************************/
/**
 * @brief This function provides DMA key hole write command execution
 *  Command payload parameters are
 *	* High Dest Addr
 *	* Low Dest Addr
 *	* Keyhole Size
 *	* DATA...
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of DMA Xfer API
 *****************************************************************************/
int XPlmi_DmaWriteKeyHole(XPlmi_Cmd * Cmd)
{
	int Status;
	u64 DestAddr;
	u32 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Flags;
	u32 Keyholesize;
	u32 ChunkLen;
	u32 Count;
	u32* CfiDataPtr;
	u64 BaseAddr;
	u32 DestOffset;
	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U)
	{
		/** store the destination address in resume data */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		Cmd->ResumeData[1U] = Cmd->Payload[1U];
		Cmd->ResumeData[2U] = Cmd->Payload[2U];
		Keyholesize = Cmd->Payload[2U];
		SrcAddr = (u32 )(UINTPTR) &Cmd->Payload[3U];
		Len -= 3U;
		Cmd->ResumeData[3U] =0U;
		DestOffset = 0U;
	} else {
		SrcAddr = (u32)(UINTPTR) &Cmd->Payload[0U];
		Keyholesize = Cmd->ResumeData[2U];
		DestOffset = 3U;
	}

	DestAddr = (u64) Cmd->ResumeData[0U];
	DestAddr = ((u64 )Cmd->ResumeData[1U] |
			(DestAddr<<32U));
	BaseAddr = DestAddr;
	DestAddr = ((Cmd->ProcessedLen-Cmd->ResumeData[3U]-DestOffset)*4U)%Keyholesize + BaseAddr;
	/** Set DMA flags to DMA0 and FIXED */
	Flags = XPLMI_PMCDMA_0 | XPLMI_DST_CH_AXI_FIXED;
	if(Cmd->ProcessedLen != 0U)
	{
			for(Count=0; Count < Cmd->ResumeData[3U]; ++Count)
			{
				XPlmi_Out32(DestAddr&0xFFFFFFFFU,Cmd->ResumeData[Count+4U]);
				DestAddr = ((Cmd->ProcessedLen-Cmd->ResumeData[3U]-DestOffset+Count+1)*4U)%Keyholesize + BaseAddr;
			}

			while((Count>0U)&&(Count<4U))
			{
				CfiDataPtr = (u32*)SrcAddr;
				XPlmi_Out32((DestAddr&0xFFFFFFFFU),*CfiDataPtr);
				SrcAddr += 4U;
				--Len;
				++Count;
				DestAddr = ((Cmd->ProcessedLen-Cmd->ResumeData[3U]-DestOffset+Count)*4U)%Keyholesize + BaseAddr;
			}
			Cmd->ResumeData[3U] = 0U;
	}

	ChunkLen = Len - (Len % 4U);

	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, ChunkLen, Flags);
	if(Status != XST_SUCCESS)
	{
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}
	Cmd->ResumeData[3U]= Len%4U;
	CfiDataPtr = (u32*)(SrcAddr + ChunkLen*4U);
	Count=0;

	while(ChunkLen < Len)
	{
		Cmd->ResumeData[Count+4U] =  *CfiDataPtr;
		CfiDataPtr++;
		ChunkLen++;
		++Count;
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
	XPLMI_MODULE_COMMAND(XPlmi_DmaXfer),
	XPLMI_MODULE_COMMAND(XPlmi_InitSeq),
	XPLMI_MODULE_COMMAND(XPlmi_CfiRead),
	XPLMI_MODULE_COMMAND(XPlmi_Set),
	XPLMI_MODULE_COMMAND(XPlmi_DmaWriteKeyHole),
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
