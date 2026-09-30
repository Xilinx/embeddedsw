/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xaxidma.h
 * @brief Defines the driver API for the AXI DMA engine.
*
* For a full description of DMA features, please see the hardware spec. This
* driver supports the following features:
*
*   - Scatter-Gather DMA (SGDMA)
*   - Simple DMA
*   - Interrupts
*   - Programmable interrupt coalescing for SGDMA
*   - APIs to manage Buffer Descriptors (BD) movement to and from the SGDMA
*     engine
*
* \par Simple DMA
*
* Simple DMA allows the application to specify a single transaction between DMA
* and Device. It has two channels: one from the DMA to Device and the other
* from Device to DMA. Application has to set the buffer address and
* length fields to initiate the transfer in respective channel.
*
* \par Transactions
*
* The object used to describe a transaction is referred to as a Buffer
* Descriptor (BD). Buffer descriptors are allocated in the user application.
* The user application needs to set buffer address, transfer length, and
* control information for this transfer. The control information includes
* SOF and EOF. Mask constants are in xaxidma_hw.h
*
* \par Scatter-Gather DMA
*
* SGDMA allows the application to specify a list of transactions in memory which
* the hardware will process without further application intervention. During
* this time, the application is free to continue adding more work to keep the
* Hardware busy.
*
* User can check for the completion of transactions through polling the
* hardware, or interrupts.
*
* SGDMA processes whole packets. A packet is a series of
* data bytes that represent a message. SGDMA allows a packet of data to be
* broken up into one or more transactions. For example, take an Ethernet IP
* packet which consists of a 14 byte header followed by a 1 or more bytes of
* payload. With SGDMA, the application may point a BD to the header and another
* BD to the payload, then transfer them as a single message. This strategy can
* make a TCP/IP stack more efficient by allowing it to keep packet header and
* data in different memory regions instead of assembling packets into
* contiguous blocks of memory.
*
* \par BD Ring Management
*
* BD rings are shared by the software and the hardware.
*
* The hardware expects BDs to be setup as a linked list. The DMA hardware walks
* through the list by following the next pointer field of a completed BD.
* The hardware stops processing when the just completed BD is the same as the
* BD specified in the Tail Ptr register in the hardware.
*
* The last BD in the ring is linked to the first BD in the ring.
*
* All BD management are done inside the driver. The user application should not
* directly modify the BD fields. Modifications to the BD fields should always
* go through the specific API functions.
*
* Within the ring, the driver maintains four groups of BDs. Each group consists
* of 0 or more adjacent BDs:
*
*   - Free: The BDs that can be allocated by the application with
*     `XAxiDma_BdRingAlloc()`.
*
*   - Pre-process: The BDs that have been allocated with
*     `XAxiDma_BdRingAlloc()`. These BDs are under application control. The
*     application modifies these BDs through driver API to prepare them
*     for DMA transactions.
*
*   - Hardware: The BDs that have been enqueued to hardware with
*     `XAxiDma_BdRingToHw()`. These BDs are under hardware control and may be in a
*     state of awaiting hardware processing, in process, or processed by
*     hardware. It is considered an error for the application to change BDs
*     while they are in this group. Doing so can cause data corruption and lead
*     to system instability.
*
*   - Post-process: The BDs that have been processed by hardware and have
*     been extracted from the Hardware group with `XAxiDma_BdRingFromHw()`.
*     These BDs are under application control. The application can check the
*     transfer status of these BDs. The application use `XAxiDma_BdRingFree()`
*     to put them into the Free group.
*
* BDs are expected to transition in the following way for continuous
* DMA transfers:
* @verbatim
*
*         XAxiDma_BdRingAlloc()                   XAxiDma_BdRingToHw()
*   Free ------------------------> Pre-process ----------------------> Hardware
*                                                                      |
*    /|\                                                               |
*     |   XAxiDma_BdRingFree()                  XAxiDma_BdRingFromHw() |
*     +--------------------------- Post-process &lt;----------------------+
*
* @endverbatim
*
* When a DMA transfer is to be cancelled before enqueuing to hardware,
* application can return the requested BDs to the Free group using
* `XAxiDma_BdRingUnAlloc()`, as shown below:
* @verbatim
*
*         XAxiDma_BdRingUnAlloc()
*   Free &lt;----------------------- Pre-process
*
* @endverbatim
*
* The API provides functions for BD list traversal:
* - `XAxiDma_BdRingNext()`
* - `XAxiDma_BdRingPrev()`
*
* These functions should be used with care as they do not understand where
* one group ends and another begins.
*
* \par SGDMA Descriptor Ring Creation
*
* BD ring is created using `XAxiDma_BdRingCreate()`. The memory for the BD ring
* is allocated by the application, and it has to be contiguous. Physical
* address is required to setup the BD ring.
*
* The application can use `XAxiDma_BdRingMemCalc()` to find out the amount of
* memory needed for a certain number of BDs. `XAxiDma_BdRingCntCalc()` can be
* used to find out how many BDs can be allocated for certain amount of memory.
*
* A helper function, `XAxiDma_BdRingClone()`, can speed up the BD ring setup if
* the BDs have same types of controls, for example, SOF and EOF. After
* using the `XAxiDma_BdRingClone()`, the application only needs to setup the
* buffer address and transfer length. Note that certain BDs in one packet,
* for example, the first BD and the last BD, may need to setup special
* control information.
*
* \par Descriptor Ring State Machine
*
* There are two states of the BD ring:
*
*   - HALTED (H), where hardware is not running
*
*   - NOT HALTED (NH), where hardware is running
*
* The following diagram shows the state transition for the DMA engine:
*
* @verbatim
*   _____ XAxiDma_StartBdRingHw(), or XAxiDma_BdRingStart(),   ______
*   |   |               or XAxiDma_Resume()                    |    |
*   | H |----------------------------------------------------->| NH |
*   |   |&lt;-----------------------------------------------------|    |
*   -----   XAxiDma_Pause() or XAxiDma_Reset()                 ------
* @endverbatim
*
* \par Interrupt Coalescing
*
* SGDMA provides control over the frequency of interrupts through interrupt
* coalescing. The DMA engine provides two ways to tune the interrupt
* coalescing:
*
* - The packet threshold counter. Interrupt will fire once the
*   programmable number of packets have been processed by the engine.
*
* - The packet delay timer counter. Interrupt will fire once the
*   programmable amount of time has passed after processing the last packet,
*   and no new packets to process. Note that the interrupt will only fire if
*   at least one packet has been processed.
*
* \par Interrupt
*
* Interrupts are handled by the user application. Each DMA channel has its own
* interrupt ID. The driver provides APIs to enable/disable interrupt,
* and tune the interrupt frequency regarding to packet processing frequency.
*
* \par Software Initialization
*
* To use the Simple mode DMA engine for transfers, the following setup is
* required:
*
* - DMA Initialization using `XAxiDma_CfgInitialize()` function. This step
*   initializes a driver instance for the given DMA engine and resets the
*   engine.
*
* - Enable interrupts if chosen to use interrupt mode. The application is
*   responsible for setting up the interrupt system, which includes providing
*   and connecting interrupt handlers and call back functions, before
*   enabling the interrupts.
*
* - Set the buffer address and length field in respective channels to start
*   the DMA transfer
*
* To use the SG mode DMA engine for transfers, the following setup are
* required:
*
* - DMA Initialization using `XAxiDma_CfgInitialize()` function. This step
*   initializes a driver instance for the given DMA engine and resets the
*   engine.
*
* - BD Ring creation. A BD ring is needed per DMA channel and can be built by
*   calling `XAxiDma_BdRingCreate()`.
*
* - Enable interrupts if chose to use interrupt mode. The application is
*   responsible for setting up the interrupt system, which includes providing
*   and connecting interrupt handlers and call back functions, before
*   enabling the interrupts.
*
* - Start a DMA transfer: Call `XAxiDma_BdRingStart()` to start a transfer for
*   the first time or after a reset, and `XAxiDma_BdRingToHw()` if the channel
*   is already started. Calling `XAxiDma_BdRingToHw()` when a DMA channel is not
*   running will not put the BDs to the hardware, and the BDs will be processed
*   later when the DMA channel is started through `XAxiDma_BdRingStart()`.
*
* \par How to start DMA transactions
*
* The user application uses `XAxiDma_BdRingToHw()` to submit BDs to the hardware
* to start DMA transfers.
*
* For both channels, if the DMA engine is currently stopped (using
* `XAxiDma_Pause()`), the newly added BDs will be accepted but not processed
* until the DMA engine is started, using `XAxiDma_BdRingStart()`, or resumed,
* using `XAxiDma_Resume()`.
*
* \par Software Post-Processing on completed DMA transactions
*
* If the interrupt system has been set up and the interrupts are enabled,
* a DMA channels notifies the software about the completion of a transfer
* through interrupts. Otherwise, the user application can poll for
* completions of the BDs, using `XAxiDma_BdRingFromHw()` or
* `XAxiDma_BdHwCompleted()`.
*
* - Once BDs are finished by a channel, the application first needs to fetch
*   them from the channel using `XAxiDma_BdRingFromHw()`.
*
* - On the TX side, the application now could free the data buffers attached to
*   those BDs as the data in the buffers has been transmitted.
*
* - On the RX side, the application now could use the received data in the
*	buffers attached to those BDs.
*
* - For both channels, completed BDs need to be put back to the Free group
*   using `XAxiDma_BdRingFree()`, so they can be used for future transactions.
*
* - On the RX side, it is the application's responsibility to have BDs ready
*   to receive data at any time. Otherwise, the RX channel refuses to
*   accept any data if it has no RX BDs.
*
* \par Examples
*
* We provide five examples to show how to use the driver API:
* - One for SG interrupt mode (xaxidma_example_sg_intr.c), multiple BD/packets transfer
* - One for SG polling mode (xaxidma_example_sg_poll.c), single BD transfer.
* - One for SG polling mode (xaxidma_poll_multi_pkts.c), multiple BD/packets transfer
* - One for simple polling mode (xaxidma_example_simple_poll.c)
* - One for simple Interrupt mode (xaxidma_example_simple_intr.c)
*
* \par Address Translation
*
* All buffer addresses and BD addresses for the hardware are physical
* addresses. The user application is responsible to provide physical buffer
* address for the BD upon BD ring creation. The user application accesses BD
* through its virtual address. The driver maintains the address translation
* between the physical and virtual address for BDs.
*
* \par Cache Coherency
*
* This driver expects all application buffers attached to BDs to be in cache
* coherent memory. If cache is used in the system, buffers for transmit MUST
* be flushed from the cache before passing the associated BD to this driver.
* Buffers for receive MUST be invalidated before accessing the data.
*
* \par Alignment
*
* For BDs:
*
* Minimum alignment uses the constant `XAXIDMA_BD_MINIMUM_ALIGNMENT`.
* This is the smallest alignment allowed by both hardware and software for them
* to properly work.
*
* If the descriptor ring is to be placed in cached memory, alignment also MUST
* be at least the processor's cache-line size. Otherwise, system instability
* occurs. For alignment larger than the cache line size, multiple cache line
* size alignment is required.
*
* Aside from the initial creation of the descriptor ring (see
* `XAxiDma_BdRingCreate()`), there are no other run-time checks for proper
* alignment of BDs.
*
* For application data buffers:
*
* Application data buffers may reside on any alignment if DRE is built into the
* hardware. Otherwise, application data buffer must be word-aligned. The word
* width uses \c XPAR_AXIDMA_0_M_AXIS_MM2S_TDATA_WIDTH for transmit and
* \c XPAR_AXIDMA_0_S_AXIS_S2MM_TDATA_WIDTH for receive.
*
* For scatter gather transfers that have more than one BDs in the chain of BDs,
* Each BD transfer length must be multiple of word too. Otherwise, internal
* error happens in the hardware.
*
* \par Error Handling
*
* The DMA engine will halt on all error conditions. It requires the software
* to do a reset before it can start process new transfer requests.
*
* \par Restart After Stopping
*
* After the DMA engine has been stopped (through reset or reset after an error)
* the software keeps track of the current BD pointer when reset happens, and
* processing of BDs can be resumed through `XAxiDma_BdRingStart()`.
*
* \par Limitations
*
* This driver does not have any mechanisms for mutual exclusion. It is up to
* the application to provide this protection.
*
* \par Hardware Defaults &amp; Exclusive Use
*
* After the initialization or reset, the DMA engine is in the following
* default mode:
* - All interrupts are disabled.
*
* - Interrupt coalescing counter is 1.
*
* - The DMA engine is not running (halted). Each DMA channel is started
*   separately, using `XAxiDma_StartBdRingHw()` if no BDs are setup for transfer
*   yet, or `XAxiDma_BdRingStart()` otherwise.
*
* The driver has exclusive use of the registers and BDs. All accesses to the
* registers and BDs should go through the driver interface.
*
* \par Debug Print
*
* To see driver debug output, add DEBUG to the compiler flags and enable
* DEBUG in xdebug.h.
*
* \par Changes From v1.00a
*
* . We have changes return type for `XAxiDma_BdSetBufAddr()` from void to int
* . We added `XAxiDma_LookupConfig()` so that user does not need to look for the
* hardware settings anymore.
*
* @verbatim
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   05/18/10 First release
* 2.00a jz   08/10/10 Second release, added in xaxidma_g.c, xaxidma_sinit.c,
*                     updated tcl file, added xaxidma_porting_guide.h
* 3.00a jz   11/22/10 Support IP core parameters change
* 4.00a rkv  02/22/11 Added support for simple DMA mode. New APIs:
*                     XAxiDma_Busy and XAxiDma_SimpleTransfer. New macros:
*                     XAxiDma_HasSg, XAxiDma_IntrEnable, XAxiDma_IntrGetEnabled,
*                     XAxiDma_IntrDisable, XAxiDma_IntrGetIrq, XAxiDma_IntrAckIrq.
* 5.00a srt  08/25/11  Added support for memory barrier and modified
*			Cache Macros to have a common API for Microblaze
*			and Zynq.
* 6.00a srt  01/24/12 Added support for Multi-Channel DMA mode.
* 7.00a srt  06/18/12 Reverted v6.00a API changes for backward compatibility.
*                     Added XAxiDma_GetRxIndexRing().
* 7.01a srt  10/26/12 Fixed C_NUM_CHANNELS check for IP versions before 6.00a.
*                     Updated MCDMA BD field Set APIs.
* 7.02a srt  01/23/13 Replaced TDATA_WIDTH parameters with DATA_WIDTH
*		       (CR 691867). Updated DDR base address for IPI designs
*		       (CR 703656).
* 8.0   adk  19/12/13 Updated as per the new Tcl APIs.
*	srt  01/29/14 Added support for Micro DMA mode and cyclic mode.
* 8.1   adk  20/01/15 Added support for peripheral test (CR823144).
* 8.1   adk  29/01/15 Added XAxiDma_Selftest to the driver source files
* 		      (xaxidma_selftest.c) and called it from the selftest example.
* 9.0 	adk  27/07/15  Added support for 64-bit Addressing.
* 9.0   adk  19/08/15  Fixed CR#873125 DMA SG Mode example tests are failing on
*		       HW in 2015.3.
* 9.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
* 9.3   adk  26/07/16 Reduce the size of the buffer descriptor to 64 bytes.
*       ms   01/23/17 Modified xil_printf statement in main function for all
*            examples to ensure that "Successfully ran" and "Failed" strings
*            are available in all examples. This is a fix for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*       ms   04/05/17 Modified Comment lines in functions to recognize
*                     it as documentation block and added tabspace at return
*                     statements in functions of axidma examples for proper
*                     documentation while generating doxygen.
*                     Modified readme file in examples folder.
* 9.4  adk   25/07/17 Added example for cyclic dma mode CR#974218.
*      adk   08/08/17 Fixed CR#980607 Can't select individual AXI DMA code examples.
*		      Fixed compilation warning in the driver
* 9.5  adk   17/10/17 Fixed CR#987026 mulit packet example fails on A53.
* 	     26/10/17 Fixed CR#987214 Fix race condition in the XAxiDma_Reset().
*      rsp   11/01/17 Fixed CR#988210 Add interface to do config lookup based
*                     on base address.
*      adk   13/11/17 Fixed CR#989455 multi-channel interrupt example fails on A53.
* 9.6  rsp   01/11/18 Fixed CR#976392 In XAxiDma struct use UINTPTR for RegBase.
*                     In XAxiDma_LookupConfigBaseAddr() use UINTPTR for Baseaddr.
* 9.7  rsp   04/25/18 Add SgLengthWidth member in dma config structure. CR\#1000474
* 9.13 rsp   01/08/21 Fix compilation failure in XAxiDma_IntrGetEnabled().
* 9.15  sa   08/12/22 Updated the examples to use latest MIG canonical XPAR
* 		       symbol for example XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
*      adk   08/16/22 Fix syntax error in the XAxiDma_BdRingGetCurrBd() API.
* @endverbatim
*
 ******************************************************************************/

