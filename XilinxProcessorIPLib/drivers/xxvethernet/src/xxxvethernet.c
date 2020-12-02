/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet.c
* @addtogroup xxvethernet_v1_5
* @{
*
* This file implements all the XXV ethernet functions to initialize, start,
* stop, reset and reconfigure the MAC.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   6/16/17  First release
*       hk   2/15/18  Add support for USXGMII
* 1.4   rsp  5/08/20  Remove unused variable in usxgmii autoneg reset and
                      restart function.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xxxvethernet.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void XXxvEthernet_InitHw(XXxvEthernet *InstancePtr);	/* HW reset */

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* XXxvEthernet_CfgInitialize initializes an XXV Ethernet device along with the
* <i>InstancePtr</i> that references it.
*
* @param	InstancePtr references the memory instance to be associated
*		with the XXV Ethernet core instance upon initialization.
* @param	CfgPtr references the structure holding the hardware
*		configuration for the Xxv Ethernet core to initialize.
* @param	EffectiveAddress is the processor address used to access the
*		base address of the XXV Ethernet instance. In systems with an
*		MMU and virtual memory, <i>EffectiveAddress</i> is the
*		virtual address mapped to the physical in
*		<code>ConfigPtr->Config.BaseAddress</code>. In systems without
*		an active MMU, <i>EffectiveAddress</i> should be set to the
*		same value as <code>ConfigPtr->Config.BaseAddress</code>.
*
* @return	XST_SUCCESS
*
* @note		None.
*
******************************************************************************/
int XXxvEthernet_CfgInitialize(XXxvEthernet *InstancePtr,
				XXxvEthernet_Config *CfgPtr,
				UINTPTR EffectiveAddress)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XXxvEthernet));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XXxvEthernet_Config));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_CfgInitialize\n");

	/* Set device base address */
	InstancePtr->Config.BaseAddress = EffectiveAddress;

	/* Reset the hardware and set default options */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	XXxvEthernet_Reset(InstancePtr);

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XxvEthernet_CfgInitialize: returning SUCCESS\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* XXxvEthernet_Initialize initializes an XXV Ethernet device along with the
* <i>InstancePtr</i> that references it.
*
* The PHY is setup independently from the Ethernet core. Use the MII or
* whatever other interface may be present for setup.
*
* @param	InstancePtr references the memory instance to be associated
*		with the AXI Ethernet core instance upon initialization.
* @param	CfgPtr references the structure holding the hardware
*		configuration for the Axi Ethernet core to initialize.
*
* @return	XST_SUCCESS.
*
* @note		When user calls this function he should ensure the hardware
*		is in a quiescent state by resetting all the hardware
*		Configurations.
*
******************************************************************************/
int XXxvEthernet_Initialize(XXxvEthernet *InstancePtr,
			    XXxvEthernet_Config *CfgPtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XXxvEthernet));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XXxvEthernet_Config));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_Initialize\n");

	/* Set device base address */
	InstancePtr->Config.BaseAddress = CfgPtr->BaseAddress;

	/* Set default options */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* XXxvEthernet_Start starts the Xxv Ethernet device as follows:
*	- Enable transmitter if XXE_TRANSMIT_ENABLE_OPTION is set
*	- Enable receiver if XXE_RECEIVER_ENABLE_OPTION is set
*	- Upon enabling RX, check for RX BLOCK LOCK bit to make sure
*         RX channel is ready.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @return	XST_FAILURE if RX is enabled and BLOCK LOCK is not set.
*
* @note		None
*
*
******************************************************************************/
int XXxvEthernet_Start(XXxvEthernet *InstancePtr)
{
	u32 Reg;
	int Timeout = 10000000;

	/* Assert bad arguments and conditions */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/* If already started, then there is nothing to do */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return 0;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_Start\n");

	/* Enable transmitter if not already enabled */
	if (InstancePtr->Options & XXE_TRANSMITTER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "enabling transmitter\n");
		Reg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XXE_TXCFG_OFFSET);
		if (!(Reg & XXE_TXCFG_TX_MASK)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"transmitter not enabled, enabling now\n");
			XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XXE_TXCFG_OFFSET,
							Reg | XXE_TXCFG_TX_MASK);
		}
		xdbg_printf(XDBG_DEBUG_GENERAL, "transmitter enabled\n");
	}

	/* Enable receiver */
	if (InstancePtr->Options & XXE_RECEIVER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "enabling receiver\n");
		Reg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
				XXE_RXCFG_OFFSET);
		if (!(Reg & XXE_RXCFG_RX_MASK)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"receiver not enabled, enabling now\n");

			XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
					XXE_RXCFG_OFFSET,
							Reg | XXE_RXCFG_RX_MASK);
		}
		/* RX block lock */
		/* Do a dummy read because this is a sticky bit */
		XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XXE_RXBLSR_OFFSET);
		while(!(XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
					XXE_RXBLSR_OFFSET) & XXE_RXBLKLCK_MASK)) {
			Timeout--;
			if(Timeout <= 0) {
				xil_printf("ERROR: Block lock is not set \n\r");
				return XST_FAILURE;
			}
		}
		xdbg_printf(XDBG_DEBUG_GENERAL, "receiver enabled\n");
	}

	/* Mark as started */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_Start: done\n");

	return 0;
}

