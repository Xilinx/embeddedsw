/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xemacps_example_intr_dma.c
*
* Implements examples that utilize the EmacPs's interrupt driven DMA
* packet transfer mode to send and receive frames.
*
* These examples demonstrate:
*
* - How to perform simple send and receive.
* - Interrupt
* - Error handling
* - Device reset
*
* Functional guide to example:
*
* - EmacPsDmaSingleFrameIntrExample demonstrates the simplest way to send and
*   receive frames in in interrupt driven DMA mode.
*
* - EmacPsErrorHandler() demonstrates how to manage asynchronous errors.
*
* - EmacPsResetDevice() demonstrates how to reset the driver/HW without
*   losing all configuration settings.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  01/10/10 First release
* 1.00a asa  11/25/11 The cache disable routines are removed. So now both
*		      I-cache and D-cache are enabled. The array RxBuffer is
*		      removed to avoid an extra copy from RxBuffer to RxFrame.
*		      Now the address of RxFrame is submitted to the Rx BD
*		      instead of the address of RxBuffer.
*		      In function EmacPsDmaSingleFrameIntrExample, BdRxPtr
*		      is made as a pointer instead of array of pointers. This
*		      is done since on the Rx path we now submit a single BD
*		      instead of all 32 BDs. Because of this change, relevant
*		      changes are made throughout the function
*		      EmacPsDmaSingleFrameIntrExample.
*		      Cache invalidation is now being done for the RxFrame
*		      buffer.
*		      The unnecessary cache flush (Xil_DCacheFlushRange) is
*		      removed. This was being done towards the end of the
*		      example which was unnecessary.
* 1.00a asa 01/24/12  Support for Zynq board is added. The SLCR divisors are
* 		      different for Zynq. Changes are made for the same.
* 		      Presently the SLCR GEM clock divisors are hard-coded
*		      assuming that IO PLL output frequency is 1000 MHz.
* 		      The BDs are allocated at the address 0xFF00000 and the
* 		      1 MB address range starting from this address is made
* 		      uncached. This is because, for GEM the BDs need to be
* 		      placed in uncached memory. The RX BDs are allocated at
* 		      address 0xFF00000 and TX BDs are allocated at address
* 		      0xFF10000.
* 		      The MDIO divisor used of 224 is used for Zynq board.
* 1.01a asa 02/27/12  The hardcoded SLCR divisors for Zynq are removed. The
*		      divisors are obtained from xparameters.h.c. The sleep
*		      values are reduced for Zynq. One sleep is added after
*		      MDIO divisor is set. Some of the prints are removed.
* 1.01a asa 03/14/12  The SLCR divisor support for ENET1 is added.
* 1.01a asa 04/15/12  The funcation calls to Xil_DisableMMU and Xil_EnableMMU
*		      are removed for setting the translation table
*		      attributes for the BD memory region.
* 1.05a asa 09/22/13 Cache handling is changed to fix an issue (CR#663885).
*			  The cache invalidation of the Rx frame is now moved to
*			  XEmacPsRecvHandler so that invalidation happens after the
*			  received data is available in the memory. The variable
*			  TxFrameLength is now made global.
* 2.1	srt 07/11/14 Implemented 64-bit changes and modified as per
*		      Zynq Ultrascale Mp GEM specification
* 3.0  kpc  01/23/14 Removed PEEP board related code
* 3.0  hk   03/18/15 Added support for jumbo frames.
*                    Add cache flush after BD terminate entries.
* 3.2  hk   10/15/15 Added clock control using CRL_APB_GEM_REF_CTRL register.
*                    Enabled 1G speed for ZynqMP GEM.
*                    Select GEM interrupt based on instance present.
*                    Manage differences between emulation platform and silicon.
* 3.2  mus  20/02/16.Added support for INTC interrupt controlller.
*                    Added support to access zynq emacps interrupt from
*                    microblaze.
* 3.3 kpc   12/09/16 Fixed issue when -O2 is enabled
* 3.4 ms    01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* 3.5 hk    08/14/17 Don't perform data cache operations when CCI is enabled
*                    on ZynqMP.
* 3.8 hk    10/01/18 Fix warning for redefinition of interrupt number.
* 3.9 hk    02/12/19 Change MDC divisor for Versal emulation.
*           03/06/19 Fix BD space assignment and its memory attributes.
*           03/20/19 Fix alignment pragmas for IAR compiler.
* 3.10 hk   05/17/19 Use correct platform register for Versal.
*           08/12/19 Add clock setup support for Versal.
*           14/08/19 Move definition of Platform to _util file for common use.
*           08/24/19 Add support for clock configuration in EL1 Non Secure for
*                    Versal.
* 3.14 sk   12/23/20 Remove documentation for None param in main function to fix
* 		     the doxygen warning.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xemacps_example.h"
#include "xil_exception.h"

#ifndef __MICROBLAZE__
#include "xil_mmu.h"
#endif

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
#include "xil_smc.h"
#endif
/*************************** Constant Definitions ***************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef __MICROBLAZE__
#define XPS_SYS_CTRL_BASEADDR	XPAR_PS7_SLCR_0_S_AXI_BASEADDR
#endif

#ifdef SDT
#define EMACPS_BASEADDR         XPAR_XEMACPS_0_BASEADDR
#elif XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define EMACPS_DEVICE_ID	XPAR_XEMACPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define INTC		XScuGic
#define EMACPS_DEVICE_ID	XPAR_XEMACPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#define ZYNQ_EMACPS_0_BASEADDR 0xE000B000
#define ZYNQ_EMACPS_1_BASEADDR 0xE000C000

#define ZYNQMP_EMACPS_0_BASEADDR 0xFF0B0000
#define ZYNQMP_EMACPS_1_BASEADDR 0xFF0C0000
#define ZYNQMP_EMACPS_2_BASEADDR 0xFF0D0000
#define ZYNQMP_EMACPS_3_BASEADDR 0xFF0E0000

#define VERSAL_EMACPS_0_BASEADDR 0xFF0C0000
#define VERSAL_EMACPS_1_BASEADDR 0xFF0D0000

#define VERSAL_NET_EMACPS_0_BASEADDR	0xF19E0000
#define VERSAL_NET_EMACPS_1_BASEADDR	0xF19F0000

#define RXBD_CNT       32	/* Number of RxBDs to use */
#define TXBD_CNT       32	/* Number of TxBDs to use */

/*
 * SLCR setting
 */
#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x144)


#define SLCR_LOCK_KEY_VALUE		0x767B
#define SLCR_UNLOCK_KEY_VALUE		0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL		(XPS_SYS_CTRL_BASEADDR + 0x214)

/* CRL APB registers for GEM clock control */
#ifdef XPAR_PSU_CRL_APB_S_AXI_BASEADDR
#define CRL_GEM0_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x50)
#define CRL_GEM1_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x54)
#define CRL_GEM2_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x58)
#define CRL_GEM3_REF_CTRL	(XPAR_PSU_CRL_APB_S_AXI_BASEADDR + 0x5C)
#endif

#define CRL_GEM_DIV_MASK	0x003F3F00
#define CRL_GEM_DIV0_SHIFT	8
#define CRL_GEM_DIV1_SHIFT	16

#ifdef XPAR_PSV_CRL_0_S_AXI_BASEADDR
#define CRL_GEM0_REF_CTRL	(XPAR_PSV_CRL_0_S_AXI_BASEADDR + 0x118)
#define CRL_GEM1_REF_CTRL	(XPAR_PSV_CRL_0_S_AXI_BASEADDR + 0x11C)
#endif

#ifdef XPAR_PSX_CRL_0_S_AXI_BASEADDR
#define CRL_GEM0_REF_CTRL  ( XPAR_PSX_CRL_0_S_AXI_BASEADDR + 0x118)
#define CRL_GEM1_REF_CTRL  ( XPAR_PSX_CRL_0_S_AXI_BASEADDR + 0x11C)
#endif

#define CRL_GEM_DIV_VERSAL_MASK		0x0003FF00
#define CRL_GEM_DIV_VERSAL_SHIFT	8

#define JUMBO_FRAME_SIZE	10240
#define FRAME_HDR_SIZE		18

#define GEMVERSION_ZYNQMP	0x7
#define GEMVERSION_VERSAL	0x107

/*************************** Variable Definitions ***************************/

#ifdef __ICCARM__
#pragma data_alignment = 64
EthernetFrame TxFrame;		/* Transmit buffer */
#pragma data_alignment = 64
EthernetFrame RxFrame;		/* Receive buffer */
#else
EthernetFrame TxFrame;		/* Transmit buffer */
EthernetFrame RxFrame;		/* Receive buffer */
#endif

/*
 * Buffer descriptors are allocated in uncached memory. The memory is made
 * uncached by setting the attributes appropriately in the MMU table.
 */
#define RXBD_SPACE_BYTES XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, RXBD_CNT)
#define TXBD_SPACE_BYTES XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, TXBD_CNT)


/*
 * Buffer descriptors are allocated in uncached memory. The memory is made
 * uncached by setting the attributes appropriately in the MMU table.
 * The minimum region for which attribute settings take effect is 2MB for
 * arm 64 variants(A53) and 1MB for the rest (R5 and A9). Hence the same
 * is allocated, even if not used fully by this example, to make sure none
 * of the adjacent global memory is affected.
 */
#ifdef __ICCARM__
#if defined __aarch64__
#pragma data_alignment = 0x200000
u8 bd_space[0x200000];
#else
#pragma data_alignment = 0x100000
u8 bd_space[0x100000];
#endif
#else
#if defined __aarch64__
u8 bd_space[0x200000] __attribute__ ((aligned (0x200000)));
#else
u8 bd_space[0x100000] __attribute__ ((aligned (0x100000)));
#endif
#endif

u8 *RxBdSpacePtr;
u8 *TxBdSpacePtr;

#define FIRST_FRAGMENT_SIZE 64

/*
 * Counters to be incremented by callbacks
 */
volatile s32 FramesRx;		/* Frames have been received */
volatile s32 FramesTx;		/* Frames have been sent */
volatile s32 DeviceErrors;	/* Number of errors detected in the device */

u32 TxFrameLength;

#ifndef SDT
#ifndef TESTAPP_GEN
static INTC IntcInstance;
#endif
#endif

#ifdef __ICCARM__
#pragma data_alignment = 64
XEmacPs_Bd BdTxTerminate;
#pragma data_alignment = 64
XEmacPs_Bd BdRxTerminate;
#else
XEmacPs_Bd BdTxTerminate __attribute__ ((aligned(64)));

XEmacPs_Bd BdRxTerminate __attribute__ ((aligned(64)));
#endif

u32 GemVersion;

/*************************** Function Prototypes ****************************/

/*
 * Example
 */
#ifdef SDT
LONG EmacPsDmaIntrExample(XEmacPs *EmacPsInstancePtr, UINTPTR BaseAddress);
#else
LONG EmacPsDmaIntrExample(INTC *IntcInstancePtr,
			  XEmacPs *EmacPsInstancePtr,
			  u16 EmacPsDeviceId);
#endif
LONG EmacPsDmaSingleFrameIntrExample(XEmacPs *EmacPsInstancePtr);

/*
 * Interrupt setup and Callbacks for examples
 */
#ifndef SDT
static LONG EmacPsSetupIntrSystem(INTC *IntcInstancePtr,
				  XEmacPs *EmacPsInstancePtr,
				  u16 EmacPsIntrId);

static void EmacPsDisableIntrSystem(INTC *IntcInstancePtr,
				    u16 EmacPsIntrId);
#endif
static void XEmacPsSendHandler(void *Callback);
static void XEmacPsRecvHandler(void *Callback);
static void XEmacPsErrorHandler(void *Callback, u8 direction, u32 word);

/*
 * Utility routines
 */
static LONG EmacPsResetDevice(XEmacPs *EmacPsInstancePtr);
void XEmacPsClkSetup(XEmacPs *EmacPsInstancePtr, u16 EmacPsIntrId);
void XEmacPs_SetMdioDivisor(XEmacPs *InstancePtr, XEmacPs_MdcDiv Divisor);
/****************************************************************************/
/**
*
* This is the main function for the EmacPs example. This function is not
* included if the example is generated from the TestAppGen test tool.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	LONG Status;

	xil_printf("Entering into main() \r\n");

	/*
	 * Call the EmacPs DMA interrupt example , specify the parameters
	 * generated in xparameters.h
	 */
#ifdef SDT
	Status = EmacPsDmaIntrExample(&EmacPsInstance,
				      EMACPS_BASEADDR);
#else
	Status = EmacPsDmaIntrExample(&IntcInstance,
				      &EmacPsInstance,
				      EMACPS_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Emacps intr dma Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emacps intr dma Example\r\n");
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* This function demonstrates the usage of the EmacPs driver by sending by
* sending and receiving frames in interrupt driven DMA mode.
*
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc driver.
* @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
*		driver.
* @param	EmacPsDeviceId is Device ID of the EmacPs Device , typically
*		XPAR_<EMACPS_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
#ifdef SDT
LONG EmacPsDmaIntrExample( XEmacPs *EmacPsInstancePtr, UINTPTR BaseAddress)
#else
LONG EmacPsDmaIntrExample(INTC *IntcInstancePtr,
			  XEmacPs *EmacPsInstancePtr,
			  u16 EmacPsDeviceId)
#endif
{
	LONG Status;
	XEmacPs_Config *Config;
	XEmacPs_Bd BdTemplate;
	u16 EmacPsIntrId;

	/*************************************/
	/* Setup device for first-time usage */
	/*************************************/

	/*
	 *  Initialize instance. Should be configured for DMA
	 *  This example calls _CfgInitialize instead of _Initialize due to
	 *  retiring _Initialize. So in _CfgInitialize we use
	 *  XPAR_(IP)_BASEADDRESS to make sure it is not virtual address.
	 */
#ifdef SDT
	Config = XEmacPs_LookupConfig(BaseAddress);
#else
	Config = XEmacPs_LookupConfig(EmacPsDeviceId);
#endif

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	/* Request device to indicate it is in use by this application */
	if (Config->BaseAddress == VERSAL_EMACPS_0_BASEADDR) {
		Xil_Smc(PM_REQUEST_DEVICE_SMC_FID, DEV_GEM_0, 1, 0, 100, 1, 0, 0);
	}
	if (Config->BaseAddress == VERSAL_EMACPS_1_BASEADDR) {
		Xil_Smc(PM_REQUEST_DEVICE_SMC_FID, DEV_GEM_1, 1, 0, 100, 1, 0, 0);
	}
#endif

	Status = XEmacPs_CfgInitialize(EmacPsInstancePtr, Config,
				       Config->BaseAddress);

	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in initialize");
		return XST_FAILURE;
	}

#if defined(XPAR_INTC_0_DEVICE_ID)
	EmacPsIntrId = XPAR_AXI_INTC_0_PROCESSING_SYSTEM7_0_IRQ_P2F_ENET0_INTR;
#else
	if ((EmacPsInstancePtr->Config.BaseAddress == ZYNQ_EMACPS_0_BASEADDR) ||
	    (EmacPsInstancePtr->Config.BaseAddress == ZYNQMP_EMACPS_0_BASEADDR) ||
	    (EmacPsInstancePtr->Config.BaseAddress == VERSAL_EMACPS_0_BASEADDR) ||
	    (EmacPsInstancePtr->Config.BaseAddress == VERSAL_NET_EMACPS_0_BASEADDR)) {
		EmacPsIntrId = XPS_GEM0_INT_ID;
	}
	else if ((EmacPsInstancePtr->Config.BaseAddress == ZYNQ_EMACPS_1_BASEADDR) ||
		 (EmacPsInstancePtr->Config.BaseAddress == ZYNQMP_EMACPS_1_BASEADDR) ||
		 (EmacPsInstancePtr->Config.BaseAddress == VERSAL_EMACPS_1_BASEADDR) ||
		 (EmacPsInstancePtr->Config.BaseAddress == VERSAL_NET_EMACPS_1_BASEADDR)) {
		EmacPsIntrId = XPS_GEM1_INT_ID;
	}
	else if (EmacPsInstancePtr->Config.BaseAddress == ZYNQMP_EMACPS_2_BASEADDR) {
#ifdef XPS_GEM2_INT_ID
		EmacPsIntrId = XPS_GEM2_INT_ID;
#endif
	}
	else if (EmacPsInstancePtr->Config.BaseAddress == ZYNQMP_EMACPS_3_BASEADDR) {
#ifdef XPS_GEM3_INT_ID
		EmacPsIntrId = XPS_GEM3_INT_ID;
#endif
	}
#endif

	GemVersion = ((Xil_In32(Config->BaseAddress + 0xFC)) >> 16) & 0xFFF;

	if (GemVersion == GEMVERSION_VERSAL) {
		Platform = Xil_In32(VERSAL_VERSION);
	}
	else if (GemVersion > 2) {
		Platform = Xil_In32(CSU_VERSION);
	}
	/* Enable jumbo frames for zynqmp */
	if (GemVersion > 2) {
		XEmacPs_SetOptions(EmacPsInstancePtr, XEMACPS_JUMBO_ENABLE_OPTION);
	}

	XEmacPsClkSetup(EmacPsInstancePtr, EmacPsIntrId);

	/*
	 * Set the MAC address
	 */
	Status = XEmacPs_SetMacAddress(EmacPsInstancePtr, EmacPsMAC, 1);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting MAC address");
		return XST_FAILURE;
	}
	/*
	 * Setup callbacks
	 */
	Status = XEmacPs_SetHandler(EmacPsInstancePtr,
				    XEMACPS_HANDLER_DMASEND,
				    (void *) XEmacPsSendHandler,
				    EmacPsInstancePtr);
	Status |=
		XEmacPs_SetHandler(EmacPsInstancePtr,
				   XEMACPS_HANDLER_DMARECV,
				   (void *) XEmacPsRecvHandler,
				   EmacPsInstancePtr);
	Status |=
		XEmacPs_SetHandler(EmacPsInstancePtr, XEMACPS_HANDLER_ERROR,
				   (void *) XEmacPsErrorHandler,
				   EmacPsInstancePtr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error assigning handlers");
		return XST_FAILURE;
	}

	/* Gem IP version on Zynq-7000 */
	if (GemVersion == 2) {
		/*
		 * The BDs need to be allocated in uncached memory. Hence the 1 MB
		 * address range that starts at "bd_space" is made uncached.
		 */
#if !defined(__MICROBLAZE__) && !defined(ARMR5)
		Xil_SetTlbAttributes((INTPTR)bd_space, DEVICE_MEMORY);
#else
		Xil_DCacheDisable();
#endif

	}

	if (GemVersion > 2) {

#if defined (ARMR5)
		Xil_SetTlbAttributes((INTPTR)bd_space, STRONG_ORDERD_SHARED |
				     PRIV_RW_USER_RW);
#endif
#if defined __aarch64__
		Xil_SetTlbAttributes((UINTPTR)bd_space, NORM_NONCACHE |
				     INNER_SHAREABLE);
#endif
	}

	/* Allocate Rx and Tx BD space each */
	RxBdSpacePtr = &(bd_space[0]);
	TxBdSpacePtr = &(bd_space[0x10000]);

	/*
	 * Setup RxBD space.
	 *
	 * We have already defined a properly aligned area of memory to store
	 * RxBDs at the beginning of this source code file so just pass its
	 * address into the function. No MMU is being used so the physical
	 * and virtual addresses are the same.
	 *
	 * Setup a BD template for the Rx channel. This template will be
	 * copied to every RxBD. We will not have to explicitly set these
	 * again.
	 */
	XEmacPs_BdClear(&BdTemplate);

	/*
	 * Create the RxBD ring
	 */
	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing
					(EmacPsInstancePtr)),
				      (UINTPTR) RxBdSpacePtr,
				      (UINTPTR) RxBdSpacePtr,
				      XEMACPS_BD_ALIGNMENT,
				      RXBD_CNT);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up RxBD space, BdRingCreate");
		return XST_FAILURE;
	}

	Status = XEmacPs_BdRingClone(&(XEmacPs_GetRxRing(EmacPsInstancePtr)),
				     &BdTemplate, XEMACPS_RECV);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up RxBD space, BdRingClone");
		return XST_FAILURE;
	}

	/*
	 * Setup TxBD space.
	 *
	 * Like RxBD space, we have already defined a properly aligned area
	 * of memory to use.
	 *
	 * Also like the RxBD space, we create a template. Notice we don't
	 * set the "last" attribute. The example will be overriding this
	 * attribute so it does no good to set it up here.
	 */
	XEmacPs_BdClear(&BdTemplate);
	XEmacPs_BdSetStatus(&BdTemplate, XEMACPS_TXBUF_USED_MASK);

	/*
	 * Create the TxBD ring
	 */
	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing
					(EmacPsInstancePtr)),
				      (UINTPTR) TxBdSpacePtr,
				      (UINTPTR) TxBdSpacePtr,
				      XEMACPS_BD_ALIGNMENT,
				      TXBD_CNT);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up TxBD space, BdRingCreate");
		return XST_FAILURE;
	}
	Status = XEmacPs_BdRingClone(&(XEmacPs_GetTxRing(EmacPsInstancePtr)),
				     &BdTemplate, XEMACPS_SEND);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up TxBD space, BdRingClone");
		return XST_FAILURE;
	}

	if (GemVersion > 2) {
		/*
		 * This version of GEM supports priority queuing and the current
		 * driver is using tx priority queue 1 and normal rx queue for
		 * packet transmit and receive. The below code ensure that the
		 * other queue pointers are parked to known state for avoiding
		 * the controller to malfunction by fetching the descriptors
		 * from these queues.
		 */
		XEmacPs_BdClear(&BdRxTerminate);
		XEmacPs_BdSetAddressRx(&BdRxTerminate, (XEMACPS_RXBUF_NEW_MASK |
							XEMACPS_RXBUF_WRAP_MASK));
		XEmacPs_Out32((Config->BaseAddress + XEMACPS_RXQ1BASE_OFFSET),
			      (UINTPTR)&BdRxTerminate);
		XEmacPs_BdClear(&BdTxTerminate);
		XEmacPs_BdSetStatus(&BdTxTerminate, (XEMACPS_TXBUF_USED_MASK |
						     XEMACPS_TXBUF_WRAP_MASK));
		XEmacPs_Out32((Config->BaseAddress + XEMACPS_TXQBASE_OFFSET),
			      (UINTPTR)&BdTxTerminate);
		if (Config->IsCacheCoherent == 0) {
			Xil_DCacheFlushRange((UINTPTR)(&BdTxTerminate), 64);
		}
	}

	/*
	 * Set emacps to phy loopback
	 */
	if (GemVersion == 2) {
		XEmacPs_SetMdioDivisor(EmacPsInstancePtr, MDC_DIV_224);
		EmacpsDelay(1);
		EmacPsUtilEnterLoopback(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED_1G);
		XEmacPs_SetOperatingSpeed(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED_1G);
	}
	else {
		if ((Platform & PLATFORM_MASK_VERSAL) == PLATFORM_VERSALEMU) {
			XEmacPs_SetMdioDivisor(EmacPsInstancePtr, MDC_DIV_8);
		}
		else {
			XEmacPs_SetMdioDivisor(EmacPsInstancePtr, MDC_DIV_224);
		}

		if (((Platform & PLATFORM_MASK) == PLATFORM_SILICON) ||
		    ((Platform & PLATFORM_MASK_VERSAL) == PLATFORM_VERSALSIL)) {
			EmacPsUtilEnterLoopback(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED_1G);
			XEmacPs_SetOperatingSpeed(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED_1G);
		}
		else {
			EmacPsUtilEnterLoopback(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED);
			XEmacPs_SetOperatingSpeed(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED);
		}
	}

	/*
	 * Setup the interrupt controller and enable interrupts
	 */
