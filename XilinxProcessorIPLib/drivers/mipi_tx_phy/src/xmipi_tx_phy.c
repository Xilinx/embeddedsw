/******************************************************************************
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmipi_tx_phy.c
* @addtogroup mipi_tx_phy Overview
* @{
*
* This file implements the functions to control and get info from the mipi_tx_phy.
*
* <pre>
* MODIFICATION HISTORY:
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 pg 16/02/24 Initial release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xmipi_tx_phy.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/************************** Macros Definitions *****************************/
#define XMIPI_TX_PHY_SOFTRESET_TIMEOUT 	5000UL

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
* Initialize the XMipi_Tx_Phy instance provided by the caller based on the
* given Config structure.
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param 	CfgPtr is the device configuration structure containing
*  		information about a specific Mipi_Tx_Phy instance.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
* @return
* 		- XST_SUCCESS Initialization was successful.
*
* @note		None.
*****************************************************************************/
u32 XMipi_Tx_Phy_CfgInitialize(XMipi_Tx_Phy *InstancePtr, XMipi_Tx_Phy_Config *CfgPtr,
						UINTPTR EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;

	InstancePtr->Config.BaseAddr = EffectiveAddr;

	InstancePtr->IsReady = (XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Configure the registers of the Mipi_Tx_Phy instance
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param 	Handle to one of the registers to be configured
* @param	Value to be set for the particular Handle of the Mipi_Tx_Phy instance
*
* @return
* 		- XST_SUCCESS on successful register update.
* 		- XST_FAILURE If incorrect handle was passed
*
* @note		There is a limit on the minimum and maximum values of
*		the HS Timeout register.
*
*****************************************************************************/
u32 XMipi_Tx_Phy_Configure(XMipi_Tx_Phy *InstancePtr, u8 Handle, u32 Value)
{
	u32 Status = XST_SUCCESS;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Handle <= XMIPI_TX_PHY_HANDLE_MAX);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);

	/* Based on the Handle, write Value to the specific register */
	switch (Handle) {

		case XMIPI_TX_PHY_HANDLE_INIT_TIMER:
			XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
					XMIPI_TX_PHY_INIT_TIMER_REG_OFFSET, Value);
			break;

		case XMIPI_TX_PHY_HANDLE_HSTIMEOUT:
			XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
					XMIPI_TX_PHY_HSTIMEOUT_REG_OFFSET,
					Value);
			break;

		case XMIPI_TX_PHY_HANDLE_ESCTIMEOUT:
			Xil_AssertNonvoid(Value >= XMIPI_TX_PHY_HS_TIMEOUT_MIN_VALUE);
			Xil_AssertNonvoid(Value <= XMIPI_TX_PHY_HS_TIMEOUT_MAX_VALUE);

			XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
					XMIPI_TX_PHY_ESCTIMEOUT_REG_OFFSET,
					Value);
			break;

		case XMIPI_TX_PHY_HANDLE_CLKLANE:
		case XMIPI_TX_PHY_HANDLE_DLANE0:
		case XMIPI_TX_PHY_HANDLE_DLANE1:
		case XMIPI_TX_PHY_HANDLE_DLANE2:
		case XMIPI_TX_PHY_HANDLE_DLANE3:
			Status = XST_FAILURE;
			break;
		default:
			break;
	}

	return Status;
}


u32 XMipi_Tx_Phy_Prog_Seq_Data(XMipi_Tx_Phy *InstancePtr, XMipi_Tx_Phy_ProgSeq bitpos, u32 Value)
{

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(!InstancePtr->Config.IsDphy);
	Xil_AssertNonvoid(Value > 7);

	u32 rdata;

	XMipi_Tx_Phy_Dis_Prog_Seq_Ctrl(InstancePtr);
	if (XMipi_Tx_Phy_Get_Prog_Seq_Ctrl(InstancePtr))
	{
		return XST_FAILURE;
	}

	if (bitpos <= PRG_SEQ_SYM_9)
	{
		rdata = XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
							XMIPI_TX_PHY_PROG_SEQ_DATA0_OFFSET);

		XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
							XMIPI_TX_PHY_PROG_SEQ_DATA0_OFFSET,
							rdata | (Value << (bitpos * 3)));
	} else
	{
		rdata = XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
							XMIPI_TX_PHY_PROG_SEQ_DATA1_OFFSET);

		XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
							XMIPI_TX_PHY_PROG_SEQ_DATA1_OFFSET,
							rdata | (Value << (bitpos * 3)));
	}

	return XST_SUCCESS;

}