/*****************************************************************************/
/**
* XXxvEthernet_Stop gracefully stops the Xxv Ethernet device by
* disabling the receiver.
*
* XXxvEthernet_Stop does not modify any of the current device options.
*
* Since the transmitter is not disabled, frames currently in internal buffers
* or in process by a DMA engine are allowed to be transmitted.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @return	None
*
* @note		None.
*
*
******************************************************************************/
void XXxvEthernet_Stop(XXxvEthernet *InstancePtr)
{
	u32 Reg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/* If already stopped, then there is nothing to do */
	if (InstancePtr->IsStarted == 0) {
		return;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_Stop\n");

	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_Stop: disabling receiver\n");

	/* Disable the receiver */
	Reg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
			XXE_RXCFG_OFFSET);
	Reg &= ~XXE_RXCFG_RX_MASK;
	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XXE_RXCFG_OFFSET, Reg);

	/* Mark as stopped */
	InstancePtr->IsStarted = 0;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_Stop: done\n");
}


/*****************************************************************************/
/**
* XXxvEthernet_Reset does not perform a soft reset of the XxvEthernet core.
* XxvEthernet hardware is reset by the device connected to the AXI4-Stream
* interface i.e. MCDMA. This function simply disabled TX and RX.
* This function inserts some delay before proceeding to stop the device.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @note		None
*
******************************************************************************/
void XXxvEthernet_Reset(XXxvEthernet *InstancePtr)
{
	volatile u32 TimeoutLoops;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Add delay of 10000 loops to give enough time to the core to come
	 * out of reset. Till the time core comes out of reset none of the
	 * XxvEthernet registers are accessible including the IS register.
	 */
	TimeoutLoops = XXE_LOOPS_TO_COME_OUT_OF_RST;
		while (TimeoutLoops > 0) {
			TimeoutLoops--;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_Reset\n");

	/* Stop the device and reset HW */
	XXxvEthernet_Stop(InstancePtr);
	InstancePtr->Options = XXE_DEFAULT_OPTIONS;

	/* Setup HW */
	XXxvEthernet_InitHw(InstancePtr);
}


/******************************************************************************
*
* XXxvEthernet_InitHw (internal use only) performs a one-time setup of
* Xxv Ethernet device. The setup performed here only need to occur once
* after any reset.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @note		None.
*
*
******************************************************************************/
static void XXxvEthernet_InitHw(XXxvEthernet *InstancePtr)
{
	u32 Reg;

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_InitHw\n");


	/* Disable the receiver */
	Reg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
								XXE_RXCFG_OFFSET);
	Reg &= ~XXE_RXCFG_RX_MASK;
	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XXE_RXCFG_OFFSET, Reg);

	/*
	 * Sync default options with HW but leave receiver and transmitter
	 * disabled. They get enabled with XXxvEthernet_Start() if
	 * XXE_TRANSMITTER_ENABLE_OPTION and XXE_RECEIVER_ENABLE_OPTION
	 * are set
	 */
	XXxvEthernet_SetOptions(InstancePtr, InstancePtr->Options &
					~(XXE_TRANSMITTER_ENABLE_OPTION |
					XXE_RECEIVER_ENABLE_OPTION));

	XXxvEthernet_ClearOptions(InstancePtr, ~InstancePtr->Options);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_InitHw: done\n");
}