#ifdef SDT
	Status = XSetupInterruptSystem(EmacPsInstancePtr, &XEmacPs_IntrHandler,
				       EmacPsInstancePtr->Config.IntrId,
				       EmacPsInstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#else
	Status = EmacPsSetupIntrSystem(IntcInstancePtr,
				       EmacPsInstancePtr, EmacPsIntrId);
#endif
	/*
	 * Run the EmacPs DMA Single Frame Interrupt example
	 */
	Status = EmacPsDmaSingleFrameIntrExample(EmacPsInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the interrupts for the EmacPs device
	 */
#ifdef SDT
	XDisconnectInterruptCntrl(EmacPsInstancePtr->Config.IntrId,
				  EmacPsInstancePtr->Config.IntrParent);
#else
	EmacPsDisableIntrSystem(IntcInstancePtr, EmacPsIntrId);
#endif
	/*
	 * Stop the device
	 */
	XEmacPs_Stop(EmacPsInstancePtr);

	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function demonstrates the usage of the EMACPS by sending and
* receiving a single frame in DMA interrupt mode.
* The source packet will be described by two descriptors. It will be
* received into a buffer described by a single descriptor.
*
* @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
*		driver.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
LONG EmacPsDmaSingleFrameIntrExample(XEmacPs *EmacPsInstancePtr)
{
	LONG Status;
	u32 PayloadSize = 1000;
	u32 NumRxBuf = 0;
	u32 RxFrLen;
	XEmacPs_Bd *Bd1Ptr;
	XEmacPs_Bd *BdRxPtr;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	DeviceErrors = 0;

	if (GemVersion > 2) {
		PayloadSize = (JUMBO_FRAME_SIZE - FRAME_HDR_SIZE);
	}
	/*
	 * Calculate the frame length (not including FCS)
	 */
	TxFrameLength = XEMACPS_HDR_SIZE + PayloadSize;

	/*
	 * Setup packet to be transmitted
	 */
	EmacPsUtilFrameHdrFormatMAC(&TxFrame, EmacPsMAC);
	EmacPsUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	EmacPsUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
		Xil_DCacheFlushRange((UINTPTR)&TxFrame, sizeof(EthernetFrame));
	}

	/*
	 * Clear out receive packet memory area
	 */
	EmacPsUtilFrameMemClear(&RxFrame);

	if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
		Xil_DCacheFlushRange((UINTPTR)&RxFrame, sizeof(EthernetFrame));
	}

	/*
	 * Allocate RxBDs since we do not know how many BDs will be used
	 * in advance, use RXBD_CNT here.
	 */
	Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetRxRing(EmacPsInstancePtr)),
				     1, &BdRxPtr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error allocating RxBD");
		return XST_FAILURE;
	}


	/*
	 * Setup the BD. The XEmacPs_BdRingClone() call will mark the
	 * "wrap" field for last RxBD. Setup buffer address to associated
	 * BD.
	 */

	XEmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)&RxFrame);

	/*
	 * Enqueue to HW
	 */
	Status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(EmacPsInstancePtr)),
				    1, BdRxPtr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error committing RxBD to HW");
		return XST_FAILURE;
	}
	/*
	 * Though the max BD size is 16 bytes for extended desc mode, performing
	 * cache flush for 64 bytes. which is equal to the cache line size.
	 */
	if (GemVersion > 2) {
		if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
			Xil_DCacheFlushRange((UINTPTR)BdRxPtr, 64);
		}
	}
	/*
	 * Allocate, setup, and enqueue 1 TxBDs. The first BD will
	 * describe the first 32 bytes of TxFrame and the rest of BDs
	 * will describe the rest of the frame.
	 *
	 * The function below will allocate 1 adjacent BDs with Bd1Ptr
	 * being set as the lead BD.
	 */
	Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetTxRing(EmacPsInstancePtr)),
				     1, &Bd1Ptr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error allocating TxBD");
		return XST_FAILURE;
	}

	/*
	 * Setup first TxBD
	 */
	XEmacPs_BdSetAddressTx(Bd1Ptr, (UINTPTR)&TxFrame);
	XEmacPs_BdSetLength(Bd1Ptr, TxFrameLength);
	XEmacPs_BdClearTxUsed(Bd1Ptr);
	XEmacPs_BdSetLast(Bd1Ptr);

	/*
	 * Enqueue to HW
	 */
	Status = XEmacPs_BdRingToHw(&(XEmacPs_GetTxRing(EmacPsInstancePtr)),
				    1, Bd1Ptr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error committing TxBD to HW");
		return XST_FAILURE;
	}
	if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
		Xil_DCacheFlushRange((UINTPTR)Bd1Ptr, 64);
	}
	/*
	 * Set the Queue pointers
	 */
	XEmacPs_SetQueuePtr(EmacPsInstancePtr, EmacPsInstancePtr->RxBdRing.BaseBdAddr, 0, XEMACPS_RECV);
	if (GemVersion > 2) {
		XEmacPs_SetQueuePtr(EmacPsInstancePtr, EmacPsInstancePtr->TxBdRing.BaseBdAddr, 1, XEMACPS_SEND);
	}
	else {
		XEmacPs_SetQueuePtr(EmacPsInstancePtr, EmacPsInstancePtr->TxBdRing.BaseBdAddr, 0, XEMACPS_SEND);
	}

	/*
	 * Start the device
	 */
	XEmacPs_Start(EmacPsInstancePtr);

	/* Start transmit */
	XEmacPs_Transmit(EmacPsInstancePtr);

	/*
	 * Wait for transmission to complete
	 */
	while (!FramesTx);

	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 1 to hardware, then there should
	 * be only 1 ready for post processing.
	 */
	if (XEmacPs_BdRingFromHwTx(&(XEmacPs_GetTxRing(EmacPsInstancePtr)),
				   1, &Bd1Ptr) == 0) {
		EmacPsUtilErrorTrap
		("TxBDs were not ready for post processing");
		return XST_FAILURE;
	}

	/*
	 * Examine the TxBDs.
	 *
	 * There isn't much to do. The only thing to check would be DMA
	 * exception bits. But this would also be caught in the error
	 * handler. So we just return these BDs to the free list.
	 */


	Status = XEmacPs_BdRingFree(&(XEmacPs_GetTxRing(EmacPsInstancePtr)),
				    1, Bd1Ptr);

	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
	}

	/*
	 * Wait for Rx indication
	 */
	while (!FramesRx);

	/*
	 * Now that the frame has been received, post process our RxBD.
	 * Since we have submitted to hardware, then there should be only 1
	 * ready for post processing.
	 */
	NumRxBuf = XEmacPs_BdRingFromHwRx(&(XEmacPs_GetRxRing
					    (EmacPsInstancePtr)), 1,
					  &BdRxPtr);
	if (0 == NumRxBuf) {
		EmacPsUtilErrorTrap("RxBD was not ready for post processing");
		return XST_FAILURE;
	}

	/*
	 * There is no device status to check. If there was a DMA error,
	 * it should have been reported to the error handler. Check the
	 * receive lengthi against the transmitted length, then verify
	 * the data.
	 */
	if (GemVersion > 2) {
		/* API to get correct RX frame size - jumbo or otherwise */
		RxFrLen = XEmacPs_GetRxFrameSize(EmacPsInstancePtr, BdRxPtr);
	}
	else {
		RxFrLen = XEmacPs_BdGetLength(BdRxPtr);
	}
	if (RxFrLen != TxFrameLength) {
		EmacPsUtilErrorTrap("Length mismatch");
		return XST_FAILURE;
	}

	if (EmacPsUtilFrameVerify(&TxFrame, &RxFrame) != 0) {
		EmacPsUtilErrorTrap("Data mismatch");
		return XST_FAILURE;
	}

	/*
	 * Return the RxBD back to the channel for later allocation. Free
	 * the exact number we just post processed.
	 */
	Status = XEmacPs_BdRingFree(&(XEmacPs_GetRxRing(EmacPsInstancePtr)),
				    NumRxBuf, BdRxPtr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error freeing up RxBDs");
		return XST_FAILURE;
	}

	/*
	 * Finished this example. If everything worked correctly, all TxBDs
	 * and RxBDs should be free for allocation. Stop the device.
	 */
	XEmacPs_Stop(EmacPsInstancePtr);

	return XST_SUCCESS;
}


