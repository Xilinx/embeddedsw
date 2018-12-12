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
* @file xpmcfw_cdo.h
*
* This is the file which contains psu3 functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/16/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XILCDO_H
#define XILCDO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xstatus.h"
#include "xpmcfw_config.h"
#include "xpmcfw_debug.h"
#include "xsecure.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u32 SrcAddr;
	u32 Len;
	u32 ActualLen;
	u32 ProcLen;
	u32 TotalSections;
	u32 Offset;
	u32* CdoBuf;
	XStatus (*CdoCopy) (u32, u64, u32, u32);
#ifdef XPMCFW_SECURE
	XSecure_Partition *SecureCdo;
#endif
}XilCdo_Prtn;

/***************** Macros (Inline Functions) Definitions *********************/
#define XilCdo_Printf XPmcFw_Printf
#define XILCDO_WORD_LEN		0x4U
/** 64KB of Buffer Length */
#define XILCDO_MAX_PARTITION_LENGTH	(0x10000U)
#define XILCDO_PMCRAM_ENDADDR	(XPMCFW_PMCRAM_BASEADDR + \
				XILCDO_MAX_PARTITION_LENGTH)
#define XILCDO_SMAP_DEST_ADDR	(0xFFFFFFFFFFFFFFFFL)
#define DMA_WRITE_LEN_INDEX	0x1U
#define DMA_WRITE_OFFSET_INDEX	0x2U
#define DMA_WRITE_SRCADDR_INDEX	0x3U
#define NPI_WRITE_ATTRB_INDEX	0x1U
#define NPI_WRITE_LEN_INDEX		0x2U
#define NPI_WRITE_SRCADDR_INDEX	0x3U
#define DMA_XFER_SRCADDR_HIGH_INDEX 0x0U
#define DMA_XFER_SRCADDR_LOW_INDEX  0x1U
#define DMA_XFER_DESTADDR_HIGH_INDEX  0x2U
#define DMA_XFER_DESTADDR_LOW_INDEX   0x3U
#define DMA_XFER_LEN_INDEX		 0x4U
#define DMA_XFER_FLAGS_INDEX	 0x5U
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#define HDR_IDN_WRD		(0x584C4E58U)
#define HDR_LEN			(0x5U)
#define SECTION_LEN		(0x3U)
#define SECTION_PLL		(0x201U)
#define SECTION_MIO		(0x202U)
#define SECTION_PERIPHERAL	(0x203U)
#define SECTION_DDR		(0x204U)
#define SECTION_GT		(0x205U)
#define SECTION_SECURE		(0x206U)
#define SECTION_NPI		(0x207U)
#define SECTION_CFI		(0x208U)
#define SECTION_RDBACK (0x209U)
#define SECTION_DMA_XFER_CFI	(0x20AU)

/*General Commands */
#define CMD_MASK_POLL		(0x101U)
#define CMD_MASK_WRITE		(0x102U)
#define CMD_WRITE		(0x103U)
#define CMD_DELAY		(0x104U)
#define CMD_DMA_WRITE		(0x105U)
#define CMD_MASK_POLL64		(0x106U)
#define CMD_MASK_WRITE64	(0x107U)
#define CMD_WRITE64		(0x108U)
#define CMD_DMA_XFER		(0x109U)
#define CMD_END			(0xFFFFFFFFU)

#define CMD_MASK_POLL_ARGS	(0x4U)
#define CMD_MASK_WRITE_ARGS	(0x3U)
#define CMD_WRITE_ARGS		(0x2U)
#define CMD_DELAY_ARGS		(0x1U)
#define CMD_MASK_POLL64_ARGS	(0x5U)
#define CMD_MASK_WRITE64_ARGS	(0x4U)
#define CMD_WRITE64_ARGS	(0x3U)
#define CMD_DMA_WRITE_ARGS	(0x3U)
#define CMD_NPI_WRITE_ARGS	(0x3U)
#define CMD_NPI_SEQ_ARGS	(0x2U)
#define CMD_NPI_PRECFG_ARGS	(0x2U)
#define CMD_NPI_SHUTDN_ARGS	(0x2U)
#define CMD_CFI_SEQ_ARGS	(0x1U)
#define CMD_DMA_XFER_ARGS	(0x6U)

/* NPI Commands */
#define CMD_NPI_SEQ		(0x201U)
#define CMD_NPI_PRECFG		(0x202U)
#define CMD_NPI_WRITE		(0x203U)
#define CMD_NPI_SHUTDN		(0x204U)

/* CFI Commands */
#define CMD_CFI_SETCRC32	(0x301U)

/* Error Codes */
#define XILCDO_ERR_HDR_CHKSUM	(0x1U)
#define XILCDO_ERR_CMD		(0x2U)
#define XILCDO_ERR_MASK_TIMEOUT	(0x3U)
#define XILCDO_ERR_HDR_ID	(0x4U)

XStatus XilCdo_CheckHdr(u32 *ConfigHdr);
XStatus XilCdo_ProcessCdo(u32 *ConfigData);
XStatus XilCdo_ExecuteCmds(u32* ConfigData, u32 TotalSections);
XStatus XilCdo_CopyCdoBuf(void);
XStatus XilCdo_DmaWrite(u32 CmdArgs[10U]);
XStatus XilCdo_DmaXfer(u32 CmdArgs[10U]);
XStatus XilCdo_DmaTransfer(u64 SrcAddr, u64 DestAddr, u32 Len);
XStatus XilCdo_NpiPreCfg(u32 CmdArgs[10U]);
#ifdef __cplusplus
}
#endif

#endif  /* XILCDO_H */