/*****************************************************************************/
/**
* XXxvEthernet_SetOptions enables the options, <i>Options</i> for the
* Xxv Ethernet, specified by <i>InstancePtr</i>. Xxv Ethernet should be
* stopped with XXxvEthernet_Stop() before changing options.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @param	Options is a bitmask of OR'd XXE_*_OPTION values for options to
*		set. Options not specified are not affected.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the device has not been stopped.
*
*
* @note
* See xxxvethernet.h for a description of the available options.
*
*
******************************************************************************/
int XXxvEthernet_SetOptions(XXxvEthernet *InstancePtr, u32 Options)
{
	u32 RegRcfg;	/* Reflects original contents of RCFG */
	u32 RegTc;	/* Reflects original contents of TC  */
	u32 RegNewRcfg;	/* Reflects new contents of RCFG */
	u32 RegNewTc;	/* Reflects new contents of TC  */

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_SetOptions\n");

	/*
	 * Set options word to its new value.
	 */
	InstancePtr->Options |= Options;

	/*
	 * Many of these options will change the RCFG or TCFG registers.
	 * To reduce the amount of IO to the device, group these options here
	 * and change them all at once.
	 */
	/* Get current register contents */
	RegRcfg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
			XXE_RXCFG_OFFSET);
	RegTc = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XXE_TXCFG_OFFSET);
	RegNewRcfg = RegRcfg;
	RegNewTc = RegTc;

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"current control regs: RCFG: 0x%0x; TC: 0x%0x\n",
			RegRcfg, RegTc);
	xdbg_printf(XDBG_DEBUG_GENERAL,
			"Options: 0x%0x; default options: 0x%0x\n",Options,
							XXE_DEFAULT_OPTIONS);
	/* Turn on FCS stripping on receive packets */
	if (Options & XXE_FCS_STRIP_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: enabling fcs stripping\n");
		RegNewRcfg |= XXE_RXCFG_DEL_FCS_MASK;
	}

	/* Turn on FCS insertion on transmit packets */
	if (Options & XXE_FCS_INSERT_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: enabling fcs insertion\n");
		RegNewTc |= XXE_TXCFG_FCS_MASK;
	}

	/* Enable transmitter */
	if (Options & XXE_TRANSMITTER_ENABLE_OPTION) {
		RegNewTc |= XXE_TXCFG_TX_MASK;
	}

	/* Enable receiver */
	if (Options & XXE_RECEIVER_ENABLE_OPTION) {
		RegNewRcfg |= XXE_RXCFG_RX_MASK;
	}

	/* Change the TC or RCFG registers if they need to be modified */
	if (RegTc != RegNewTc) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: writing tc: 0x%0x\n", RegNewTc);
		XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XXE_TXCFG_OFFSET, RegNewTc);
	}

	if (RegRcfg != RegNewRcfg) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"setOptions: writing rcfg: 0x%0x\n", RegNewRcfg);
		XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XXE_RXCFG_OFFSET, RegNewRcfg);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "setOptions: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XXxvEthernet_ClearOptions clears the options, <i>Options</i> for the
* Xxv Ethernet, specified by <i>InstancePtr</i>. Xxv Ethernet should be stopped
* with XXxvEthernet_Stop() before changing options.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @param	Options is a bitmask of OR'd XXE_*_OPTION values for options to
*		clear. Options not specified are not affected.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the device has not been stopped.
*
* @note
* See xxxvethernet.h for a description of the available options.
*
******************************************************************************/
int XXxvEthernet_ClearOptions(XXxvEthernet *InstancePtr, u32 Options)
{
	u32 RegRcfg;	/* Reflects original contents of RCFG */
	u32 RegTc;	/* Reflects original contents of TC  */
	u32 RegNewRcfg;	/* Reflects new contents of RCFG */
	u32 RegNewTc;	/* Reflects new contents of TC  */

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_ClearOptions: 0x%08x\n",
								Options);
	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	/*
	 * Set options word to its new value.
	 */
	InstancePtr->Options &= ~Options;

	/*
	 * Many of these options will change the RCFG or TC registers.
	 * Group these options here and change them all at once. What we are
	 * trying to accomplish is to reduce the amount of IO to the device
	 */

	/* Get the current register contents */
	RegRcfg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XXE_RXCFG_OFFSET);
	RegTc = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XXE_TXCFG_OFFSET);
	RegNewRcfg = RegRcfg;
	RegNewTc = RegTc;

	/* Turn off FCS stripping on receive packets */
	if (Options & XXE_FCS_STRIP_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_ClearOptions: disabling fcs strip\n");
		RegNewRcfg &= ~XXE_RXCFG_DEL_FCS_MASK;
	}

	/* Turn off FCS insertion on transmit packets */
	if (Options & XXE_FCS_INSERT_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_ClearOptions: disabling fcs insert\n");
		RegNewTc &= ~XXE_TXCFG_FCS_MASK;
	}

	/* Disable transmitter */
	if (Options & XXE_TRANSMITTER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_ClearOptions: disabling transmitter\n");
		RegNewTc &= ~XXE_TXCFG_TX_MASK;
	}

	/* Disable receiver */
	if (Options & XXE_RECEIVER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_ClearOptions: disabling receiver\n");
		RegNewRcfg &= ~XXE_RXCFG_RX_MASK;
	}

	/* Change the TC and RCFG registers if they need to be
	 * modified
	 */
	if (RegTc != RegNewTc) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_ClearOptions: setting TC: 0x%0x\n", RegNewTc);
		XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XXE_TXCFG_OFFSET, RegNewTc);
	}

	if (RegRcfg != RegNewRcfg) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XXxvEthernet_ClearOptions: setting RCFG: 0x%0x\n",RegNewRcfg);
		XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XXE_RXCFG_OFFSET, RegNewRcfg);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "ClearOptions: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XXxvEthernet_GetOptions returns the current option settings.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return	Returns a bitmask of XXE_*_OPTION constants,