u32 XMipi_Tx_Phy_Get_Seq_Data(XMipi_Tx_Phy *InstancePtr, XMipi_Tx_Phy_ProgSeq bitpos)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(!InstancePtr->Config.IsDphy);

	if (bitpos <= PRG_SEQ_SYM_9)
	{
		return (XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
							XMIPI_TX_PHY_PROG_SEQ_DATA0_OFFSET));
	}
	else
	{
		return (XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
							XMIPI_TX_PHY_PROG_SEQ_DATA1_OFFSET));
	}
}

u32 XMipi_Tx_Phy_En_Prog_Seq_Ctrl(XMipi_Tx_Phy *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(!InstancePtr->Config.IsDphy);

	XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
								XMIPI_TX_PHY_PROG_SEQ_CTRL_OFFSET,
								XMIPI_TX_PHY_PROG_SEQ_EN_MASK);

	return XST_SUCCESS;
}

u32 XMipi_Tx_Phy_Dis_Prog_Seq_Ctrl(XMipi_Tx_Phy *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(!InstancePtr->Config.IsDphy);

	XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr,
								XMIPI_TX_PHY_PROG_SEQ_CTRL_OFFSET,
								~XMIPI_TX_PHY_PROG_SEQ_EN_MASK);

	return XST_SUCCESS;
}

u32 XMipi_Tx_Phy_Get_Prog_Seq_Ctrl(XMipi_Tx_Phy *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(!InstancePtr->Config.IsDphy);

	return (XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
								XMIPI_TX_PHY_PROG_SEQ_CTRL_OFFSET));

}

/****************************************************************************/
/**
* Get if register interface is present from the config structure for specified
* Mipi_Tx_Phy instance.
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
*
* @return
* 		- 1 if register interface is present
* 		- 0 if register interface is absent
*
* @note		None.
*****************************************************************************/
u8 XMipi_Tx_Phy_GetRegIntfcPresent(XMipi_Tx_Phy *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Config.IsRegisterPresent);
}

/****************************************************************************/
/**
* Get information stored in the Mipi_Tx_Phy instance based on the handle passed
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param 	Handle to one of the registers to be configured
*
* @return 	The value stored in the corresponding register
*
* @note		None.
*****************************************************************************/
u32 XMipi_Tx_Phy_GetInfo(XMipi_Tx_Phy *InstancePtr, u8 Handle)
{
	u32 RegVal = 0;
	UINTPTR RegAddr;
	u32 MaxLanesPresent;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Handle <= XMIPI_TX_PHY_HANDLE_MAX)
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);

	RegAddr = (InstancePtr)->Config.BaseAddr;
	MaxLanesPresent = InstancePtr->Config.MaxLanesPresent;

	/* Based on Handle, return value from the corresponding registers */
	switch (Handle) {
		case XMIPI_TX_PHY_HANDLE_INIT_TIMER:
			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
					       XMIPI_TX_PHY_INIT_TIMER_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_HSTIMEOUT:
			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
							XMIPI_TX_PHY_HSTIMEOUT_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_ESCTIMEOUT:
			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
						XMIPI_TX_PHY_ESCTIMEOUT_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_CLKLANE:
			Xil_AssertNonvoid(InstancePtr->Config.IsDphy);

			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
					       XMIPI_TX_PHY_CLSTATUS_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_DLANE0:
			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
					       XMIPI_TX_PHY_DL0STATUS_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_DLANE1:
			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
					       XMIPI_TX_PHY_DL1STATUS_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_DLANE2:
			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
					       XMIPI_TX_PHY_DL2STATUS_REG_OFFSET);
			break;
		case XMIPI_TX_PHY_HANDLE_DLANE3:
			Xil_AssertNonvoid(InstancePtr->Config.IsDphy);

			RegVal = XMipi_Tx_Phy_ReadReg(RegAddr,
					       XMIPI_TX_PHY_DL3STATUS_REG_OFFSET);
			break;
		default:
			break;
	}

	return RegVal;
}