/****************************************************************************/
/**
* This function resets the device but preserves the options set by the user.
*
* The descriptor list could be reinitialized with the same calls to
* XEmacPs_BdRingClone() as used in main(). Doing this is a matter of
* preference.
* In many cases, an OS may have resources tied up in the descriptors.
* Reinitializing in this case may bad for the OS since its resources may be
* permamently lost.
*
* @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
*		driver.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static LONG EmacPsResetDevice(XEmacPs *EmacPsInstancePtr)
{
	LONG Status = 0;
	u8 MacSave[6];
	u32 Options;
	XEmacPs_Bd BdTemplate;


	/*
	 * Stop device
	 */
	XEmacPs_Stop(EmacPsInstancePtr);

	/*
	 * Save the device state
	 */
	XEmacPs_GetMacAddress(EmacPsInstancePtr, &MacSave, 1);
	Options = XEmacPs_GetOptions(EmacPsInstancePtr);

	/*
	 * Stop and reset the device
	 */
	XEmacPs_Reset(EmacPsInstancePtr);

	/*
	 * Restore the state
	 */
	XEmacPs_SetMacAddress(EmacPsInstancePtr, &MacSave, 1);
	Status |= XEmacPs_SetOptions(EmacPsInstancePtr, Options);
	Status |= XEmacPs_ClearOptions(EmacPsInstancePtr, ~Options);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error restoring state after reset");
		return XST_FAILURE;
	}

	/*
	 * Setup callbacks
	 */
	Status = XEmacPs_SetHandler(EmacPsInstancePtr,
				    XEMACPS_HANDLER_DMASEND,
				    (void *) XEmacPsSendHandler,
				    EmacPsInstancePtr);
	Status |= XEmacPs_SetHandler(EmacPsInstancePtr,
				     XEMACPS_HANDLER_DMARECV,
				     (void *) XEmacPsRecvHandler,
				     EmacPsInstancePtr);
	Status |= XEmacPs_SetHandler(EmacPsInstancePtr, XEMACPS_HANDLER_ERROR,
				     (void *) XEmacPsErrorHandler,
				     EmacPsInstancePtr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error assigning handlers");
		return XST_FAILURE;
	}

	/*
	 * Setup RxBD space.
	 *
	 * We have already defined a properly aligned area of memory to store
	 * RxBDs at the beginning of this source code file so just pass its
	 * address into the function. No MMU is being used so the physical and
	 * virtual addresses are the same.
	 *
	 * Setup a BD template for the Rx channel. This template will be copied
	 * to every RxBD. We will not have to explicitly set these again.
	 */
	XEmacPs_BdClear(&BdTemplate);

	/*
	 * Create the RxBD ring
	 */
	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing
					(EmacPsInstancePtr)),
				      (UINTPTR) RxBdSpacePtr,
				      (UINTPTR) RxBdSpacePtr,
				      XEMACPS_BD_ALIGNMENT,
				      RXBD_CNT);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up RxBD space, BdRingCreate");
		return XST_FAILURE;
	}

	Status = XEmacPs_BdRingClone(&
				     (XEmacPs_GetRxRing(EmacPsInstancePtr)),
				     &BdTemplate, XEMACPS_RECV);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up RxBD space, BdRingClone");
		return XST_FAILURE;
	}

	/*
	 * Setup TxBD space.
	 *
	 * Like RxBD space, we have already defined a properly aligned area of
	 * memory to use.
	 *
	 * Also like the RxBD space, we create a template. Notice we don't set
	 * the "last" attribute. The examples will be overriding this
	 * attribute so it does no good to set it up here.
	 */
	XEmacPs_BdClear(&BdTemplate);
	XEmacPs_BdSetStatus(&BdTemplate, XEMACPS_TXBUF_USED_MASK);

	/*
	 * Create the TxBD ring
	 */
	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing
					(EmacPsInstancePtr)),
				      (UINTPTR) TxBdSpacePtr,
				      (UINTPTR) TxBdSpacePtr,
				      XEMACPS_BD_ALIGNMENT,
				      TXBD_CNT);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up TxBD space, BdRingCreate");
		return XST_FAILURE;
	}
	Status = XEmacPs_BdRingClone(&
				     (XEmacPs_GetTxRing(EmacPsInstancePtr)),
				     &BdTemplate, XEMACPS_SEND);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Error setting up TxBD space, BdRingClone");
		return XST_FAILURE;
	}

	/*
	 * Restart the device
	 */
	XEmacPs_Start(EmacPsInstancePtr);

	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* EMACPS.