/**
 * @addtogroup AXIDMA Overview
 * @{
 */

#ifndef XAXIDMA_H_	/* prevent circular inclusions */
/** @brief Include guard for the AXI DMA driver header. */
#define XAXIDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xaxidma_bdring.h"
#ifdef __MICROBLAZE__
#include "xenv.h"
#else
#include <string.h>
#include "xil_cache.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * @brief Holds driver instance data for one DMA engine.
 */
typedef struct XAxiDma {
	UINTPTR RegBase;		/**< Virtual base address of the DMA engine */

	int HasMm2S;		/**< Has transmit channel */
	int HasS2Mm;		/**< Has receive channel */
	int Initialized;	/**< Driver has been initialized */
	int HasSg;		/**< Has scatter gather support */

	XAxiDma_BdRing TxBdRing;     /**< BD container management for TX channel */
	XAxiDma_BdRing RxBdRing[16]; /**< BD container management for RX channel */
	int TxNumChannels; /**< Number of transmit channels. */
	int RxNumChannels; /**< Number of receive channels. */
	int MicroDmaMode; /**< Indicates whether Micro DMA mode is enabled. */
	int AddrWidth;		  /**< Address Width */
} XAxiDma;

/**
 * @brief Holds hardware configuration for an AXI DMA engine.
 *
 * Non-SDT builds use DeviceId. SDT builds use Name, IntrId, and IntrParent.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId;	/**< Unique ID of the device */