*		each bit specifying an option that is currently active.
*
* @note
* See xxxvethernet.h for a description of the available options.
*
******************************************************************************/
u32 XXxvEthernet_GetOptions(XXxvEthernet *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Options);
}

/*****************************************************************************/
/**
 * XXxvEthernet_SetUsxgmiiRateAndDuplex sets the speed and duplex ability in
 * USXGMII Autonegotiation register.
 *
 * @param	InstancePtr references the Xxv Ethernet on which to
 *		operate.
 * @param	Rate to be set - currently only 1G and 2.5G are tested.
 *		Pass RATE_1G or RATE_2G5 to the function.
 * @param	Set FD or HD - if 0 Half duplex is set, else Full duplex.
 *
 * @return	- XST_SUCCESS on successful setting of speed.
 *		- XST_FAILURE, if the speed cannot be set for the present
 *		  harwdare configuration.
 *
 * @note	None.
 *
 *
 ******************************************************************************/
int XXxvEthernet_SetUsxgmiiRateAndDuplex(XXxvEthernet *InstancePtr, u32 Rate, u32 SetFD)
{
	u32 RateMask, UsxgmiiAnReg;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_SetUsxgmiiRateAndDuplex\n");

	/* Set speed in axi lite register field and the signal field to
	 * keep them in sync
	 */
	switch(Rate) {
		case RATE_1G:
			RateMask = (XXE_USXGMII_RATE_1G_MASK << XXE_USXGMII_RATE_SHIFT) |
						(XXE_USXGMII_RATE_1G_MASK << XXE_USXGMII_SPEED_SHIFT);
			break;
		case RATE_2G5:
			RateMask = (XXE_USXGMII_RATE_2G5_MASK << XXE_USXGMII_RATE_SHIFT) |
						(XXE_USXGMII_RATE_2G5_MASK << XXE_USXGMII_SPEED_SHIFT);
			break;
		case RATE_10G:
			RateMask = (XXE_USXGMII_RATE_10G_MASK << XXE_USXGMII_RATE_SHIFT) |
						(XXE_USXGMII_RATE_10G_MASK << XXE_USXGMII_SPEED_SHIFT);
			break;
		case RATE_10M:
			RateMask = (XXE_USXGMII_RATE_10M_MASK << XXE_USXGMII_RATE_SHIFT) |
						(XXE_USXGMII_RATE_10M_MASK << XXE_USXGMII_SPEED_SHIFT);
			break;
		case RATE_100M:
			RateMask = (XXE_USXGMII_RATE_100M_MASK << XXE_USXGMII_RATE_SHIFT) |
						(XXE_USXGMII_RATE_100M_MASK << XXE_USXGMII_SPEED_SHIFT);
			break;
		default:
			return XST_FAILURE;
	}
	/* Set the speed and make sure to enable USXGMII */
	UsxgmiiAnReg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
				XXE_USXGMII_AN_OFFSET);
	UsxgmiiAnReg &= ~(XXE_USXGMII_RATE_MASK | XXE_USXGMII_ANA_SPEED_MASK );
	UsxgmiiAnReg |= (XXE_USXGMII_ANA_MASK | XXE_USXGMII_LINK_STS_MASK | RateMask);


	/* Duplex setting */
	if(!SetFD) {
		UsxgmiiAnReg &= ~XXE_USXGMII_ANA_FD_MASK;
	} else {
		UsxgmiiAnReg |= XXE_USXGMII_ANA_FD_MASK;
	}
	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XXE_USXGMII_AN_OFFSET, UsxgmiiAnReg);
	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XXxvEthernet_SetUsxgmiiRateAndDuplex: done\n");

	return (XST_SUCCESS);
}
/** @} */