* @param	IntcInstancePtr is a pointer to the instance of the Intc driver.
* @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
*		driver.
* @param	EmacPsIntrId is the Interrupt ID and is typically
*		XPAR_<EMACPS_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
static LONG EmacPsSetupIntrSystem(INTC *IntcInstancePtr,
				  XEmacPs *EmacPsInstancePtr,
				  u16 EmacPsIntrId)
{
	LONG Status;

#ifdef XPAR_INTC_0_DEVICE_ID

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, EmacPsIntrId,
			       (XInterruptHandler) XEmacPs_IntrHandler, EmacPsInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */

	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt from the hardware
	 */
	XIntc_Enable(IntcInstancePtr, EmacPsIntrId);

#ifndef TESTAPP_GEN
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler) XIntc_InterruptHandler,
				     IntcInstancePtr);
#endif

#else
#ifndef TESTAPP_GEN
	XScuGic_Config *GicConfig;
	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, GicConfig,
				       GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     IntcInstancePtr);
#endif

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, EmacPsIntrId,
				 (Xil_InterruptHandler) XEmacPs_IntrHandler,
				 (void *) EmacPsInstancePtr);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap
		("Unable to connect ISR to interrupt controller");
		return XST_FAILURE;
	}

	/*
	 * Enable interrupts from the hardware
	 */
	XScuGic_Enable(IntcInstancePtr, EmacPsIntrId);