#else
	char *Name;	/**< Unique name of the device */
#endif
	UINTPTR BaseAddr;	/**< Base address of the device */

	int HasStsCntrlStrm;	/**< Has status and control streams */
	int HasMm2S;		/**< Has memory to stream channel */
	int HasMm2SDRE;		/**< Has MM2S data realignment engine */
	int Mm2SDataWidth;	/**< MM2S data width in bits */
	int HasS2Mm;		/**< Has stream to memory channel */
	int HasS2MmDRE;		/**< Has S2MM data realignment engine */
	int S2MmDataWidth;	/**< S2MM data width in bits */
	int HasSg;		/**< Has scatter gather support */
	int Mm2sNumChannels;	/**< Number of MM2S channels */
	int S2MmNumChannels;	/**< Number of S2MM channels */
	int Mm2SBurstSize;	/**< Maximum burst length for MM2S channel (in beats) */
	int S2MmBurstSize;	/**< Maximum burst length for S2MM channel (in beats) */
	int MicroDmaMode;	/**< Micro DMA mode enable */
	int AddrWidth;		  /**< Address Width */
	int SgLengthWidth;	/**< Scatter Gather transfer length field width */
#ifdef SDT
	u16 IntrId[2]; /**< Bits[11:0] Interrupt-id Bits[15:12] trigger type and level flags */
	UINTPTR IntrParent; /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XAxiDma_Config;


