/******************************************************************************
*
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmcdma.c
* @addtogroup mcdma_v1_3
* @{
*
* This file contains the implementation of the interface functions for MCDMA
* driver. Refer to the header file xmcdma.h for more detailed information.
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0    adk    18/07/17 Initial version.
* 1.1    rsp    20/02/18 Fix unused variable warning.
*                        Remove TimeOut variable.CR-979061
* 1.3    rsp    14/02/19 Populate HasRxLength value from config.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xmcdma.h"

/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes an MCDMA core. This function must be called
* prior to using an MCDMA core. Initialization of an MCDMA includes setting
* up the instance data and ensuring the hardware is in a quiescent state and
* resets all the hardware configurations.
*
* @param	InstancePtr is a pointer to the XMcdma instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XMcdma instance.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
s32 XMcDma_CfgInitialize(XMcdma *InstancePtr, XMcdma_Config *CfgPtr)
{
	u32 TimeOut;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	XMcDma_Initialize(InstancePtr, CfgPtr);

	XMcDma_Reset(InstancePtr);

	/* At the initialization time, hardware should finish reset quickly */
	TimeOut = XMCDMA_LOOP_COUNT;

	while (TimeOut) {

		if(XMcdma_ResetIsDone(InstancePtr)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut) {
		xil_printf("Failed reset in initialize\r\n");

		/* Need system hard reset to recover */
		InstancePtr->IsReady = 0;
		return XST_DMA_ERROR;
	}

	if (InstancePtr->Config.IsTxCacheCoherent)
		XMcdma_SetARCache(InstancePtr, XMCDMA_AXCACHE);
	if (InstancePtr->Config.IsRxCacheCoherent)
		XMcdma_SetAWCache(InstancePtr, XMCDMA_AXCACHE);

	InstancePtr->IsReady = 1;

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function initializes an MCDMA core. This function must be called
* prior to using an MCDMA core. Initialization of an MCDMA includes setting
* up the instance data and updating the driver filed with a proper values.
*
* @param	InstancePtr is a pointer to the XMcdma instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XMcdma instance.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		When user calls this function he should ensure the hardware
*		is in a quiescent state by resetting all the hardware
*		Configurations.
*
******************************************************************************/
s32 XMcDma_Initialize(XMcdma *InstancePtr, XMcdma_Config *CfgPtr)
{
	int i;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	memset(InstancePtr, 0, sizeof(XMcdma));

	InstancePtr->Config.BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
	InstancePtr->Config.AddrWidth = CfgPtr->AddrWidth;
	InstancePtr->Config.Has_SingleIntr = CfgPtr->Has_SingleIntr;
	InstancePtr->Config.HasMM2S = CfgPtr->HasMM2S;
	InstancePtr->Config.HasS2MM = CfgPtr->HasS2MM;
	InstancePtr->Config.MaxTransferlen = CfgPtr->MaxTransferlen;
	InstancePtr->Config.HasStsCntrlStrm = CfgPtr->HasStsCntrlStrm;
	InstancePtr->Config.HasRxLength = CfgPtr->HasRxLength;
	InstancePtr->Config.IsTxCacheCoherent = CfgPtr->IsTxCacheCoherent;
	InstancePtr->Config.IsRxCacheCoherent = CfgPtr->IsRxCacheCoherent;

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.HasMM2S) {
		InstancePtr->Config.HasMM2SDRE = CfgPtr->HasMM2SDRE;
		InstancePtr->Config.TxNumChannels = CfgPtr->TxNumChannels;
		for (i = 1; i <= InstancePtr->Config.TxNumChannels; i++) {
			InstancePtr->Tx_Chan[i].ChanBase = CfgPtr->BaseAddress;
			InstancePtr->Tx_Chan[i].Chan_id = i;
			InstancePtr->Tx_Chan[i].Has_Txdre = CfgPtr->HasMM2SDRE;
			InstancePtr->Tx_Chan[i].TxDataWidth =
				((unsigned int)CfgPtr->MM2SDataWidth >> 3);
			InstancePtr->Tx_Chan[i].HasStsCntrlStrm = CfgPtr->HasStsCntrlStrm;
			InstancePtr->Tx_Chan[i].MaxTransferLen =
					MAX_TRANSFER_LEN(CfgPtr->MaxTransferlen - 1);
			InstancePtr->Tx_Chan[i].IsRxChan = 0;
			if (InstancePtr->Config.AddrWidth > 32)
				InstancePtr->Tx_Chan[i].ext_addr = 1;
		}
	}

	if (InstancePtr->Config.HasS2MM) {
		InstancePtr->Config.HasS2MMDRE = CfgPtr->HasS2MMDRE;
		InstancePtr->Config.RxNumChannels = CfgPtr->RxNumChannels;
		for (i = 1; i <= InstancePtr->Config.RxNumChannels; i++) {
			InstancePtr->Rx_Chan[i].ChanBase = CfgPtr->BaseAddress +
							XMCDMA_RX_OFFSET;
			InstancePtr->Rx_Chan[i].Has_Rxdre = CfgPtr->HasS2MMDRE;
			InstancePtr->Rx_Chan[i].HasStsCntrlStrm = CfgPtr->HasStsCntrlStrm;
			InstancePtr->Rx_Chan[i].HasRxLength = CfgPtr->HasRxLength;
			InstancePtr->Rx_Chan[i].RxDataWidth =
				((unsigned int)CfgPtr->S2MMDataWidth >> 3);
			InstancePtr->Rx_Chan[i].Chan_id = i;

			InstancePtr->Rx_Chan[i].MaxTransferLen =
				   MAX_TRANSFER_LEN(CfgPtr->MaxTransferlen - 1);

			InstancePtr->Rx_Chan[i].IsRxChan = 1;
			if (InstancePtr->Config.AddrWidth > 32)
				InstancePtr->Rx_Chan[i].ext_addr = 1;
		}
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function resets the MCDMA core.
*
* @param	InstancePtr is a pointer to the XMcdma instance.
*
* @return	None.
*
* @note		This function resets all the configurations made previously.
*
*****************************************************************************/
void XMcDma_Reset(XMcdma *InstancePtr)
{
	u32 RegBase;
	int Chan_id;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Config.HasMM2S)
		RegBase = InstancePtr->Config.BaseAddress;
	else
		RegBase = InstancePtr->Config.BaseAddress + XMCDMA_RX_OFFSET;

	/* Reset the device - In MCDMA Resetting TX or RX will reset the
	 * Entire MCDMA Engine
	 */
	XMcdma_WriteReg(RegBase, XMCDMA_CCR_OFFSET, XMCDMA_CCR_RESET_MASK);

	if (InstancePtr->Config.HasMM2S) {
		for (Chan_id = 1; Chan_id <= InstancePtr->Config.TxNumChannels;
		     Chan_id++) {
			InstancePtr->Tx_Chan[Chan_id].ChanState = XMCDMA_CHAN_IDLE;
		}
	}

	if (InstancePtr->Config.HasS2MM) {
		for (Chan_id = 1; Chan_id <= InstancePtr->Config.RxNumChannels;
		     Chan_id++) {
			InstancePtr->Rx_Chan[Chan_id].ChanState = XMCDMA_CHAN_IDLE;
		}
	}
}

/*****************************************************************************/
/**
 * This function sets the AWCACHE field of the SG interface
 * With the user specified value
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Value is the AWCACHE value to be written.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XMcdma_SetSGAWCache(XMcdma *InstancePtr, u8 Value)
{
	u32 Val;


	if (Value > 0xF) {
		xil_printf("Invalid AWCACHE Value\n\r");
		return;
	}

	Val = XMcdma_ReadReg(InstancePtr->Config.BaseAddress, XMCDMA_SGCACHE_OFFSET);

	Val &= ~XMCDMA_SGAWCACHE_MASK;
	Val |= (Value << XMCDMA_SGAWCACHE_SHIFT);

	XMcdma_WriteReg(InstancePtr->Config.BaseAddress, XMCDMA_SGCACHE_OFFSET, Val);
}

/*****************************************************************************/
/**
 * This function sets the ARCACHE field of the SG interface
 * With the user specified value
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Value is the ARCACHE value to be written.
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XMcdma_SetSGARCache(XMcdma *InstancePtr, u8 Value)
{
	u32 Val;


	if (Value > 0xF) {
		xil_printf("Invalid ARCACHE Value\n\r");
		return;
	}

	Val = XMcdma_ReadReg(InstancePtr->Config.BaseAddress, XMCDMA_SGCACHE_OFFSET);

	Val = (Val & ~XMCDMA_SGARCACHE_MASK) | Value;

	XMcdma_WriteReg(InstancePtr->Config.BaseAddress, XMCDMA_SGCACHE_OFFSET, Val);
}

/*****************************************************************************/
/**
*
* Check whether reset is done
*
* @param	InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
*
* @return
* 		- 1 if reset is done.
*		- 0 if reset is not done
*
* @note		None
*
******************************************************************************/
u32 XMcdma_ResetIsDone(XMcdma *InstancePtr)
{
	u32 RegisterValue;
	XMcdma_ChanCtrl *Chan;
	u32 Chan_id = 1;

	/* Resetting One side of MCDMA will reset the other side as well */
	/* Check transmit channel */
	if (InstancePtr->Config.HasMM2S) {
		Chan = XMcdma_GetMcdmaTxChan(InstancePtr, Chan_id);
		RegisterValue = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_CCR_OFFSET);

		/* Reset is done when the reset bit is low */
		if (RegisterValue & XMCDMA_CCR_RESET_MASK)
			return 0;
	}

	/* Check receive channel */
	if (InstancePtr->Config.HasS2MM) {
		Chan = XMcdma_GetMcdmaRxChan(InstancePtr, Chan_id);
		RegisterValue = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_CCR_OFFSET);

		/* Reset is done when the reset bit is low */
		if (RegisterValue & XMCDMA_CCR_RESET_MASK)
			return 0;
	}

	return 1;
}