#endif
#ifndef TESTAPP_GEN
	/*
	 * Enable interrupts in the processor
	 */
	Xil_ExceptionEnable();
#endif
	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function disables the interrupts that occur for EmacPs.
*
* @param	IntcInstancePtr is the pointer to the instance of the ScuGic
*		driver.
* @param	EmacPsIntrId is interrupt ID and is typically
*		XPAR_<EMACPS_instance>_INTR value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void EmacPsDisableIntrSystem(INTC *IntcInstancePtr,
				    u16 EmacPsIntrId)
{
	/*
	 * Disconnect and disable the interrupt for the EmacPs device
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, EmacPsIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, EmacPsIntrId);
#endif

}
#endif
/****************************************************************************/
/**
*
* This the Transmit handler callback function and will increment a shared
* counter that can be shared by the main thread of operation.
*
* @param	Callback is the pointer to the instance of the EmacPs device.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XEmacPsSendHandler(void *Callback)
{
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	/*
	 * Disable the transmit related interrupts
	 */
	XEmacPs_IntDisable(EmacPsInstancePtr, (XEMACPS_IXR_TXCOMPL_MASK |
					       XEMACPS_IXR_TX_ERR_MASK));
	if (GemVersion > 2) {
		XEmacPs_IntQ1Disable(EmacPsInstancePtr, XEMACPS_INTQ1_IXR_ALL_MASK);
	}
	/*
	 * Increment the counter so that main thread knows something
	 * happened.
	 */
	FramesTx++;
}