/***************** Macros (Inline Functions) Definitions *********************/
/**@cond axidma_internal
 * @{
 */

/*****************************************************************************/
/**
 * @def XAxiDma_GetTxRing
* @brief Gets the transmit (Tx) BD ring pointer.
*
* Warning: This has a different API than the LLDMA driver. It now returns
* the pointer to the BD ring.
*
* @param[in] InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
*
* @return	Pointer to the Tx Ring
*
* @note		C-style signature:
* 		\c XAxiDma_BdRing * \c XAxiDma_GetTxRing(\c XAxiDma * InstancePtr)
* 		Usable only when the system is configured as SG mode
*
*****************************************************************************/
#define XAxiDma_GetTxRing(InstancePtr) \
	(&((InstancePtr)->TxBdRing))

/*****************************************************************************/
/**
 * @def XAxiDma_GetRxRing
* @brief Gets the receive (Rx) BD ring pointer.
*
* Warning: This has a different API than the LLDMA driver. It now returns
* the pointer to the BD ring.
*
* @param[in] InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
*
* @return	Pointer to the Rx Ring
*
* @note
* 		C-style signature:
* 		\c XAxiDma_BdRing * \c XAxiDma_GetRxRing(\c XAxiDma * InstancePtr)
*		Usable only when the system is configured as SG mode
*
*****************************************************************************/
#define XAxiDma_GetRxRing(InstancePtr) \
	(&((InstancePtr)->RxBdRing[0]))