/*****************************************************************************/
/**
*
* Start the DMA engine.
*
* @param	Chan is the Channel instance to be worked on
*
* @return
*		- XST_SUCCESS for success.
*		- XST_FAILURE if starting the hardware fails.
*
* @note		None
*
******************************************************************************/
u32 XMcdma_Start(XMcdma_ChanCtrl *Chan)
{
	u32 RegBase;
	u32 StatReg;
	u32 TimeOut;

	RegBase = Chan->ChanBase;

	XMcdma_WriteReg(RegBase, XMCDMA_CCR_OFFSET,
				XMcdma_ReadReg(RegBase, XMCDMA_CCR_OFFSET)
				| XMCDMA_CCR_RUNSTOP_MASK);

	/* At the initialization time, hardware should finish reset quickly */
	TimeOut = XMCDMA_LOOP_COUNT;

	while (TimeOut) {

		StatReg = XMcdma_ReadReg(RegBase, XMCDMA_CSR_OFFSET);
		if(!(StatReg & XMCDMA_CSR_HALTED_MASK)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut) {
		xil_printf("Failed to start the DMA\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Stops the DMA engine.
*
* @param	Chan is the Channel instance to be worked on
*
* @return
*		- XST_SUCCESS for success.
*		- XST_FAILURE if hardware fails stop.
*
* @note		None
*
******************************************************************************/
u32 XMcdma_ChanHwStop(XMcdma_ChanCtrl *Chan)
{
	u32 RegBase;
	u32 CtrlReg;
	u32 TimeOut;
	u32 Offset;

	RegBase = Chan->ChanBase;
	Offset = (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET;

	XMcdma_WriteReg(RegBase, XMCDMA_CR_OFFSET + Offset,
				XMcdma_ReadReg(RegBase, XMCDMA_CR_OFFSET + Offset)
				& ~XMCDMA_CCR_RUNSTOP_MASK);

	/* At the initialization time, hardware should finish reset quickly */
	TimeOut = XMCDMA_LOOP_COUNT;

	while (TimeOut) {

		CtrlReg = XMcdma_ReadReg(RegBase, XMCDMA_CR_OFFSET + Offset);
		if(!(CtrlReg & XMCDMA_CCR_RUNSTOP_MASK)) {
			break;
		}

		TimeOut -= 1;

	}

	if (!TimeOut) {
		xil_printf("Failed to stop the DMA Channel\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Enables a particular channel.
*
* @param	Chan is the Channel instance to be worked on
*
* @return	none
*
* @note		None
*
******************************************************************************/
void XMcdma_EnableCh(XMcdma_ChanCtrl *Chan)
{
	u32 Reg;

	Reg = XMcdma_ReadReg((Chan)->ChanBase, XMCDMA_CHEN_OFFSET);
	Reg |= (1 << (Chan->Chan_id -1));
	XMcdma_WriteReg(Chan->ChanBase, XMCDMA_CHEN_OFFSET, Reg);
}

/*****************************************************************************/
/**
*
* Gets the last serviced s2mm channel number.
*
* @param	InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
*
* @return
* 		- Channel id that last serviced.
*
* @note		None
*
******************************************************************************/
u16 XMcdma_GetChanServiced(XMcdma *InstancePtr)
{
	u16 ServReg;
	u16 Chan_id = 0;

	ServReg = XMcdma_ReadReg(InstancePtr->Config.BaseAddress,
				 XMCDMA_RX_OFFSET + XMCDMA_CHSER_OFFSET);
	while(ServReg >>= 1)
		Chan_id++;

	Chan_id += 1;

	return Chan_id;
}

/*****************************************************************************/
/**
*
* Gets the last serviced mm2s channel number.
*
* @param	InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
*
* @return
* 		- Channel id that last serviced.
*
* @note		None
*
******************************************************************************/
u16 XMcdma_GetTxChanServiced(XMcdma *InstancePtr)
{
	u16 ServReg;
	u16 Chan_id = 0;

	ServReg = XMcdma_ReadReg(InstancePtr->Config.BaseAddress,
				 XMCDMA_CHSER_OFFSET);
	while(ServReg >>= 1)
		Chan_id++;

	Chan_id += 1;

	return Chan_id;
}

u32 XMCdma_GetChan_Weight(XMcdma_ChanCtrl *Chan)
{
	u32 Weight, Chanid;

	if (Chan->IsRxChan) {
		xil_printf("Invalid Channel\n\r");
		return XST_FAILURE;
	}

	if (Chan->Chan_id > 8) {
		Weight = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_TX_WRR_REG1_OFFSET);
		Chanid = (Chan->Chan_id - 1) - 8;
	} else {
		Weight = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_TX_WRR_REG_OFFSET);
		Chanid = Chan->Chan_id - 1;
	}

	Weight &= XMCDMA_TX_WRRCH_MASK(Chanid);
	Weight =  Weight >> XMCDMA_TX_WRRCH_SHIFT(Chanid);

	return Weight;
}

u32 XMCdma_SetChan_Weight(XMcdma_ChanCtrl *Chan, u8 Weight)
{
	u32 Chanid;
	u32 Val;

	if (Chan->IsRxChan) {
		xil_printf("Invalid Channel\n\r");
		return XST_FAILURE;
	}

	if (Weight == 0 || Weight > 0xF) {
		xil_printf("Invalid Weight to Configure\n\r");
		return XST_FAILURE;
	}

	if (Chan->Chan_id > 8) {
		Val = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_TX_WRR_REG1_OFFSET);
		Chanid = (Chan->Chan_id - 1) - 8;
	} else {
		Val = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_TX_WRR_REG_OFFSET);
		Chanid = Chan->Chan_id - 1;
	}

	Val &= ~XMCDMA_TX_WRRCH_MASK(Chanid);
	Val |=  Weight << XMCDMA_TX_WRRCH_SHIFT(Chanid);

	if (Chan->Chan_id > 8)
		XMcdma_WriteReg(Chan->ChanBase, XMCDMA_TX_WRR_REG1_OFFSET, Val);
	else
		XMcdma_WriteReg(Chan->ChanBase, XMCDMA_TX_WRR_REG_OFFSET, Val);

	return XST_SUCCESS;
}

u32 XMCdma_GetChan_PktDoneCnt(XMcdma_ChanCtrl *Chan)
{
	u32 Offset;
	u32 PktCnt;

	Offset = (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET;

	if (Chan->IsRxChan)
		PktCnt = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_RX_PKTCNT_STAT_OFFSET + Offset);
	else
		PktCnt = XMcdma_ReadReg(Chan->ChanBase, XMCDMA_TX_PKTCNT_STAT_OFFSET + Offset);

	return PktCnt;
}

/*****************************************************************************/
/**
 * Dump the registers for a channel.
 *
 * @param	Chan is the MCDMA Channel to be worked on.
 *
 * @return	None
 *
 *****************************************************************************/
void XMcdma_DumpChanRegs(XMcdma_ChanCtrl *Chan) {
	u32 RegBase = Chan->ChanBase;
	u32 Offset;

	Offset = (Chan->Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET;

	xil_printf("Dump registers %x:\r\n", (unsigned int)RegBase);
	xil_printf("Control REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_CR_OFFSET + Offset));
	xil_printf("Status REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_SR_OFFSET + Offset));

	xil_printf("Cur BD REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_CDESC_OFFSET + Offset));
	xil_printf("Cur BD MSB REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_CDESC_MSB_OFFSET + Offset));
	xil_printf("Tail BD REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_TDESC_OFFSET + Offset));
	xil_printf("Tail BD MSB  REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_TDESC_MSB_OFFSET + Offset));

	xil_printf("Packet Drop REG: %08x\r\n",
		(unsigned int)XMcdma_ReadReg(RegBase, XMCDMA_PKTDROP_OFFSET + Offset));
	xil_printf("\r\n");
}