/****************************************************************************/
/**
*
* This is the Receive handler callback function and will increment a shared
* counter that can be shared by the main thread of operation.
*
* @param	Callback is a pointer to the instance of the EmacPs device.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XEmacPsRecvHandler(void *Callback)
{
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	/*
	 * Disable the transmit related interrupts
	 */
	XEmacPs_IntDisable(EmacPsInstancePtr, (XEMACPS_IXR_FRAMERX_MASK |
					       XEMACPS_IXR_RX_ERR_MASK));
	/*
	 * Increment the counter so that main thread knows something
	 * happened.
	 */
	FramesRx++;
	if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
		Xil_DCacheInvalidateRange((UINTPTR)&RxFrame, sizeof(EthernetFrame));
	}
	if (GemVersion > 2) {
		if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
			Xil_DCacheInvalidateRange((UINTPTR)RxBdSpacePtr, 64);
		}
	}
}


/****************************************************************************/
/**
*
* This is the Error handler callback function and this function increments
* the error counter so that the main thread knows the number of errors.
*
* @param	Callback is the callback function for the driver. This
*		parameter is not used in this example.
* @param	Direction is passed in from the driver specifying which
*		direction error has occurred.
* @param	ErrorWord is the status register value passed in.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XEmacPsErrorHandler(void *Callback, u8 Direction, u32 ErrorWord)
{
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	/*
	 * Increment the counter so that main thread knows something
	 * happened. Reset the device and reallocate resources ...
	 */
	DeviceErrors++;

	switch (Direction) {
		case XEMACPS_RECV:
			if (ErrorWord & XEMACPS_RXSR_HRESPNOK_MASK) {
				EmacPsUtilErrorTrap("Receive DMA error");
			}
			if (ErrorWord & XEMACPS_RXSR_RXOVR_MASK) {
				EmacPsUtilErrorTrap("Receive over run");
			}
			if (ErrorWord & XEMACPS_RXSR_BUFFNA_MASK) {
				EmacPsUtilErrorTrap("Receive buffer not available");
			}
			break;
		case XEMACPS_SEND:
			if (ErrorWord & XEMACPS_TXSR_HRESPNOK_MASK) {
				EmacPsUtilErrorTrap("Transmit DMA error");
			}
			if (ErrorWord & XEMACPS_TXSR_URUN_MASK) {
				EmacPsUtilErrorTrap("Transmit under run");
			}
			if (ErrorWord & XEMACPS_TXSR_BUFEXH_MASK) {
				EmacPsUtilErrorTrap("Transmit buffer exhausted");
			}
			if (ErrorWord & XEMACPS_TXSR_RXOVR_MASK) {
				EmacPsUtilErrorTrap("Transmit retry excessed limits");
			}
			if (ErrorWord & XEMACPS_TXSR_FRAMERX_MASK) {
				EmacPsUtilErrorTrap("Transmit collision");
			}
			if (ErrorWord & XEMACPS_TXSR_USEDREAD_MASK) {
				EmacPsUtilErrorTrap("Transmit buffer not available");
			}
			break;
	}
	/*
	 * Bypassing the reset functionality as the default tx status for q0 is
	 * USED BIT READ. so, the first interrupt will be tx used bit and it resets
	 * the core always.
	 */
	if (GemVersion == 2) {
		EmacPsResetDevice(EmacPsInstancePtr);
	}
}