/*****************************************************************************/
/**
 * @def XAxiDma_GetRxIndexRing
* @brief Gets the receive (Rx) BD ring pointer for an index.
*
* Warning: This has a different API than the LLDMA driver. It now returns
* the pointer to the BD ring.
*
* @param[in] InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
* @param[in] RingIndex is the channel Index.
*
* @return	Pointer to the Rx Ring
*
* @note
* 		C-style signature:
* 		\c XAxiDma_BdRing * \c XAxiDma_GetRxIndexRing(\c XAxiDma * InstancePtr,
* 						int RingIndex)
*		Usable only when the system is configured as SG mode
*
*****************************************************************************/
#define XAxiDma_GetRxIndexRing(InstancePtr, RingIndex) \
	(&((InstancePtr)->RxBdRing[RingIndex]))

/*****************************************************************************/
/**
 * @def XAxiDma_HasSg
* @brief Checks whether the system is configured as simple or SG mode.
*
* @param[in] InstancePtr is a pointer to the DMA engine instance to be
*		worked on.
*
* @return TRUE if configured as SG mode, FALSE if configured as simple mode.
*
*****************************************************************************/
#define XAxiDma_HasSg(InstancePtr)	((InstancePtr)->HasSg) ? TRUE : FALSE

/*****************************************************************************/
/**
 * @def XAxiDma_IntrEnable
 * @brief Enables interrupts specified by the mask in the given direction.
 *
 * @param[in] InstancePtr is the driver instance we are working on
 * @param[in] Mask is the mask for the interrupts to be enabled
 * @param[in] Direction is DMA transfer direction. Use `XAXIDMA_DMA_TO_DEVICE` or
 *		`XAXIDMA_DEVICE_TO_DMA`.
 *
 *****************************************************************************/
