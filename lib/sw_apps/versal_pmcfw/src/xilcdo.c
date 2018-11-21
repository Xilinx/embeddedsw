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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
 * @file xilcdo.c
 *
 * This is the file which contains psu3 functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  kc   03/20/2017 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilcdo.h"
#include "xpmcfw_misc.h"
#include "xilcdo_npi.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
/************************** Variable Definitions *****************************/
XilCdo_Prtn XilCdoPrtnInst; /** Instance to copy CDO partition to buffer */
u32 BlkDma = 0U; /** BlkDma denotes whether Dma0/Dma1 is used as blocking Dma*/
/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to configuration data
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_MaskPoll(u32 CmdArgs[10U])
{
	u32 Addr = CmdArgs[0U];
	u32 Mask = CmdArgs[1U];
	u32 Value = CmdArgs[2U];
	u32 TimeOutInMs = CmdArgs[3U];
	u32 ReadValue;
	u32 TimeOut = TimeOutInMs*10000; //TBD: remove multiplication

	XilCdo_Printf(DEBUG_DETAILED, "CMD: Mask Poll\n\r");
	XilCdo_Printf(DEBUG_DETAILED, "Addr: 0x%08x\n\r", Addr);
	XilCdo_Printf(DEBUG_DETAILED, "Mask: 0x%08x\n\r", Mask);
	XilCdo_Printf(DEBUG_DETAILED, "Value: 0x%08x\n\r", Value);
	XilCdo_Printf(DEBUG_DETAILED, "TimeOutInMs: 0x%08x\n\r", TimeOutInMs);

	/**
	 * Read the Register value
	 */
	ReadValue = Xil_In32(Addr);

	/**
	 * Loop while the Mask is not set or we timeout
	 */
	while(((ReadValue & Mask) != Value) && (TimeOut > 0U)){
		/**
		 * Latch up the value again
		 */
		ReadValue = Xil_In32(Addr);

		/**
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}
	CmdArgs[9U] = CMD_MASK_POLL_ARGS;
	return ((TimeOut == 0U) ? XILCDO_ERR_MASK_TIMEOUT : XST_SUCCESS);
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to configuration data
 *
 * @return	XST_SUCCESS
 *
 ******************************************************************************/
XStatus XilCdo_MaskWrite(u32 CmdArgs[10U])
{
	u32 Addr = CmdArgs[0U];
	u32 Mask = CmdArgs[1U];
	u32 Value = CmdArgs[2U];
	u32 ReadVal;

	XilCdo_Printf(DEBUG_DETAILED, "CMD: Mask Write; ");
	XilCdo_Printf(DEBUG_DETAILED, "Addr: 0x%08x; ", Addr);
	XilCdo_Printf(DEBUG_DETAILED, "Mask: 0x%08x; ", Mask);
	XilCdo_Printf(DEBUG_DETAILED, "Value: 0x%08x;", Value);
	XilCdo_Printf(DEBUG_DETAILED, "\n\r");

	ReadVal = Xil_In32(Addr);
	ReadVal = (ReadVal & (~Mask)) | (Mask & Value);

	Xil_Out32(Addr, ReadVal);
	CmdArgs[9U] = CMD_MASK_WRITE_ARGS;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to configuration data
 *
 * @return	XST_SUCCESS
 *
 ******************************************************************************/
XStatus XilCdo_Write(u32 CmdArgs[10U])
{
	u32 Addr = CmdArgs[0U];
	u32 Value = CmdArgs[1U];
	XilCdo_Printf(DEBUG_DETAILED, "CMD: Write; ");
	XilCdo_Printf(DEBUG_DETAILED, "Addr: 0x%08x; ", Addr);
	XilCdo_Printf(DEBUG_DETAILED, "Value: 0x%08x;", Value);
	XilCdo_Printf(DEBUG_DETAILED, "\n\r");
	Xil_Out32(Addr, Value);
	CmdArgs[9U] = CMD_WRITE_ARGS;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to configuration data
 *
 * @return	XST_SUCCESS
 *
 ******************************************************************************/
XStatus XilCdo_Delay(u32 CmdArgs[10U])
{
	u32 TimeOutCount = CmdArgs[0U];
	u32 TimeOut = TimeOutCount;
	XilCdo_Printf(DEBUG_DETAILED, "CMD: Delay; ");
	XilCdo_Printf(DEBUG_DETAILED, "Delay: 0x%08x;", TimeOutCount);
	XilCdo_Printf(DEBUG_DETAILED, "\n\r");

	while (TimeOut > 0U) {
		TimeOut--;
	}
	CmdArgs[9U] = CMD_DELAY_ARGS;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to configuration data
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_MaskPoll64(u32 CmdArgs[10U])
{
	u64 Addr = (((u64)CmdArgs[0U] << 32U) | CmdArgs[1U]);
	u32 Mask = CmdArgs[2U];
	u32 Value = CmdArgs[3U];
	u32 ReadValue;
	u32 TimeOut = CmdArgs[4U];

	XilCdo_Printf(DEBUG_DETAILED, "CMD: Mask Poll\n\r");
	XilCdo_Printf(DEBUG_DETAILED, "Hight Addr: 0x%08x: ", CmdArgs[0U]);
	XilCdo_Printf(DEBUG_DETAILED, "Low Addr: 0x%08x: ", CmdArgs[1U]);
	XilCdo_Printf(DEBUG_DETAILED, "Mask: 0x%08x\n\r", Mask);
	XilCdo_Printf(DEBUG_DETAILED, "Value: 0x%08x\n\r", Value);
	XilCdo_Printf(DEBUG_DETAILED, "TimeOut: 0x%08x\n\r", TimeOut);

	/**
	 * Read the Register value
	 */
	ReadValue = XPmcFw_In64(Addr);

	/**
	 * Loop while the Mask is not set or we timeout
	 */
	while(((ReadValue & Mask) != Value) && (TimeOut > 0U)){
		/**
		 * Latch up the value again
		 */
		ReadValue = XPmcFw_In64(Addr);

		/**
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	CmdArgs[9U] = CMD_MASK_POLL64_ARGS;
	return ((TimeOut == 0U) ? XILCDO_ERR_MASK_TIMEOUT : XST_SUCCESS);

}

/*****************************************************************************/
/**
 * @param       CmdArgs is pointer to configuration data
 *
 * @return      XST_SUCCESS
 *
 *******************************************************************************/
XStatus XilCdo_MaskWrite64(u32 CmdArgs[10U])
{
	u64 Addr = (((u64)CmdArgs[0U] << 32U) | CmdArgs[1U]);
	u32 Mask = CmdArgs[2U];
	u32 Value = CmdArgs[3U];
	u32 ReadVal;

	XilCdo_Printf(DEBUG_DETAILED, "CMD: Mask Write; ");
	XilCdo_Printf(DEBUG_DETAILED, "Hight Addr: 0x%08x: ", CmdArgs[0U]);
	XilCdo_Printf(DEBUG_DETAILED, "Low Addr: 0x%08x: ", CmdArgs[1U]);
	XilCdo_Printf(DEBUG_DETAILED, "Mask: 0x%08x; ", Mask);
	XilCdo_Printf(DEBUG_DETAILED, "Value: 0x%08x;", Value);
	XilCdo_Printf(DEBUG_DETAILED, "\n\r");

	ReadVal = XPmcFw_In64(Addr);
	ReadVal = (ReadVal & (~Mask)) | (Mask & Value);

	XPmcFw_Out64(Addr, ReadVal);

	CmdArgs[9U] = CMD_MASK_WRITE64_ARGS;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @param       CmdArgs is pointer to configuration data
 *
 * @return      XST_SUCCESS
 *
 *******************************************************************************/
XStatus XilCdo_Write64(u32 CmdArgs[10U])
{
	u64 Addr = (((u64)CmdArgs[0U] << 32U) | CmdArgs[1U]);
	u32 Value = CmdArgs[2U];

	XilCdo_Printf(DEBUG_DETAILED, "CMD: Write; ");
	XilCdo_Printf(DEBUG_DETAILED, "Hight Addr: 0x%08x: ", CmdArgs[0U]);
	XilCdo_Printf(DEBUG_DETAILED, "Low Addr: 0x%08x: ", CmdArgs[1U]);
	XilCdo_Printf(DEBUG_DETAILED, "Value: 0x%08x:", Value);
	XilCdo_Printf(DEBUG_DETAILED, "\n\r");
	XPmcFw_Out64(Addr, Value);

	CmdArgs[9U] = CMD_WRITE64_ARGS;
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
 * @param	CmdData is pointer to Command data
 * @param	ArgNum is no of arguments to the Cmd
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_ProcessCmd(u32 Cmd, u32 *CmdData, u32 *ArgNum)
{
	XStatus Status;
	u32 RemArgs;
	u32 CmdArgs[10U];
	XStatus (*XilCdo_CmdHdlr) (u32[10U]);

	switch (Cmd)
	{
		case CMD_MASK_POLL:
			{
				XilCdo_CmdHdlr = XilCdo_MaskPoll;
				*ArgNum = CMD_MASK_POLL_ARGS;
			}break;

		case CMD_MASK_WRITE:
			{
				XilCdo_CmdHdlr = XilCdo_MaskWrite;
				*ArgNum = CMD_MASK_WRITE_ARGS;
			}break;

		case CMD_WRITE:
			{
				XilCdo_CmdHdlr = XilCdo_Write;
				*ArgNum = CMD_WRITE_ARGS;
			}break;

		case CMD_DELAY:
			{
				XilCdo_CmdHdlr = XilCdo_Delay;
				*ArgNum = CMD_DELAY_ARGS;
			}break;

		case CMD_MASK_POLL64:
			{
				XilCdo_CmdHdlr = XilCdo_MaskPoll64;
				*ArgNum = CMD_MASK_POLL64_ARGS;
			}break;

		case CMD_MASK_WRITE64:
			{
				XilCdo_CmdHdlr = XilCdo_MaskWrite64;
				*ArgNum = CMD_MASK_WRITE64_ARGS;
			}break;

		case CMD_WRITE64:
			{
				XilCdo_CmdHdlr = XilCdo_Write64;
				*ArgNum = CMD_WRITE64_ARGS;
			}break;

		case CMD_NPI_SEQ:
			{
				XilCdo_CmdHdlr = XilCdo_NpiSeq;
				*ArgNum = CMD_NPI_SEQ_ARGS;
			}break;
		case CMD_DMA_WRITE:
			{
				XilCdo_CmdHdlr = XilCdo_DmaWrite;
				*ArgNum = CMD_DMA_WRITE_ARGS;
			}
			break;
		case CMD_NPI_WRITE:
			{
				XilCdo_CmdHdlr = XilCdo_NpiWrite;
				*ArgNum = CMD_NPI_WRITE_ARGS;
			}
			break;
		case CMD_DMA_XFER:
			{
				XilCdo_CmdHdlr = XilCdo_DmaXfer;
				*ArgNum = CMD_DMA_XFER_ARGS;
			}
			break;
		case CMD_NPI_SHUTDN:
			{
				XilCdo_CmdHdlr = XilCdo_NpiShutDown;
				*ArgNum = CMD_NPI_SHUTDN_ARGS;
			}
			break;
		case CMD_NPI_PRECFG:
			{
				*ArgNum = CMD_NPI_PRECFG_ARGS;
				Status = XST_SUCCESS;
				goto END;
			}
			break;
		default:
			{
				Status = XILCDO_ERR_CMD;
				goto END;
			}break;
	}

	if((u32)&CmdData[*ArgNum -1U] >= XILCDO_PMCRAM_ENDADDR)
	{
		RemArgs = (XILCDO_PMCRAM_ENDADDR - (u32)&CmdData[0U])/XILCDO_WORD_LEN;
		memcpy(CmdArgs, &CmdData[0U], RemArgs*XILCDO_WORD_LEN);
		XilCdo_CopyCdoBuf();
		memcpy(&CmdArgs[RemArgs], (void*)XPMCFW_PMCRAM_BASEADDR, (*ArgNum - RemArgs)*XILCDO_WORD_LEN);
		CmdArgs[9U] = XPMCFW_PMCRAM_BASEADDR + (*ArgNum - RemArgs)*XILCDO_WORD_LEN;
	}
	else
	{
		memcpy(CmdArgs, &CmdData[0U], (*ArgNum)*XILCDO_WORD_LEN);
		CmdArgs[9U] = (u32)&CmdData[*ArgNum];
	}
	Status = (*XilCdo_CmdHdlr)(CmdArgs);
	*ArgNum = CmdArgs[9U];
END:
	return Status;
}

/*****************************************************************************/
/**
 * @param	ConfigHdr is pointer to the header data
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_CheckHdr(u32 *ConfigHdr)
{
	u32 CheckSum=0U;
	u32 Index;
	XStatus Status;

	if (ConfigHdr[1] != HDR_IDN_WRD)
	{
		Status = XILCDO_ERR_HDR_ID;
		goto END;
	}

	for (Index=0U;Index<HDR_LEN-1;Index++)
	{
		CheckSum += ConfigHdr[Index];
	}

	/* Invert checksum */
	CheckSum ^= 0xFFFFFFFFU;

	if (CheckSum != ConfigHdr[Index])
	{
#ifdef XPMCFW_CDO_CHKSUM_BYPASS
		Status = XST_SUCCESS;
#else
		XilCdo_Printf(DEBUG_GENERAL,
				"Config Object Checksum Failed\n\r");
		Status = XILCDO_ERR_HDR_CHKSUM;
		goto END;
#endif
	}else{
		Status = XST_SUCCESS;
	}

	XilCdo_Printf(DEBUG_INFO,
			"Config Object Version 0x%08x\n\r", ConfigHdr[2]);
	XilCdo_Printf(DEBUG_INFO,
			"No of Sections 0x%08x\n\r", ConfigHdr[3]);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @param	ConfigDat is pointer to the configuration data
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_ProcessCdo(u32 *ConfigData)
{
	XStatus Status;
	/* Read Hdr */
	Status = XilCdo_CheckHdr(ConfigData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilCdo_ExecuteCmds(ConfigData, ConfigData[3U]);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @param	ConfigData is pointer to the configuration data
 *
 * @param   Len is length of configuration data present
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_ExecuteCmds(u32* ConfigData, u32 TotalSections)
{
	u32 Index = HDR_LEN;
	u32 ArgNum;
	u32 CmdIndex;
	u32 SecIndex;
	u32 TotalCmds;
	u32 CurrCmd;
	XStatus Status;
	u32 XilCdoMaxWords = XILCDO_MAX_PARTITION_LENGTH>>2U;
	for (SecIndex = 0U;SecIndex < TotalSections; SecIndex++)
	{
		/* TODO Print the section details */
		XilCdo_Printf(DEBUG_INFO,
				"---------------------------------------\n\r");
		XilCdo_Printf(DEBUG_INFO,
			"Section ID: 0x%08x\n\r", ConfigData[Index]);
		XilCdo_Printf(DEBUG_INFO,
			"Section Length: 0x%08x\n\r", ConfigData[Index+1]);
		XilCdo_Printf(DEBUG_INFO,
			"Section No of Cmds: 0x%08x\n\r", ConfigData[Index+2]);

		TotalCmds = ConfigData[Index+2U];
		Index += SECTION_LEN;
		if(TotalCmds == 0U)
		{
			Status = XST_SUCCESS;
			continue;
		}
		CmdIndex = 0U;
		/* Check for end of commands */
		while ((ConfigData[Index] != CMD_END) && (CmdIndex < TotalCmds))
		{
			/* Execute Commands */
			CurrCmd = ConfigData[Index++];
			Status = XilCdo_ProcessCmd(CurrCmd, &ConfigData[Index],
						  &ArgNum);
			if (Status != XST_SUCCESS) {
				XilCdo_Printf(DEBUG_GENERAL,"CMD Failed:\
				Section ID: 0x%0x, CMD[0x%0x]: 0x%0x 0x%0x\n\r",
				SecIndex, Index, ConfigData[Index], CurrCmd);
				goto END;
			}
			else
			{
				Index += ArgNum;
				Index %= XilCdoMaxWords;
				if(Index==0U)
				{
					/** If the processed length exceeds max 
					 * threshold for partition length, copy 
					 * the partition*/
					Status = XilCdo_CopyCdoBuf();
					if (XST_SUCCESS != Status)
					{
						goto END;
					}
				}
				++CmdIndex;
			}
		}
		if(CmdIndex < TotalCmds)
		{
			/** This check is for CMD_END */
			++Index;
		}
		/** Current section completed */
	}
	Status = XST_SUCCESS;

	XilCdo_Printf(DEBUG_INFO, "------End of Configuration------ \n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to the argument data
 *
 * @param   ArgNum is pointer to number of arguments
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_DmaWrite(u32 CmdArgs[10U])
{
	XStatus Status;
	u32 DestAddr = CmdArgs[0U];
	u32 Len = CmdArgs[DMA_WRITE_LEN_INDEX];
	u32 SrcAddr = CmdArgs[9U] + CmdArgs[DMA_WRITE_OFFSET_INDEX]
			 * XILCDO_WORD_LEN;

	if(SrcAddr >= XILCDO_PMCRAM_ENDADDR)
	{
		SrcAddr %= XILCDO_MAX_PARTITION_LENGTH;
		SrcAddr += XPMCFW_PMCRAM_BASEADDR;
		XilCdo_CopyCdoBuf();
	}
	CmdArgs[9U] = CMD_DMA_WRITE_ARGS + Len + CmdArgs[DMA_WRITE_OFFSET_INDEX];

	Status = XilCdo_DmaTransfer((u64)SrcAddr, (u64)DestAddr, Len);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @param	SrcAddr is starting address of source data
 *
 * @param   DestAddr is starting address of destination data
 *
 * @param	Len is number of words to be transferred
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
 XStatus XilCdo_DmaTransfer(u64 SrcAddr, u64 DestAddr, u32 Len)
 {
	XStatus Status;
	u32 Offset = 0U;
	u32 CurLen;
	u32 ProcBytes = 0U;
	u32 Flags = XPMCFW_PMCDMA_0;
	do
	{
		/** CurLen is initially assigend to max bytes of DMA that can
		 * be carried over from the source address */
		CurLen = XILCDO_PMCRAM_ENDADDR - SrcAddr;
		CurLen = (CurLen>>2U);
		if(CurLen > Len)
		{
			CurLen = Len;
		}

		if(CurLen == 0U)
		{
			break;
		}
		Status = XPmcFw_DmaXfr((u64)SrcAddr, (u64)(DestAddr+Offset),
			CurLen, Flags);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
		ProcBytes = (CurLen<<2U);
		Offset += ProcBytes;
		Len -= CurLen;
		if(Len > 0U)
		{
			Status = XilCdo_CopyCdoBuf();
			if (XST_SUCCESS != Status)
			{
				goto END;
			}
			SrcAddr = (u32)XilCdoPrtnInst.CdoBuf;
		}
	}while(Len > 0U);
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to the arguments data
 *
 *
 * @return	returns the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_DmaXfer(u32 CmdArgs[10U])
{
	XStatus Status;
	u64 SrcAddrHigh = CmdArgs[DMA_XFER_SRCADDR_HIGH_INDEX];
	u32 SrcAddrLow =  CmdArgs[DMA_XFER_SRCADDR_LOW_INDEX];
	u64 DestAddrHigh = CmdArgs[DMA_XFER_DESTADDR_HIGH_INDEX];
	u32 DestAddrLow =  CmdArgs[DMA_XFER_DESTADDR_LOW_INDEX];
	u32 Len = CmdArgs[DMA_XFER_LEN_INDEX];
	u32 Flags = CmdArgs[DMA_XFER_FLAGS_INDEX];
	u64 SrcAddr = (SrcAddrHigh<<32U) | SrcAddrLow;
	u64 DestAddr = (DestAddrHigh<<32U) | DestAddrLow;
	u32 DmaFlags = Flags;
	CmdArgs[9U] = CMD_DMA_XFER_ARGS;

	if(SrcAddr == 0L)
	{
		SrcAddr = (u32)&CmdArgs[CMD_DMA_XFER_ARGS];
		CmdArgs[9U] += Len;

		if(SrcAddr == XILCDO_PMCRAM_ENDADDR)
		{
			SrcAddr = XPMCFW_PMCRAM_BASEADDR;
		}
	}
	if((Flags & XPMCFW_DMA_DST_NONBLK) || (Flags & XPMCFW_DMA_SRC_NONBLK))
	{
		if(BlkDma == 0U)
		{
			DmaFlags |= XPMCFW_PMCDMA_0;
			DmaFlags &= ~XPMCFW_PMCDMA_1;
			BlkDma = 1U;
		}
		else
		{
			DmaFlags |= XPMCFW_PMCDMA_1;
			DmaFlags &= ~XPMCFW_PMCDMA_0;
		}
	}
	else if(BlkDma == 0U)
	{
		DmaFlags |= XPMCFW_PMCDMA_0;
		DmaFlags &= ~XPMCFW_PMCDMA_1;
	}
	else
	{
		DmaFlags |= XPMCFW_PMCDMA_1;
		DmaFlags &= ~XPMCFW_PMCDMA_0;
	}

	if(DestAddr == XILCDO_SMAP_DEST_ADDR)
	{
		Status = XPmcFw_DmaSbiXfer(SrcAddr, Len, DmaFlags);
	}
	else
	{
		Status = XPmcFw_DmaXfr(SrcAddr, DestAddr, Len, DmaFlags);
	}

	if(Status != XST_SUCCESS)
	{
		goto END;
	}
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to copy chunk of the partition to buffer.
 *
 * @param	void
 *
 * @return
 *		- XST_SUCCESS on Success
 *		- ErrorCode as returned by Copy callback
 *
 * @note
 *
 *****************************************************************************/
XStatus XilCdo_CopyCdoBuf(void)
{
	XStatus Status;
	u32 CurLen;

	if(XilCdoPrtnInst.CdoCopy == NULL)
	{
		Status = XST_SUCCESS;
		goto END;
	}
	if(XilCdoPrtnInst.Offset >= XilCdoPrtnInst.ActualLen)
	{
		Status = XST_SUCCESS;
		goto END;
	}

#ifdef XPMCFW_SECURE
	if ((XilCdoPrtnInst.SecureCdo->IsAuthenticated == TRUE) ||
			(XilCdoPrtnInst.SecureCdo->IsEncrypted == TRUE)) {

		XilCdoPrtnInst.SecureCdo->ChunkBuffer = (u8 *)XilCdoPrtnInst.CdoBuf;
		XilCdoPrtnInst.SecureCdo->ChunkSize = XILCDO_MAX_PARTITION_LENGTH;
		Status = XPmcFw_ProcessSecurePrtn(XilCdoPrtnInst.SecureCdo);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto END;
		}
		goto END;
	}
#endif

	CurLen  = XilCdoPrtnInst.Len - XilCdoPrtnInst.Offset;

	if(CurLen > XILCDO_MAX_PARTITION_LENGTH)
	{
		CurLen = XILCDO_MAX_PARTITION_LENGTH;
	}
	Status = XilCdoPrtnInst.CdoCopy(XilCdoPrtnInst.SrcAddr +
		XilCdoPrtnInst.Offset, (u32)XilCdoPrtnInst.CdoBuf, CurLen, 0U);
	XilCdoPrtnInst.Offset += CurLen;
END:
	return Status;
}
