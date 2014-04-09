/******************************************************************************
*
* (c) Copyright 2012-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file pcap.h
*
* This file contains the interface for intiializing and accessing the PCAP
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a ecm	02/10/10 Initial release
* 2.00a mb  16/08/12 Added the macros and function prototypes
* </pre>
*
* @note
*
******************************************************************************/
#ifndef ___PCAP_H___
#define ___PCAP_H___


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xdevcfg.h"

/************************** Function Prototypes ******************************/


/* Multiboot register offset mask */
#define PCAP_MBOOT_REG_REBOOT_OFFSET_MASK	0x1FFF
#define PCAP_CTRL_PCFG_AES_FUSE_EFUSE_MASK	0x1000

#define PCAP_LAST_TRANSFER 1
#define MAX_COUNT 1000000000
#define LVL_PL_PS 0x0000000F
#define LVL_PS_PL 0x0000000A

/* Fix for #672779 */
#define FSBL_XDCFG_IXR_ERROR_FLAGS_MASK		(XDCFG_IXR_AXI_WERR_MASK | \
						XDCFG_IXR_AXI_RTO_MASK |  \
						XDCFG_IXR_AXI_RERR_MASK | \
						XDCFG_IXR_RX_FIFO_OV_MASK | \
						XDCFG_IXR_DMA_CMD_ERR_MASK |\
						XDCFG_IXR_DMA_Q_OV_MASK |   \
						XDCFG_IXR_P2D_LEN_ERR_MASK |\
						XDCFG_IXR_PCFG_HMAC_ERR_MASK)

int InitPcap(void);
void PcapDumpRegisters(void);
u32 ClearPcapStatus(void);
void FabricInit(void);
int XDcfgPollDone(u32 MaskValue, u32 MaxCount);
u32 PcapLoadPartition(u32 *SourceData, u32 *DestinationData, u32 SourceLength,
		 	u32 DestinationLength, u32 Flags);
u32 PcapDataTransfer(u32 *SourceData, u32 *DestinationData, u32 SourceLength,
 			u32 DestinationLength, u32 Flags);
/************************** Variable Definitions *****************************/
#ifdef __cplusplus
}
#endif


#endif /* ___PCAP_H___ */