/*****************************************************************************/
/**
*
* XXxvEthernet_UsxgmiiAnMainReset sets the USXGMII AN Main reset.
* A delay is provided in between setting and clearing the main reset bit.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @return	None
* @note 	This function is only supported for USXGMII Ethernet MAC
*
******************************************************************************/
void XXxvEthernet_UsxgmiiAnMainReset(XXxvEthernet *InstancePtr)
{
	u32 UsxgmiiAnReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	UsxgmiiAnReg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
					XXE_USXGMII_AN_OFFSET);
	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XXE_USXGMII_AN_OFFSET,
				UsxgmiiAnReg | XXE_USXGMII_ANMAINRESET_MASK);

	usleep(100);

	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XXE_USXGMII_AN_OFFSET,
				UsxgmiiAnReg & ~XXE_USXGMII_ANMAINRESET_MASK);
}

/*****************************************************************************/
/**
*
* XXxvEthernet_UsxgmiiAnMainRestart sets the USXGMII AN Main restart.
* A delay is provided in between setting and clearing this bit as it is
* not self clearing.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @return	None
* @note 	This function is only supported for USXGMII Ethernet MAC
*
******************************************************************************/
void XXxvEthernet_UsxgmiiAnMainRestart(XXxvEthernet *InstancePtr)
{
	u32 UsxgmiiAnReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	UsxgmiiAnReg = XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
					XXE_USXGMII_AN_OFFSET);
	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XXE_USXGMII_AN_OFFSET,
				UsxgmiiAnReg & ~XXE_USXGMII_ANRESTART_MASK);

	usleep(100);

	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XXE_USXGMII_AN_OFFSET,
				UsxgmiiAnReg | XXE_USXGMII_ANRESTART_MASK);

	usleep(100);

	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XXE_USXGMII_AN_OFFSET,
				UsxgmiiAnReg & ~XXE_USXGMII_ANRESTART_MASK);
}
/** @} */

/*****************************************************************************/
/**
 * XXxvEthernet_GetAutoNegSpeed reports the speed (only 10G supported) from
 * the Autonegotiation status register.
 *
 * @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
 *		worked on.
 *
 * @return	Returns the link speed in units of gigabits per second (10)
 *		Can return a value of 0, in case 10Gbps is not set.
 *
 * @note	This function is only supported for XXV Ethernet MAC
 *
 ******************************************************************************/
u16 XXxvEthernet_GetAutoNegSpeed(XXxvEthernet *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_GetOperatingSpeed\n");
	if (XXxvEthernet_ReadReg(InstancePtr->Config.BaseAddress,
				XXE_ANASR_OFFSET) & XXE_ANA_10GKR_MASK) {

		xdbg_printf(XDBG_DEBUG_GENERAL,
			"XXxvEthernet_GetOperatingSpeed: returning 1000\n");
		return XXE_SPEED_10_GBPS;
	} else {
		return 0;
	}
}

/*****************************************************************************/
/**
 * XXxvEthernet_SetAutoNegSpeed sets the speed (only 10G supported) in
 * the Autonegotiation control register.
 *
 * @param	InstancePtr references the Xxv Ethernet on which to
 *		operate.
 *
 * @return	- XST_SUCCESS on successful setting of speed.
 *		- XST_FAILURE, if the speed cannot be set for the present
 *		  harwdare configuration.
 *
 * @note	This function is only supported for XXV Ethernet MAC
 *
 *
 ******************************************************************************/
int XXxvEthernet_SetAutoNegSpeed(XXxvEthernet *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XXxvEthernet_SetOperatingSpeed\n");

	/* Set register and return */
	XXxvEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XXE_ANACR_OFFSET, XXE_ANA_10GKR_MASK);
	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XXxvEthernet_SetOperatingSpeed: done\n");
	return (XST_SUCCESS);
}
/** @} */