/****************************************************************************/
/**
*
* This function sets up the clock divisors for 1000Mbps.
*
* @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
*			driver.
* @param	EmacPsIntrId is the Interrupt ID and is typically
*			XPAR_<EMACPS_instance>_INTR value from xparameters.h.
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacPsClkSetup(XEmacPs *EmacPsInstancePtr, u16 EmacPsIntrId)
{
	u32 ClkCntrl;
	u32 BaseAddress = EmacPsInstancePtr->Config.BaseAddress;

	if (GemVersion == 2) {
		/*************************************/
		/* Setup device for first-time usage */
		/*************************************/

		/* SLCR unlock */
		*(volatile unsigned int *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;
		if (BaseAddress == ZYNQ_EMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
			/* GEM0 1G clock configuration*/
			ClkCntrl =
				*(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR);
			ClkCntrl &= EMACPS_SLCR_DIV_MASK;
			ClkCntrl |= (EmacPsInstancePtr->Config.S1GDiv1 << 20);
			ClkCntrl |= (EmacPsInstancePtr->Config.S1GDiv0 << 8);
			*(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR) =
				ClkCntrl;
#endif
		}
		else if (BaseAddress == ZYNQ_EMACPS_1_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1
			/* GEM1 1G clock configuration*/
			ClkCntrl =
				*(volatile unsigned int *)(SLCR_GEM1_CLK_CTRL_ADDR);
			ClkCntrl &= EMACPS_SLCR_DIV_MASK;
			ClkCntrl |= (EmacPsInstancePtr->Config.S1GDiv1 << 20);
			ClkCntrl |= (EmacPsInstancePtr->Config.S1GDiv0 << 8);
			*(volatile unsigned int *)(SLCR_GEM1_CLK_CTRL_ADDR) =
				ClkCntrl;
#endif
		}
		/* SLCR lock */
		*(unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
#ifndef __MICROBLAZE__
		sleep(1);
#else
		unsigned long count = 0;
		while (count < 0xffff) {
			count++;
		}
#endif
	}

	if ((GemVersion == GEMVERSION_ZYNQMP) && ((Platform & PLATFORM_MASK) == PLATFORM_SILICON)) {

#ifdef XPAR_PSU_CRL_APB_S_AXI_BASEADDR
		if (BaseAddress == ZYNQMP_EMACPS_0_BASEADDR) {
			/* GEM0 1G clock configuration*/
			ClkCntrl =
				*(volatile unsigned int *)(CRL_GEM0_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv1 << CRL_GEM_DIV1_SHIFT;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV0_SHIFT;
			*(volatile unsigned int *)(CRL_GEM0_REF_CTRL) =
				ClkCntrl;

		}
		if (BaseAddress == ZYNQMP_EMACPS_1_BASEADDR) {

			/* GEM1 1G clock configuration*/
			ClkCntrl =
				*(volatile unsigned int *)(CRL_GEM1_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv1 << CRL_GEM_DIV1_SHIFT;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV0_SHIFT;
			*(volatile unsigned int *)(CRL_GEM1_REF_CTRL) =
				ClkCntrl;
		}
		if (BaseAddress == ZYNQMP_EMACPS_2_BASEADDR) {

			/* GEM2 1G clock configuration*/
			ClkCntrl =
				*(volatile unsigned int *)(CRL_GEM2_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv1 << CRL_GEM_DIV1_SHIFT;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV0_SHIFT;
			*(volatile unsigned int *)(CRL_GEM2_REF_CTRL) =
				ClkCntrl;

		}
		if (BaseAddress == ZYNQMP_EMACPS_3_BASEADDR) {
			/* GEM3 1G clock configuration*/
			ClkCntrl =
				*(volatile unsigned int *)(CRL_GEM3_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv1 << CRL_GEM_DIV1_SHIFT;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV0_SHIFT;
			*(volatile unsigned int *)(CRL_GEM3_REF_CTRL) =
				ClkCntrl;
		}
#endif
	}
	if ((GemVersion == GEMVERSION_VERSAL) &&
	    ((Platform & PLATFORM_MASK_VERSAL) == PLATFORM_VERSALSIL)) {

#ifdef XPAR_PSV_CRL_0_S_AXI_BASEADDR
		if (BaseAddress == VERSAL_EMACPS_0_BASEADDR) {
			/* GEM0 1G clock configuration*/
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
			Xil_Smc(PM_SET_DIVIDER_SMC_FID, (((u64)EmacPsInstancePtr->Config.S1GDiv0 << 32) | CLK_GEM0_REF), 0, 0, 0, 0, 0, 0);
#else
			ClkCntrl = Xil_In32((UINTPTR)CRL_GEM0_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_VERSAL_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV_VERSAL_SHIFT;
			Xil_Out32((UINTPTR)CRL_GEM0_REF_CTRL, ClkCntrl);
#endif
		}
		if (BaseAddress == VERSAL_EMACPS_1_BASEADDR) {

			/* GEM1 1G clock configuration*/
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
			Xil_Smc(PM_SET_DIVIDER_SMC_FID, (((u64)EmacPsInstancePtr->Config.S1GDiv0 << 32) | CLK_GEM1_REF), 0, 0, 0, 0, 0, 0);
#else
			ClkCntrl = Xil_In32((UINTPTR)CRL_GEM1_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_VERSAL_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV_VERSAL_SHIFT;
			Xil_Out32((UINTPTR)CRL_GEM1_REF_CTRL, ClkCntrl);
#endif
		}
#endif

#ifdef XPAR_PSX_CRL_0_S_AXI_BASEADDR
		if (BaseAddress == VERSAL_NET_EMACPS_0_BASEADDR) {
			/* GEM0 1G clock configuration*/
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
			Xil_Smc(PM_SET_DIVIDER_SMC_FID, (((u64)EmacPsInstancePtr->Config.S1GDiv0 << 32) | CLK_GEM0_REF), 0, 0, 0, 0, 0, 0);
#else
			ClkCntrl = Xil_In32((UINTPTR)CRL_GEM0_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_VERSAL_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV_VERSAL_SHIFT;
			Xil_Out32((UINTPTR)CRL_GEM0_REF_CTRL, ClkCntrl);
#endif
		}
		if (BaseAddress == VERSAL_NET_EMACPS_1_BASEADDR) {

			/* GEM1 1G clock configuration*/
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
			Xil_Smc(PM_SET_DIVIDER_SMC_FID, (((u64)EmacPsInstancePtr->Config.S1GDiv0 << 32) | CLK_GEM1_REF), 0, 0, 0, 0, 0, 0);
#else
			ClkCntrl = Xil_In32((UINTPTR)CRL_GEM1_REF_CTRL);
			ClkCntrl &= ~CRL_GEM_DIV_VERSAL_MASK;
			ClkCntrl |= EmacPsInstancePtr->Config.S1GDiv0 << CRL_GEM_DIV_VERSAL_SHIFT;
			Xil_Out32((UINTPTR)CRL_GEM1_REF_CTRL, ClkCntrl);
#endif
		}
#endif
	}
}