#define XAxiDma_IntrEnable(InstancePtr, Mask, Direction) 	\
	XAxiDma_WriteReg((InstancePtr)->RegBase + \
			 (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_CR_OFFSET, \
			 (XAxiDma_ReadReg((InstancePtr)->RegBase + \
					  (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_CR_OFFSET)) \
			 | (Mask & XAXIDMA_IRQ_ALL_MASK))


/*****************************************************************************/
/**
 * @def XAxiDma_IntrGetEnabled
 * @brief Reads the mask for currently enabled interrupts.
 *
 * @param[in] InstancePtr is the driver instance we are working on
 * @param[in] Direction is DMA transfer direction. Use `XAXIDMA_DMA_TO_DEVICE` or
 *		`XAXIDMA_DEVICE_TO_DMA`.
 *
 * @return	The bit mask for the interrupts that are currently enabled
 *
 *****************************************************************************/
#define XAxiDma_IntrGetEnabled(InstancePtr, Direction)	\
	(XAxiDma_ReadReg((InstancePtr)->RegBase + \
			 (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_CR_OFFSET) &\
	 XAXIDMA_IRQ_ALL_MASK)



/*****************************************************************************/
/**
 * @def XAxiDma_IntrDisable
 * @brief Disables interrupts specified by the mask.
 *
 * @param[in] InstancePtr is the driver instance we are working on
 * @param[in] Mask is the mask for the interrupts to be disabled
 * @param[in] Direction is DMA transfer direction. Use `XAXIDMA_DMA_TO_DEVICE` or
 *		`XAXIDMA_DEVICE_TO_DMA`.
 *
 *****************************************************************************/
#define XAxiDma_IntrDisable(InstancePtr, Mask, Direction)	\
	XAxiDma_WriteReg((InstancePtr)->RegBase + \
			 (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_CR_OFFSET, \
			 (XAxiDma_ReadReg((InstancePtr)->RegBase + \
					  (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_CR_OFFSET)) \
			 & ~(Mask & XAXIDMA_IRQ_ALL_MASK))


/*****************************************************************************/
/**
 * @def XAxiDma_IntrGetIrq
 * @hideinitializer
 * @brief Reads asserted interrupts.
 *
 * @param[in] InstancePtr is the driver instance we are working on.
 * @param[in] Direction is DMA transfer direction. Use `XAXIDMA_DMA_TO_DEVICE` or
 *		`XAXIDMA_DEVICE_TO_DMA`.
 *
 * @return Returns the bit mask for the interrupts that are asserted.
 *
 *****************************************************************************/
#define XAxiDma_IntrGetIrq(InstancePtr, Direction)	\
	(XAxiDma_ReadReg((InstancePtr)->RegBase + \
			 (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_SR_OFFSET) &\
	 XAXIDMA_IRQ_ALL_MASK)

/*****************************************************************************/
/**
 * @def XAxiDma_IntrAckIrq
 * @brief Acknowledges the specified interrupt mask.
 *
 * @param[in] InstancePtr is the driver instance we are working on
 * @param[in] Mask is the mask for the interrupts to be acknowledge
 * @param[in] Direction is DMA transfer direction. Use `XAXIDMA_DMA_TO_DEVICE` or
 *		`XAXIDMA_DEVICE_TO_DMA`.
 *
 *****************************************************************************/
#define XAxiDma_IntrAckIrq(InstancePtr, Mask, Direction)	\
	XAxiDma_WriteReg((InstancePtr)->RegBase + \
			 (XAXIDMA_RX_OFFSET * Direction), XAXIDMA_SR_OFFSET, \
			 (Mask) & XAXIDMA_IRQ_ALL_MASK)



/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/

/*
 * Initialization and control functions in xaxidma.c
 */
#ifndef SDT
/*****************************************************************************/
/**
 * @brief Looks up device configuration by device ID.
 *
 * @param[in] DeviceId contains the unique ID of the device.
 *
 * @return A pointer to the configuration table entry corresponding to the given
 *         device ID, or NULL if no match is found.
 *
 *****************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfig(u32 DeviceId);
/*****************************************************************************/
/**
 * @brief Looks up device configuration by base address.
 *
 * @param[in] Baseaddr is the base address of the device to look up.
 *
 * @return The configuration structure for the device. If the device base
 *         address is not found, a NULL pointer is returned.
 *
 *****************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfigBaseAddr(UINTPTR Baseaddr);
#else
/*****************************************************************************/
/**
 * @brief Looks up device configuration by base address.
 *
 * @param[in] BaseAddress contains the base address of the device.
 *
 * @return A pointer to the configuration table entry corresponding to the given
 *         base address, or NULL if no match is found.
 *
 *****************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfig(UINTPTR BaseAddress);
#endif
int XAxiDma_CfgInitialize(XAxiDma *InstancePtr, XAxiDma_Config *Config);
void XAxiDma_Reset(XAxiDma *InstancePtr);
int XAxiDma_ResetIsDone(XAxiDma *InstancePtr);
int XAxiDma_Pause(XAxiDma *InstancePtr);
int XAxiDma_Resume(XAxiDma *InstancePtr);
u32 XAxiDma_Busy(XAxiDma *InstancePtr, int Direction);
u32 XAxiDma_SimpleTransfer(XAxiDma *InstancePtr, UINTPTR BuffAddr, u32 Length,
			   int Direction);
int XAxiDma_SelectKeyHole(XAxiDma *InstancePtr, int Direction, int Select);
int XAxiDma_SelectCyclicMode(XAxiDma *InstancePtr, int Direction, int Select);
int XAxiDma_Selftest(XAxiDma *InstancePtr);
#ifdef __cplusplus
}
#endif

/** @} */

#endif /* end of protection macro */
