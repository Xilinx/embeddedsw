/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xavb_example.c
*
* This file implements a simple example to show the usage of Audio Video
* Bridging (AVB) functionality of Axi Ethernet IP in loopback mode.
* The example uses the PTP Timer and the PTP Rx interrupts. A PDelay_Req packet
* is sent and is received back (as we are in loopback mode). .
* After this loop backed PDelay_Req packet is received, a PDelay_Resp and
* PDelay_RespFollowUp packets are  sent. These packets are also received.
* Since the source port identity of the received packets matches with our
* systems's own source port identity there is no further processing done.
*
* @note
*
* This code assumes the processor type is Microblaze, Xilinx interrupt
* controller (XIntc) is used in the system , and that no operating
* system is used.
*
* It also assumes that all the relevant AVB interrupts are properly
* connected to the Intc module.
*
* The Ethernet AVB Endpoint functionality should be enabled in the Xilinx
* Axi Ethernet core for this example to work.
*
* The Axi Ethernet is used with a GMII interface. The example initializes
* the GMII interface with 1000 Mbps speed.
*
* IMPORTANT NOTE:
* The user must define the macro XAVB_CLOCK_LOCK_THRESHOLD in xavb.h to
* an appropriate value as relevant for the corresponding use case. Presently
* it is defined to 1000 ns which is typical for telecom industry.
* This macro is used to compare against the slave error as calculated every time
* after receiving 2 successive sync/followup frames. Slave error is the
* difference between master time duration and slave time duration as calculated
* for the time gap (the time it takes to receive two successive sync/follow up
* frames). If slave error is greater than the value defined in
* XAVB_CLOCK_LOCK_THRESHOLD, then master and slave clocks are unlocked. This
* means the node running this SW assumes that the peer is no more capable of
* processing 802.1as frames. The node running the SW then waits till it successful
* calculates the path delay (which essentially means the peer is again capable
* of processing 802.1as frames.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------  -------- -----------------------------------------------------
* 1.00a kag/asa  08/25/10 First release
* 3.00a asa	 04/10/12 Disabled enabling of promiscuous mode. This is
*		          required for AxiEthernet cores with version v3_01_a
*		          onwards because of a change in the AVB implementation.
* 4.0   asa  03/06/14 Fix for CR 740863. Added a #warning message for
*				  users of this example to take note of the fact that
*				  we have just used a typical value for XAVB_CLOCK_LOCK_THRESHOLD
*                 and users may want to change it as per their requirements.
* 5.17  ml   11/15/23  Fix compilation errors reported with -std=c2x compiler flag
* </pre>
*******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xil_types.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xavb_hw.h"
#include "xavb.h"
#include "xil_cache.h"
#include "xaxiethernet.h"	/* defines Axi Ethernet APIs */

/************************** Constant Definitions *****************************/
/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define AXIETHERNET_DEVICE_ID	XPAR_AXIETHERNET_0_DEVICE_ID

#define AVB_PTP_RX_INTERRUPT_ID  \
	XPAR_INTC_0_AXIETHERNET_0_AV_INTERRUPT_PTP_RX_VEC_ID  	/*
							 * AVB PTP Received
							 * Frame Interrupt ID
*/
#define AVB_PTP_INTERRUPT_ID     \
	XPAR_INTC_0_AXIETHERNET_0_AV_INTERRUPT_10MS_VEC_ID  	/* AVB PTP Timer
							 * Interrupt ID
*/

/*
 * Other constants used in this file.
 */

#define PHY_DETECT_REG  	1
#define PHY_DETECT_MASK 	0x1808

#define PHY_R0_RESET		0x8000
#define PHY_R0_LOOPBACK		0x4000
#define PHY_R0_DFT_SPD_1000	0x0040

/*
 * The source MAC address used in this example. This will also form
 * a part of source port identity field in the PTP messages
 */
#define ETH_SYSTEM_ADDRESS_EUI48_HIGH  0x000A35
#define ETH_SYSTEM_ADDRESS_EUI48_LOW   0x010203

/******************************************************************************/
#warning The threshold or tolerance limit for slave error is currently set to \
		 1000 ns. This can be configured through the macro 					  \
		 XAVB_CLOCK_LOCK_THRESHOLD in xavb.h. Slave error is the difference   \
		 between measured master time duration and slave time duration over   \
		 two sync-frame intervals. If slave error exceeds the configured      \
		 threshold, the master and slave clocks are unlocked. For usage of the \
		 macro XAVB_CLOCK_LOCK_THRESHOLD refer to the function 				  \
		 XAvb_UpdateRtcIncrement in file xavb_rtc_sync.c
/******************************************************************************/

/************************** Function Prototypes ******************************/

static int AvbSetupInterruptSystem(XAvb *InstancePtr);
static int AvbConfigHW(XAxiEthernet *AxiEthernetInstancePtr, XAvb *InstancePtr);
void AvbGMDiscontinuityHandler(void *CallBackRef, u32 TimestampsUncertain);
static int AvbConfigureGmii(XAxiEthernet *InstancePtr, XAvb *AvbInstancePtr);
static int AvbEnterPhyLoopBack(XAxiEthernet *InstancePtr);
static void AvbUtilPhyDelay(unsigned int Seconds);
static void AvbEnablePTPInterrupts(void);
static void AvbUtilPrintMessage(char *Message);

static XAxiEthernet AxiEthernetInstance; /* Instance of Axi Ethernet driver */
static XAvb Avb; 			 /* Instance of AVB driver */
static XIntc InterruptController;	 /* Instance of INTC driver */

static XAvb_Config AvbConfigStruct;
volatile u8 EchoPTPFramesReceived = 0;

/*****************************************************************************/
/**
*
* This is the main function for the AVB example.
*
* @param	None.
*
* @return	- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate failure
*
* @note		This example will be in a infinite loop if the HW is not working
*		properly and if the interrupts are not received.
*
****************************************************************************/
int main()
{

	int Status;
	XAxiEthernet_Config *AxiEtherCfgPtr;
	XAvb_PortIdentity SystemIdentity;
	u32 WriteData;

#if XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
#endif

#if XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
#endif

	AvbUtilPrintMessage("\r\n--- Entering main() ---");

	/*
	 * Initialize Axi Ethernet Driver.
	 */
	AxiEtherCfgPtr = XAxiEthernet_LookupConfig(AXIETHERNET_DEVICE_ID);
	Status = XAxiEthernet_CfgInitialize(&AxiEthernetInstance,
					    AxiEtherCfgPtr,
					    AxiEtherCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		AvbUtilPrintMessage("Failed initializing config for Axi Ethernet\r\n");
		AvbUtilPrintMessage("--- Exiting main() ---\r\n");
		return XST_FAILURE;
	}

	/*
	 * Initialize the AVB Config structure.
	 */
	AvbConfigStruct.DeviceId = AxiEtherCfgPtr->DeviceId;
	AvbConfigStruct.BaseAddress = AxiEtherCfgPtr->BaseAddress;
	Status = XAvb_CfgInitialize(&Avb, &AvbConfigStruct, AvbConfigStruct.BaseAddress);
	if (Status != XST_SUCCESS) {
		AvbUtilPrintMessage("Failed initializing config for AVB\r\n");
		AvbUtilPrintMessage("--- Exiting main() ---\r\n");
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the AVB that will be called if the PTP drivers
	 * identify a possible discontinuity in GrandMaster time.
	 */
	XAvb_SetGMDiscontinuityHandler(&Avb, AvbGMDiscontinuityHandler, &Avb);

	/*
	 * Reset and initialize the AVB driver.
	 */
	XAvb_Reset(&Avb);

	/*
	 * Perform configuration on the Axi Ethernet and Ethernet AVB Endpoint
	 * cores
	 */
	Status = AvbConfigHW(&AxiEthernetInstance, &Avb);
	if (Status != XST_SUCCESS) {
		AvbUtilPrintMessage("Failed initializing Axi Ethernet and AVB\r\n");
		AvbUtilPrintMessage("--- Exiting main() ---\r\n");
		return XST_FAILURE;
	}

	/*
	 * Initialize Interrupt Controller
	 */
	Status = AvbSetupInterruptSystem(&Avb);
	if (Status != XST_SUCCESS) {
		AvbUtilPrintMessage("Failed initializing INTC system\r\n");
		AvbUtilPrintMessage("--- Exiting main() ---\r\n");
		return XST_FAILURE;
	}

	/*
	 * Setup the Source Address and Source Port Identity fields in all
	 * TX PTP Buffers
	 */
	SystemIdentity.ClockIdentityUpper = (ETH_SYSTEM_ADDRESS_EUI48_HIGH
					     << 8) | 0xFF;
	SystemIdentity.ClockIdentityLower = (0xFE << 24) |
					    (ETH_SYSTEM_ADDRESS_EUI48_LOW);
	SystemIdentity.PortNumber = 1;

	/*
	 * Write the SA to all default TX PTP buffers
	 */
	WriteData = (XAvb_ReorderWord(SystemIdentity.ClockIdentityUpper)) << 16;
	XAvb_WriteToMultipleTxPtpFrames(Avb.Config.BaseAddress,
					XAVB_PTP_TX_PKT_SA_UPPER_OFFSET,
					(WriteData & 0xFFFF0000),
					0xFFFF0000,
					0x7F);

	WriteData = (SystemIdentity.ClockIdentityUpper << 16);
	WriteData = (WriteData & 0xFF000000);
	WriteData = (WriteData | (SystemIdentity.ClockIdentityLower & 0x00FFFFFF));
	WriteData = XAvb_ReorderWord(WriteData);
	XAvb_WriteToMultipleTxPtpFrames(Avb.Config.BaseAddress,
					XAVB_PTP_TX_PKT_SA_LOWER_OFFSET,
					WriteData,
					0xFFFFFFFF,
					0x7F);

	/*
	 * Write sourceportidentity to all default TX PTP buffers
	 */
	XAvb_SetupSourcePortIdentity(&Avb, SystemIdentity);

	/*
	 * Start AVB and enable the PTP interrupts.
	 */
	XAvb_Start(&Avb);
	AvbEnablePTPInterrupts();

	while (1) {
		if (EchoPTPFramesReceived) {
			AvbUtilPrintMessage("\r\nExample passed\r\n");
			AvbUtilPrintMessage("--- Exiting main() ---\r\n");
			break;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function configures Axi Ethernet core and AVB module.
*
* @param	AxiEthernetInstancePtr is a pointer to the Axi Ethernet driver
*		instance
* @param	AvbInstancePtr is a pointer to the AVB instance.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure.
*
* @note		None.
*
******************************************************************************/
static int AvbConfigHW(XAxiEthernet *AxiEthernetInstancePtr, XAvb *AvbInstancePtr)
{
	u32 ReadData;
	int Status;

	/*
	 * Configure MDIO Master in Axi Ethernet - MUST be done before
	 * any MDIO accesses
	 */
	XAxiEthernet_WriteReg(AxiEthernetInstancePtr->Config.BaseAddress,
			      XAE_MDIO_MC_OFFSET, 0x0000005D);

	/*
	 * Disable Axi Ethernet Flow Control
	 */
	XAxiEthernet_WriteReg(AxiEthernetInstancePtr->Config.BaseAddress,
			      XAE_FCC_OFFSET, 0x0);

	/*
	 * Initialise Axi Ethernet by enabling Tx and Rx with VLAN capability
	 */
	XAxiEthernet_WriteReg(AxiEthernetInstancePtr->Config.BaseAddress,
			      XAE_TC_OFFSET, XAE_TC_TX_MASK | XAE_TC_VLAN_MASK);
	XAxiEthernet_WriteReg(AxiEthernetInstancePtr->Config.BaseAddress,
			      XAE_RCW1_OFFSET, XAE_RCW1_RX_MASK | XAE_RCW1_VLAN_MASK);

	/*
	 * Initialise RTC reference clock for nominal frequency 125MHz -
	 * (see xavb_hw.h for value)
	 */
	XAvb_WriteReg(AvbInstancePtr->Config.BaseAddress,
		      XAVB_RTC_INCREMENT_OFFSET,
		      XAVB_RTC_INCREMENT_NOMINAL_RATE);

	Status = AvbConfigureGmii(AxiEthernetInstancePtr, AvbInstancePtr);
	return Status;
}

/*****************************************************************************/
/**
*
* This function configures the GMII interface and Axi Ethernet registers for
* 1000 Mbps speed configuration.
*
* @param	InstancePtr is a pointer to the Axi Ethernet driver instance
* @param	AvbInstancePtr is a pointer to the AVB instance.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure...
*
* @note		None.
*
******************************************************************************/
static int AvbConfigureGmii(XAxiEthernet *InstancePtr, XAvb *AvbInstancePtr)
{
	u32 EmmcReg;
	int Status;

	/*
	 * Set PHY to loopback.
	 */
	Status = AvbEnterPhyLoopBack(InstancePtr);
	if (Status != XST_SUCCESS) {
		XAvb_Stop(AvbInstancePtr);
		return XST_FAILURE;
	}

	/*
	 * Get the current contents of the EMAC config register and
	 * zero out speed bits
	 */
	EmmcReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
				       XAE_EMMC_OFFSET);
	EmmcReg = EmmcReg & (~XAE_EMMC_LINKSPEED_MASK);
	EmmcReg |= XAE_EMMC_LINKSPD_1000;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			      XAE_EMMC_OFFSET, EmmcReg);

	/*
	 * Setting the operating speed of the MAC needs a delay.  There
	 * doesn't seem to be register to poll, so please consider this
	 * during your application design.
	 */
	AvbUtilPhyDelay(1);
	return XST_SUCCESS;

}

/******************************************************************************/
/**
*
* This function sets the PHY to loopback mode. This works with the marvell PHY
* common on Xilinx evaluation boards. This sets the PHY speed to 1000 Mbps.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
******************************************************************************/
static int AvbEnterPhyLoopBack(XAxiEthernet *InstancePtr)
{
	u16 PhyReg0;
	signed int PhyAddr;
	u16 PhyReg;

	for (PhyAddr = 31; PhyAddr >= 0; PhyAddr--) {
		XAxiEthernet_PhyRead(&AxiEthernetInstance, PhyAddr,
				     PHY_DETECT_REG, &PhyReg);
		if ((PhyReg != 0xFFFF) && ((PhyReg & PHY_DETECT_MASK)
					   == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			break;
		}
	}

	if (PhyAddr == 0) {
		return XST_FAILURE;
	}
	/*
	 * Clear the PHY of any existing bits by zeroing this out
	 */
	PhyReg0 = 0;
	PhyReg0 |= PHY_R0_DFT_SPD_1000;

	/*
	 * Set the speed and put the PHY in reset, then put the PHY in loopback
	 */
	XAxiEthernet_PhyWrite(&AxiEthernetInstance, PhyAddr, 0,
			      PhyReg0 | PHY_R0_RESET);
	AvbUtilPhyDelay(1);
	XAxiEthernet_PhyRead(&AxiEthernetInstance, PhyAddr, 0, &PhyReg0);
	XAxiEthernet_PhyWrite(&AxiEthernetInstance, PhyAddr, 0,
			      PhyReg0 | PHY_R0_LOOPBACK);
	AvbUtilPhyDelay(1);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* AVB design.
*
* @param	InstancePtr contains a pointer to the instance of the AVB
*		component which is going to be connected to the interrupt
*		controller.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
****************************************************************************/
static int AvbSetupInterruptSystem(XAvb *InstancePtr)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it can be used.
	 * XPAR_INTC_0_DEVICE_ID specifies the XINTC device ID that is
	 * generated in xparameters.h
	 */
	Status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the Ethernet AVB Endpoint's 10 ms interrupt
	 */
	Status = XIntc_Connect(&InterruptController,
			       AVB_PTP_INTERRUPT_ID,
			       (XInterruptHandler)XAvb_PtpTimerInterruptHandler,
			       InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the Ethernet AVB Endpoint's PTP Rx interrupt
	 */
	Status = XIntc_Connect(&InterruptController,
			       AVB_PTP_RX_INTERRUPT_ID,
			       (XInterruptHandler)XAvb_PtpRxInterruptHandler,
			       InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable interrupt on Microblaze
	 */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIntc_InterruptHandler,
				     (void *)&InterruptController);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables the PTP Timer interrupt and PTP Rx interrupt in the
* INTC module.
*
* @param	None
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void AvbEnablePTPInterrupts(void)
{
	XIntc_Enable(&InterruptController, AVB_PTP_RX_INTERRUPT_ID);
	XIntc_Enable(&InterruptController, AVB_PTP_INTERRUPT_ID);
	return;
}

/****************************************************************************/
/**
*
* This function is the handler which will be called if the PTP drivers
* identify a possible discontinuity in GrandMaster time.
*
* This handler provides an example of how to handle this situation -
* but this function is application specific.
*
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the AVB driver.
* @param	TimestampsUncertain - a value of 1 indicates that there is a
*		possible discontinuity in GrandMaster time. A value of 0
*		indicates that Timestamps are no longer uncertain.
*
* @return	None.
*
* @note		This Handler need to be defined otherwise the XAvb_StubHandler
*		will generate an error
*
****************************************************************************/
void AvbGMDiscontinuityHandler(void *CallBackRef, u32 TimestampsUncertain)
{

	xil_printf("\r\nGMDiscontinuityHandler: Timestamps are now %s\r\n",
		   TimestampsUncertain ? "uncertain" : "certain");

}

/******************************************************************************/
/**
*
* For Microblaze we use an assembly loop that is roughly the same regardless of
* optimization level, although caches and memory access time can make the delay
* vary.  Just keep in mind that after resetting or updating the PHY modes,
* the PHY typically needs time to recover.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void AvbUtilPhyDelay(unsigned int Seconds)
{
	static int WarningFlag = 0;

	/* If MB caches are disabled or do not exist, this delay loop could
	 * take minutes instead of seconds (e.g., 30x longer).  Print a warning
	 * message for the user (once).  If only MB had a built-in timer!
	 */
	if (((mfmsr() & 0x20) == 0) && (!WarningFlag)) {
		WarningFlag = 1;
	}

#define ITERS_PER_SEC   (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 6)
	__asm volatile ("\n"
			      "1:               \n\t"
			      "addik r7, r0, %0 \n\t"
			      "2:               \n\t"
			      "addik r7, r7, -1 \n\t"
			      "bneid  r7, 2b    \n\t"
			      "or  r0, r0, r0   \n\t"
			      "bneid %1, 1b     \n\t"
			      "addik %1, %1, -1 \n\t"
			      :: "i"(ITERS_PER_SEC), "d" (Seconds));

}

/******************************************************************************/
/**
*
* This function is called by example code to display a console message
*
* @param	Message is the text explaining the error
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void AvbUtilPrintMessage(char *Message)
{
#ifdef STDOUT_BASEADDRESS
	xil_printf("%s\r\n", Message);
#endif
}
