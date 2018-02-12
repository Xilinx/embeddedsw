/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet.h
* @addtogroup xxvethernet_v1_4
* @{
* @details
*
* The Xilinx XXV Ethernet MAC driver component. This driver supports both
* XXV Ethernet core and USXGMII core on Zynq Ultrascale+ MPSoC.
* The MAC portion of USXMGII and XXV ethernet is similar.
* Speed supported for XXV Ethernet core is 10Gbps.
* Speed supported for USXGMII core is 1Gbps or 2.5Gbps.
*
* For a full description of XXV Ethernet features, please see the hardware
* spec.
* This driver supports the following features:
*   - Access to host interface registers
*   - Interfacing to target MCDMA device via application
*   - Full duplex operation
*   - Automatic PAD & FCS insertion and stripping (programmable)
*   - Jumbo frame support
*
* For full description of USXGMII features, please refer to hardware spec.
* In addition to the above MAC features, USXGMII core supports
* USXGMII PHY functionality. This driver supports the same.
*
* <h2>Driver Description</h2>
*
* The device driver enables higher layer software (e.g., an application) to
* configure an Xxv Ethernet device. It is intended that this driver be used in
* cooperation with another MCDMA driver for data communication..
*
* <h2>Initialization & Configuration</h2>
*
* The XXxvEthernet_Config structure can be used by the driver to configure
* itself. This configuration structure is typically created by the tool-chain
* based on hardware build properties.
*
* The driver instance can be initialized using the
* XXvEthernet_CfgInitialze() routine.
*
* <h2>Interrupts and Asynchronous Callbacks</h2>
*
* The driver has no dependencies on the interrupt controller. It provides
* no interrupt handlers. The application/OS software should set up its own
* interrupt handlers if required based on the target DMA device.
*
* <h2>Device Reset</h2>
*
* When an Xxv Ethernet device is connected up to a MCDMA core in hardware,
* reset is controlled by the latter. If a reset is performed,
* the calling code should also reconfigure and reapply the proper
* settings in the Xxv Ethernet device.
* When an Xxv Ethernet device reset is required, XXxvEthernet_Reset() should
* be utilized. For GT/Serdes reset, refer to the example.
*
* <h2>Transferring Data</h2>
*
* The Xxv Ethernet core by itself is not capable of transmitting or receiving
* data in any meaningful way. Instead the Xxv Ethernet device needs to be
* connected to a FIFO or DMA core in hardware, currently MCDMA is supported.
*
* This Xxv Ethernet driver is modeled in a similar fashion where the
* application code or O/S adapter driver needs to make use of a separate
* MCDMA driver in connection with this driver to establish meaningful
* communication over Ethernet.
*
* <h2>PHY Communication</h2>
*
* XXV Ethernet core:
* This device supports clause 49 10GBaseKR at the moment. There is no
* autonegotiation enabled by default - the  autonegotiation speed is limited
* to 10G.
*
* USXGMII core:
* This driver supports USXMGII autonegotiation at 1G and and 2.5G. The core
* also supports 10G and 5G speeds but it is not validated with this driver.
* There is also an option to bypass autonegotiation.
*
* <h2>Asserts</h2>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development. For deployment
* use -DNDEBUG compiler switch to remove assert code.
*
*
* @note
*
* Xilinx drivers are typically composed of two components, one is the driver
* and the other is the adapter.  The driver is independent of OS and processor
* and is intended to be highly portable.  The adapter is OS-specific and
* facilitates communication between the driver and an OS.
* <br><br>
* This driver is intended to be RTOS and processor independent. Any needs for
* dynamic memory management, threads or thread mutual exclusion, or cache
* control must be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.0   hk   6/16/17  First release
*       hk   2/15/18  Add support for USXGMII
* 1.1   mj   3/30/18  Add Macro XXxvEthernet_IsMcDma(InstancePtr) to check
*                     McDma is connected or not.
* 1.4   rsp  05/08/20 Include sleep.h header
* </pre>
*
******************************************************************************/

#ifndef XXXVETHERNET_H		/* prevent circular inclusions */
#define XXXVETHERNET_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "sleep.h"
#include "xenv.h"
#include "xstatus.h"
#include "xil_assert.h"
#include "xxxvethernet_hw.h"

/************************** Constant Definitions *****************************/

/** @name Configuration options
 *
 * The following are device configuration options. See the
 * <i>XXxvEthernet_SetOptions</i>, <i>XXxvEthernet_ClearOptions</i> and
 * <i>XXxvEthernet_GetOptions</i> routines for information on how to use
 * options.
 *
 * The default state of the options are also noted below.
 *
 * @{
 */

/**< XXE_FCS_STRIP_OPTION specifies the Xxv Ethernet device to strip FCS and
 *   PAD from received frames.
 *   This driver sets this option to enabled (set) by default.
 */
#define XXE_FCS_STRIP_OPTION		0x00000010

/**< XXE_FCS_INSERT_OPTION specifies the Xxv Ethernet device to generate the
 *   FCS field and add PAD automatically for outgoing frames.
 *   This driver sets this option to enabled (set) by default.
 */
#define XXE_FCS_INSERT_OPTION		0x00000020

/**< XXE_TRANSMITTER_ENABLE_OPTION specifies the Xxv Ethernet device
 *   transmitter to be enabled.
 *   This driver sets this option to enabled (set) by default.
 */
#define XXE_TRANSMITTER_ENABLE_OPTION	0x00000080

/**< XXE_RECEIVER_ENABLE_OPTION specifies the Xxv Ethernet device receiver to
 *   be enabled.
 *   This driver sets this option to enabled (set) by default.
 */
#define XXE_RECEIVER_ENABLE_OPTION	0x00000100

#define XXE_DEFAULT_OPTIONS				\
		(XXE_FCS_INSERT_OPTION |		\
		 XXE_FCS_STRIP_OPTION |			\
		 XXE_TRANSMITTER_ENABLE_OPTION | 	\
		 XXE_RECEIVER_ENABLE_OPTION)
/**< XXE_DEFAULT_OPTIONS specify the options set in XXxvEthernet_Reset() and
 *   XXxvEthernet_CfgInitialize()
 */
/*@}*/

/*
 * The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define XXE_MAC_ADDR_SIZE		6	/* MAC addresses are 6 bytes */
#define XXE_MTU				1500	/* Max MTU size of an Ethernet
						 * frame
						 */
#define XXE_JUMBO_MTU			8982	/* Max MTU size of a jumbo
						 * Ethernet frame
						 */
#define XXE_HDR_SIZE			14	/* Size of an Ethernet header*/

#define XXE_TRL_SIZE			4	/* Size of an Ethernet trailer
						 * (FCS)
						 */
#define XXE_MAX_FRAME_SIZE	 (XXE_MTU + XXE_HDR_SIZE + XXE_TRL_SIZE)
#define XXE_MAX_JUMBO_FRAME_SIZE (XXE_JUMBO_MTU + XXE_HDR_SIZE + XXE_TRL_SIZE)


#define XXE_RX				1 /* Receive direction  */
#define XXE_TX				2 /* Transmit direction */

#define RATE_10M	10
#define RATE_100M	100
#define RATE_1G		1000
#define RATE_2G5	2500
#define RATE_10G	10000

/**************************** Type Definitions *******************************/


/**
 * This typedef contains configuration information for a Xxv Ethernet device.
 */
typedef struct XXxvEthernet_Config {
	u16 DeviceId;	/**< DeviceId is the unique ID  of the device */
	UINTPTR BaseAddress;/**< BaseAddress is the physical base address of the
			  *  device's registers
			  */
	u8 Stats;	/**< Statistics gathering option */

	int XxvDevType;  /**< XxvDevType is the type of device attached to the
			  *   Xxv Ethernet's AXI4-Stream interface -
			  *   MCDMA in this case
			  */
	u32 XxvDevBaseAddress; /**< XxvDevBaseAddress is the base address of
				 *  the device attached to the Xxv Ethernet's
				 *  AXI4-Stream interface.
				 */
	u8 AxiMcDmaChan_Cnt;
	u8 AxiMcDmaRxIntr[16];
	u8 AxiMcDmaTxIntr[16];
} XXxvEthernet_Config;


/**
 * struct XXxvEthernet is the type for Xxv Ethernet driver instance data.
 * The calling code is required to use a unique instance of this structure
 * for every Xxv Ethernet device used in the system. A reference to a structure
 * of this type is then passed to the driver API functions.
 */
typedef struct XXxvEthernet {
	XXxvEthernet_Config Config; /**< Hardware configuration */
	u32 IsStarted;		 /**< Device is currently started */
	u32 IsReady;		 /**< Device is initialized and ready */
	u32 Options;		 /**< Current options word */
	u32 Flags;		 /**< Internal driver flags */
} XXxvEthernet;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* XXxvEthernet_IsStarted reports if the device is in the started or stopped
* state. To be in the started state, the calling code must have made a
* successful call to <i>XXxvEthernet_Start</i>. To be in the stopped state,
* <i>XXxvEthernet_Stop</i> or <i>XXxvEthernet_CfgInitialize</i> function must
* have been called.
*
* @param	InstancePtr is a pointer to the of Xxv Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if the device has been started.
*		- FALSE.if the device has not been started
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_IsStarted(XXxvEthernet *InstancePtr)
*
 ******************************************************************************/
#define XXxvEthernet_IsStarted(InstancePtr) \
	(((InstancePtr)->IsStarted == XIL_COMPONENT_IS_STARTED) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XXxvEthernet_XxvDevBaseAddress reports the base address of the core connected
* to the Xxv Ethernet's Axi4 Stream interface (expected to be MCDMA).
* This function is currently not in use because of limitation in exporting
* target MCDMA information.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return	The base address of the core connected to the Xxv Ethernet's
*		streaming interface.
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_XxvDevBaseAddress(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_XxvDevBaseAddress(InstancePtr) \
	((InstancePtr)->Config.XxvDevBaseAddress)

/*****************************************************************************/
/**
*
* XXxvEthernet_IsTxErr determines if there is any TX error reported by device.
* The TX status register is read for this to be determined.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if error bit mask is set
*		- FALSE if error bit mask is not set
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_IsTxErr(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_IsTxErr(InstancePtr)			 \
	((XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress,  \
	XXE_TXSR_OFFSET) & XXE_STS_TX_ERROR_MASK) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XXxvEthernet_IsRxErr determines if there is any RX error reported by device.
* The RX status register is read for this to be determined.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if error bit mask is set
*		- FALSE if error bit mask is not set
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_IsRxErr(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_IsRxErr(InstancePtr)			 \
	((XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress,  \
	XXE_RXSR_OFFSET) & XXE_STS_RX_ERROR_MASK) ? TRUE : FALSE

/****************************************************************************/
/**
*
* XXxvEthernet_GetStatus returns the contents of status register
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return	Returns the status register.
*
* @note		C-style signature:
*		u32 XXxvEthernet_GetStatus(XXxvEthernet *InstancePtr)
*
*****************************************************************************/
#define XXxvEthernet_GetStatus(InstancePtr) \
	 XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress, XXE_SR_OFFSET)

/*****************************************************************************/
/**
*
* XXxvEthernet_IsStatsConfigured returns determines if Statistics gathering.
* is configured in the harwdare or not.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return
*	 	- TRUE if the device is configured with
*		  statistics gathering.
*		- FALSE if the device is NOT configured with
*		  statistics gathering.
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_IsStatsConfigured(XXxvEthernet *InstancePtr)
*
 *****************************************************************************/
#define XXxvEthernet_IsStatsConfigured(InstancePtr)	\
	(((InstancePtr)->Config.Stats) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XXxvEthernet_UsxgmiiLinkSts returns the USXGMII link status
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @return
*		- TRUE if link is up
*		- FALSE if link is down
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_UsxgmiiLinkSts(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_UsxgmiiLinkSts(InstancePtr)			 \
	((XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress,  \
	XXE_USXGMII_AN_OFFSET) & XXE_USXGMII_LINK_STS_MASK) ? TRUE : FALSE)

/*****************************************************************************/
/**
*
* XXxvEthernet_SetUsxgmiiAnEnable enables USXGMII autonegotiation.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_SetUsxgmiiAnEnable(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_SetUsxgmiiAnEnable(InstancePtr)			\
	(XXxvEthernet_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XXE_USXGMII_AN_OFFSET,					\
		XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XXE_USXGMII_AN_OFFSET) | XXE_USXGMII_ANENABLE_MASK));

/*****************************************************************************/
/**
*
* XXxvEthernet_SetUsxgmiiAnBypass bypasses USXGMII autonegotiation.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_SetUsxgmiiAnBypass(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_SetUsxgmiiAnBypass(InstancePtr)			\
	(XXxvEthernet_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XXE_USXGMII_AN_OFFSET,					\
		XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XXE_USXGMII_AN_OFFSET) | XXE_USXGMII_ANBYPASS_MASK));

/*****************************************************************************/
/**
*
* XXxvEthernet_GetUsxgmiiAnSts returns the contents of USXGMII
* autonegotiation status register
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_GetUsxgmiiAnSts(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_GetUsxgmiiAnSts(InstancePtr)			\
	XXxvEthernet_ReadReg((InstancePtr)->Config.BaseAddress,		\
				XXE_ANSR_OFFSET);

/*****************************************************************************/
/**
*
* XXxvEthernet_IsMcDma reports if the device is currently connected to MCDMA.
*
* @param	InstancePtr is a pointer to the Xxv Ethernet instance to be
*		worked on.
* @return
*		- TRUE if the Xxv Ethernet device is connected MCDMA.
*		- FALSE.if the Xxv Ethernet device is NOT connected to MCDMA
*
* @note 	C-style signature:
* 		u32 XXxvEthernet_IsMcDma(XXxvEthernet *InstancePtr)
*
******************************************************************************/
#define XXxvEthernet_IsMcDma(InstancePtr) \
	(((InstancePtr)->Config.XxvDevType == XPAR_MCDMA) ? TRUE: FALSE)

/************************** Function Prototypes ******************************/

/*
 * Initialization functions in xxxvethernet.c
 */
int XXxvEthernet_CfgInitialize(XXxvEthernet *InstancePtr,
			XXxvEthernet_Config *CfgPtr,UINTPTR VirtualAddress);
int XXxvEthernet_Start(XXxvEthernet *InstancePtr);
void XXxvEthernet_Stop(XXxvEthernet *InstancePtr);
void XXxvEthernet_Reset(XXxvEthernet *InstancePtr);

/*
 * Initialization functions in xxxvtemac_sinit.c
 */
XXxvEthernet_Config *XXxvEthernet_LookupConfig(u16 DeviceId);
XXxvEthernet_Config *XXxvEthernet_LookupConfigBaseAddr(UINTPTR Baseaddr);

/*
 * MAC configuration/control functions in xxxvethernet.c
 */
int XXxvEthernet_SetOptions(XXxvEthernet *InstancePtr, u32 Options);
int XXxvEthernet_ClearOptions(XXxvEthernet *InstancePtr, u32 Options);
u32 XXxvEthernet_GetOptions(XXxvEthernet *InstancePtr);

u16 XXxvEthernet_GetAutoNegSpeed(XXxvEthernet *InstancePtr);
int XXxvEthernet_SetAutoNegSpeed(XXxvEthernet *InstancePtr);
int XXxvEthernet_SetUsxgmiiRateAndDuplex(XXxvEthernet *InstancePtr, u32 Rate, u32 SetFD);
void XXxvEthernet_UsxgmiiAnMainReset(XXxvEthernet *InstancePtr);
void XXxvEthernet_UsxgmiiAnMainRestart(XXxvEthernet *InstancePtr);
int XXxvEthernet_Initialize(XXxvEthernet *InstancePtr,
			    XXxvEthernet_Config *CfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