/****************************************************************************/
/**
* This is used to do a soft reset of the Mipi_Tx_Phy IP instance.
* The reset takes approx 20 core clock cycles to become effective.
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
*
* @return 	None
*
* @note		None.
*****************************************************************************/
void XMipi_Tx_Phy_Reset(XMipi_Tx_Phy *InstancePtr)
{
	u32 Value = XMIPI_TX_PHY_SOFTRESET_TIMEOUT;
	u32 RegVal;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.IsRegisterPresent != 0);

	/* Set the Reset bit in Control Register */
	RegVal = XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
			XMIPI_TX_PHY_CTRL_REG_OFFSET);

	RegVal |= XMIPI_TX_PHY_CTRL_REG_SOFTRESET_MASK;

	XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr, XMIPI_TX_PHY_CTRL_REG_OFFSET, RegVal);

	InstancePtr->IsReady = 0;

	/* Wait for at least 20 core clock cycles for reset to occur */
	while (Value--) {
	};

	/* Clear the reset bit */
	RegVal = XMipi_Tx_Phy_ReadReg((InstancePtr)->Config.BaseAddr,
			XMIPI_TX_PHY_CTRL_REG_OFFSET);

	RegVal &= ~XMIPI_TX_PHY_CTRL_REG_SOFTRESET_MASK;

	XMipi_Tx_Phy_WriteReg((InstancePtr)->Config.BaseAddr, XMIPI_TX_PHY_CTRL_REG_OFFSET, RegVal);

	/* Mark instance to be ready to be used */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}

/****************************************************************************/
/**
* This is used to clear the Abort Error (Escape or High Speed) bits
* in the Data Lane 0 through 3
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param 	DataLane represents which Data Lane to act upon
* @param 	Mask contains information about which bits to reset
*
* @return 	None
*
* @note     	None.
*****************************************************************************/
void XMipi_Tx_Phy_ClearDataLane(XMipi_Tx_Phy *InstancePtr, u8 DataLane, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertVoid(DataLane < InstancePtr->Config.MaxLanesPresent);
	Xil_AssertVoid((Mask & (XMIPI_TX_PHY_DLXSTATUS_REG_ESCABRT_MASK |
			XMIPI_TX_PHY_DLXSTATUS_REG_HSABRT_MASK)) != 0);

	/* Ensure only Escape Abort or HS Abort are set */
	Mask = Mask & (XMIPI_TX_PHY_DLXSTATUS_REG_ESCABRT_MASK |
			XMIPI_TX_PHY_DLXSTATUS_REG_HSABRT_MASK);

	XMipi_Tx_Phy_WriteReg(InstancePtr->Config.BaseAddr,
			XMIPI_TX_PHY_DL0STATUS_REG_OFFSET + (DataLane * 4), Mask);
}

/****************************************************************************/
/**
* This is used to get information about Clock Lane status
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
*
* @return 	Bitmask containing which of the events have occured along with
* 		the mode of the Clock Lane in DPhy
*
* @note 	None.
*****************************************************************************/
u32 XMipi_Tx_Phy_GetClkLaneStatus(XMipi_Tx_Phy *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(InstancePtr->Config.IsDphy);

	return (XMipi_Tx_Phy_ReadReg(InstancePtr->Config.BaseAddr,
				XMIPI_TX_PHY_CLSTATUS_REG_OFFSET));

}

/****************************************************************************/
/**
* This is used to get specific Lane mode information about Clock Lane.
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
*
* @return 	Bitmask containing mode in which the Clock Lane in Mipi_Tx_Phy is in.
*
* @note		None.
*****************************************************************************/
u32 XMipi_Tx_Phy_GetClkLaneMode(XMipi_Tx_Phy *InstancePtr)
{
	u32 Value;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(InstancePtr->Config.IsDphy);

	Value = XMipi_Tx_Phy_GetClkLaneStatus(InstancePtr);

	return (Value & XMIPI_TX_PHY_CLSTATUS_REG_MODE_MASK);
}

/****************************************************************************/
/**
* This is used to get information about a Data Lane status
*
* @param	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param	DataLane for which the status is sought for.
*
* @return	Bitmask containing which of the events have occured along with
* 		the mode of the Data Lane in DPhy
*
* @note		None.
*****************************************************************************/
u32 XMipi_Tx_Phy_GetDataLaneStatus(XMipi_Tx_Phy *InstancePtr, u8 DataLane)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(DataLane < InstancePtr->Config.MaxLanesPresent);

	return (XMipi_Tx_Phy_ReadReg(InstancePtr->Config.BaseAddr,
				(XMIPI_TX_PHY_DL0STATUS_REG_OFFSET + (4 * DataLane))));
}

/****************************************************************************/
/**
* This is used to get Data Lane Calibration status
*
* @param	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param	DataLane for which the calib status is sought for.
*
* @return	XST_SUCCESS - Calibration Complete, Calibration packet received
* 		XST_NO_DATA - Calibration Complete, Calibration packet is not received
* 		XST_FAILURE - Calibration failed
*
* @note		None.
*****************************************************************************/
u8 XMipi_Tx_Phy_GetDLCalibStatus(XMipi_Tx_Phy *InstancePtr, u8 DataLane)
{
	u32 Data;
	u8 ret;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(DataLane < InstancePtr->Config.MaxLanesPresent);

	Data = XMipi_Tx_Phy_ReadReg(InstancePtr->Config.BaseAddr,
				(XMIPI_TX_PHY_DL0STATUS_REG_OFFSET + (DL_LANE_OFFSET * DataLane)));

	if (Data & XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_COMPLETE_MASK) {
		if (Data & XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_STATUS_MASK)
			ret = XST_FAILURE;
		else
			ret = XST_SUCCESS;
	} else {
		if (Data & XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_STATUS_MASK)
			ret = XST_FAILURE;
		else
			ret = XST_NO_DATA;
	}

	return ret;
}

/****************************************************************************/
/**
* This is used to get specfic Lane mode information about a Data Lane.
*
* @param	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param	DataLane for which the mode info is requested.
*
* @return	Bitmask containing mode in which the Data Lane in Mipi_Tx_Phy is in.
*
* @note		None.
*****************************************************************************/
u32 XMipi_Tx_Phy_GetDataLaneMode(XMipi_Tx_Phy *InstancePtr, u8 DataLane)
{
	u32 Value;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(DataLane < InstancePtr->Config.MaxLanesPresent);

	Value = XMipi_Tx_Phy_GetDataLaneStatus(InstancePtr, DataLane);

	return (Value & XMIPI_TX_PHY_DLXSTATUS_REG_MODE_MASK);
}

/****************************************************************************/
/**
* This is used to get count of packets received on each lane
*
* @param	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param	DataLane for which the mode info is requested.
*
* @return	Bitmask containing mode in which the Data Lane in Mipi_Tx_Phy is in.
*
* @note		None.
*
*****************************************************************************/
u16 XMipi_Tx_Phy_GetPacketCount(XMipi_Tx_Phy *InstancePtr, u8 DataLane)
{
	u32 Value;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);
	Xil_AssertNonvoid(DataLane < InstancePtr->Config.MaxLanesPresent);

	Value = (XMipi_Tx_Phy_ReadReg(InstancePtr->Config.BaseAddr,
				(XMIPI_TX_PHY_DL0STATUS_REG_OFFSET + (4 * DataLane))));
	Value = Value & XMIPI_TX_PHY_DLXSTATUS_REG_PACKETCOUNT_MASK;
	Value >>= XMIPI_TX_PHY_DLXSTATUS_REG_PACKCOUNT_OFFSET;

	return (u16)Value;
}

/****************************************************************************/
/**
* This is used to get Mipi_Tx_Phy Version
*
* @param 	InstancePtr is the XMipi_Tx_Phy instance to operate on.
*
* @return 	Returns major and minor Version number of this Mipi_Tx_Phy IP
*
* @note 	None.
*****************************************************************************/
u32 XMipi_Tx_Phy_GetVersionReg(XMipi_Tx_Phy *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.IsRegisterPresent != 0);

	return (XMipi_Tx_Phy_ReadReg(InstancePtr->Config.BaseAddr,
				XMIPI_TX_PHY_VERSION_REG_OFFSET));

}

/****************************************************************************/
/**
* This function is used to enable or disable the Mipi_Tx_Phy core.
*
* @param	InstancePtr is the XMipi_Tx_Phy instance to operate on.
* @param	Flag denoting whether to enable or disable the Mipi_Tx_Phy core
*
* @return	None.
*
* @note 	None.
*****************************************************************************/
void XMipi_Tx_Phy_Activate(XMipi_Tx_Phy *InstancePtr, u8 Flag)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.IsRegisterPresent != 0);

	Value = XMipi_Tx_Phy_ReadReg(InstancePtr->Config.BaseAddr,
				XMIPI_TX_PHY_CTRL_REG_OFFSET);

	if (Flag == XMIPI_TX_PHY_ENABLE_FLAG) {
		Value |= XMIPI_TX_PHY_CTRL_REG_PHYEN_MASK;
	}
	else if (Flag == XMIPI_TX_PHY_DISABLE_FLAG) {
		Value &= ~(XMIPI_TX_PHY_CTRL_REG_PHYEN_MASK);
	} else {
		xil_printf("\n\r INFO: Invalid flag value: %d \t Expected 1 or 0\n\r", Flag);
	}

	/* Set or reset the Phy Enable bit in Control Register based
	 * on the flag
	 */
	XMipi_Tx_Phy_WriteReg(InstancePtr->Config.BaseAddr,
			XMIPI_TX_PHY_CTRL_REG_OFFSET, Value);
}
/** @} */
