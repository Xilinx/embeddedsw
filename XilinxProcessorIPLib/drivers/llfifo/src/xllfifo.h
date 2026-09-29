/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XLLFIFO_H		/* prevent circular inclusions */
#define XLLFIFO_H

/*****************************************************************************/
/**
 *
 * @file xllfifo.h
* @addtogroup llfifo Overview
* @{
* @brief Defines the XLlFifo driver software API.
*
* The Xilinx Dual Channel Fifo driver component. This driver supports the
* Virtex-5(TM) and Virtex-4(TM) XPS_ll_Fifo and the AxiFifo.
*
* For a full description of the bridge features, see the hardware spec. This
* driver supports the following features:
 *   - Memory mapped access to host interface registers
 *   - API for polled frame transfers
 *   - API for interrupt driven frame transfers
 *   - Virtual memory support
 *   - Full duplex operation
 *
 * <h2>Driver Description</h2>
 *
 * This driver enables higher layer software to access the XPS_llFifo core
 * using any alignment in the data buffers.
 *
 * This driver supports send and receive channels in the same instance
 * structure in the same fashion as the hardware core.
 *
 * <h2>Initialization</h2>
 *
 * An instance of this driver is initialized using a call to Initialize().
 *
 * <h2>Usage</h2>
 *
 * It is fairly simple to use the API provided by this FIFO driver. The
 * only somewhat tricky part is that the calling code must correctly call
 * a couple routines in the right sequence for receive and transmit.
 *
 * This sequence is described here. Check the routine functional
 * descriptions for information on how to use a specific API routine.
 *
 * <h3>Receive</h3>
 *
 * A frame is received by using the following sequence:<br>
 * 1) call XLlFifo_iRxGetLen() to get the length of the incoming frame<br>
 * 2) call XLlFifo_RxGetWord() one or more times to read the number of
 *    bytes reported by the hardware<br>
 * 3) call XLlFifo_iRxOccupancy() to know the availability of the data
 *    in the FIFO.<br>
 *
 * For example:
 * <pre>
 * 	ReceiveLength = (XLlFifo_iRxGetLen(InstancePtr))/WORD_SIZE;
 *	for ( i=0; i < ReceiveLength; i++){
 *		RxWord = XLlFifo_RxGetWord(InstancePtr);
 * 		// ********
 * 		// do something here with the data
 * 		// ********
 *		if(XLlFifo_iRxOccupancy(InstancePtr)){
 *			RxWord = XLlFifo_RxGetWord(InstancePtr);
 *		}
 *	}
 *
 * </pre>
 *
 * This FIFO hardware core does <b>not</b> support a sequence where the
 * calling code calls iRxGetLen() twice in a row and then receive the data
 * for two frames. Each frame must be read in by calling iRxGetLen() just
 * prior to reading the data.
 *
 * <h3>Transmit</h3>
 * A frame is transmittted by using the following sequence:<br>
 * 1) XLlFifo_iTxVacancy() one or more times to know the availability of
 *    unused 32-bit words in the FIFO channel.<br>
 * 2) call XLlFifo_TxPutWord() one or more times to write all the of bytes in
 *    the next frame.<br>
 * 3) call XLlFifo_iTxSetLen() to begin the transmission of frame just
 *    written.<br>
 *
 * For example:
 * <pre>
 *	for (j=0 ; j < MAX_PACKET_LEN ; j++){
 *		if( XLlFifo_iTxVacancy(InstancePtr) ){
 *			XLlFifo_TxPutWord(InstancePtr,
 *				*(SourceAddr+(i*MAX_PACKET_LEN)+j));
 *		}
 *	}
 * 	XLlFifo_iTxSetLen(InstancePtr, (MAX_DATA_BUFFER_SIZE * WORD_SIZE));
 * </pre>
 *
 * This FIFO hardware core does <b>not</b> support a sequence where the
 * calling code writes the data for two frames and then calls iTxSetLen()
 * twice in a row. Each frame must be written by writing the data for one
 * frame and then calling iTxSetLen().
 *
 * <h2>Interrupts</h2>
 * This driver does not handle interrupts from the FIFO hardware. The
 * software layer above may make use of the interrupts by setting up its
 * own handlers for the interrupts.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jvb  10/12/06 First release
 * 1.01a sdm  08/22/08 Removed support for static interrupt handlers from the
 *		       MDD file
 * 1.02a jz   12/04/09 Hal phase 1 support
 * 2.01a asa  09/17/10 Added code for resetting Local Link/AXI Streaming
 *		       interface for CR574868
 * 2.02a asa  12/27/11 Changed the function XStrm_Read in xtreamer.c to reset
 *		       HeadIndex to zero when all bytes have been read.
 *		       Changed the macro XStrm_IsRxInternalEmpty in file
 *		       xtreamer.h to use FrmByteCnt instead of HeadIndex.
 *		       When FrmByteCnt is zero, this means all internal buffers
 *		       in streamer are empty. Earlier implementation using
 *		       HeadIndex was not very clear and could give improper
 *		       results for some cases.
 *		       Changed the macro XLlFifo_IsRxEmpty in file xllfifo.h
 *		       These changes are done to fix the CR 604650.
  * 2.03a asa  14/08/12  Added XLLF_TDR_OFFSET, XLLF_RDR_OFFSET
 *		         defines for the new registers, and XLLF_INT_TFPF_MASK,
 *		         XLLF_INT_TFPE_MASK, XLLF_INT_RFPF_MASK and
 *		         XLLF_INT_RFPE_MASK for the new version of the
 *		         AXI4-Stream FIFO core (v2.01a and later)
 *
 * 3.00a adk 08/10/13 Added support for AXI4 Datainterface.Changes are
 * 		      In Xllfifo.c file XLlFifo_RxGetWord,XLlFifo_TxPutword.
 * 		      In XLlfifo.h file updated XLlfifo structure for
 * 		      Axi4BaseAddress and for Datainterface type provided
 *		      polling and interrupt examples. XLlfifo_IsRxDone Macro
 *		      Is added in the XLlfifo.h file for polledmode example.
 *		      Added Static initialization for the driver.
 *		      XLlFifo_Initialize is still used to make the driver
 *		      backward compatible.
 * 4.0   adk  19/12/13 Updated as per the New Tcl API's
 * 5.0   adk  15/05/15 Updated the register offsets in the AXI4 data path
 *		       as per latest IP version(v4.1)(CR:860254).
 * 5.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
 *                     Changed the prototypes of XLlFifo_CfgInitialize,
 *                     XLlFifo_Initialize APIs.
 * 5.1  adk   01/02/15 CR#885653 Fix Incorrect AXI4 Base address being
 *                     Exported to the xparameters.h file.
 *      ms    01/23/17 Modified xil_printf statement in main function for all
 *                     examples to ensure that "Successfully ran" and "Failed"
 *                     strings are available in all examples. This is a fix
 *                     for CR-965028.
 *      ms    03/17/17 Added readme.txt file in examples folder for doxygen
 *                     generation.
 *      ms    04/05/17  Added tabspace for return statements in functions for
 *                      proper documentation and Modified Comment lines
 *                      to consider it as a documentation block while
 *                      generating doxygen for llfifo examples.
 * 5.2  ms    04/18/17 Modified tcl file to add suffix U for all macros
 *                     definitions of llfifo in xparameters.h
 * 5.2 adk    03/07/17 CR#978769 Fix doxygen issues in the driver.
 *		       Updated comments in the usage section as per example code.
 *		       Fix doxygen warnings in the driver.
 * 5.5 sd     09/04/20  Makefile update for parallel execution.
 * 5.6 sd     07/7/23  Add system devicetree support.
 * 6.1 vlt    12/30/25 Update Doxygen comments to include SDT flow details.
 * 6.1 vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR
 *                     to support 64-bit addressing.
 * 6.2   tb  09/15/26 Align Doxygen comments with AMD standards.
 * </pre>
 *
 *****************************************************************************/

/* force C linkage */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/* This order needs to be kept this way to avoid xstatus/xil_types conflict */
#include "xstreamer.h"
#include "xllfifo_hw.h"

/**************************** Type Definitions *******************************/

/**
 * @brief Defines the XLlFifo driver instance.
 *
 * This typedef defines a run-time instance of an XLlFifo device.
 */
typedef struct XLlFifo {
	UINTPTR BaseAddress; /**< Physical base address of the device
	                      *  registers.
	                      */

	u32 IsReady;           /**< IsReady is non-zero if the driver instance
	                        *   has been initialized.
	                        */

	UINTPTR Axi4BaseAddress; /**< AXI4 FIFO data-path base address. */
	u32 Datainterface; /**< Zero if the data interface is AXI4-Lite. */
	XStrm_RxFifoStreamer RxStreamer; /**< RxStreamer is the byte streamer
	                                  *   instance for the receive channel.
	                                  */
	XStrm_TxFifoStreamer TxStreamer; /**< TxStreamer is the byte streamer
	                                  *   instance for the transmit channel.
	                                  */
#ifdef SDT
	u32 IntId; /**< Interrupt ID on GIC **/
	UINTPTR IntrParent; 	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
} XLlFifo;

/**
 * @brief Defines the XLlFifo hardware configuration.
 */
typedef struct XLlFifo_Config {
#ifndef SDT
	u32 DeviceId;		/**< Deviceid of the AXI FIFO */
#else
	char *Name;             /**< Name of the device */
#endif
	UINTPTR BaseAddress;	/**< Base Address of the AXI FIFO */
	UINTPTR Axi4BaseAddress;    /**< Axi4 interface Base address */
	u32 Datainterface;	/**< Type of Datainterface */
#ifdef SDT
	u32 IntId; /**< Interrupt ID on GIC **/
	UINTPTR IntrParent; 	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
}XLlFifo_Config;

/****************************************************************************/
/**
* @brief Resets both FIFO channels and the local-link interface.
*
* XLlFifo_Reset resets both the Tx and Rx channels and the local-link
* interface of the FIFO specified by <i>InstancePtr</i>. XLlFifo_TxReset
* also sends a reset pulse to the downstream device (for example, TEMAC).
* XLlFifo_Reset drops any bytes in the FIFO not yet retrieved or transmitted.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @note
* C-style signature
*    void XLlFifo_Reset(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_Reset(InstancePtr) \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_LLR_OFFSET, \
			XLLF_LLR_RESET_MASK)


/****************************************************************************/
/**
* @brief Returns the FIFO interrupt status mask.
*
* XLlFifo_Status returns a bit mask of the interrupt status register (ISR)
* for the FIFO specified by <i>InstancePtr</i>. XLlFifo_Status can be used
* to query the status of the FIFO without having to have interrupts enabled.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_IntStatus returns a bit mask of the status conditions.
*           The mask is a set of bitwise or'd values from the
*           <code>XLLF_INT_*_MASK</code> preprocessor symbols.
*
* @note
* C-style signature
*    u32 XLlFifo_IntStatus(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_Status(InstancePtr) \
	 XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_ISR_OFFSET)

/****************************************************************************/
/**
* @brief Enables selected FIFO interrupts.
*
* XLlFifo_IntEnable enables the interrupts specified in <i>Mask</i> for the
* FIFO specified by <i>InstancePtr</i>. The corresponding interrupt for each bit
* set to 1 in <i>Mask</i> is enabled.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    Mask Contains a bit mask of the interrupts to enable. The mask
*               can be formed using a set of bitwise or'd values from the
*               <code>XLLF_INT_*_MASK</code> preprocessor symbols.
*
* @note
* C-style signature
*    void XLlFifo_IntEnable(XLlFifo *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XLlFifo_IntEnable(InstancePtr, Mask) \
{ \
	u32 Reg = XLlFifo_ReadReg((InstancePtr)->BaseAddress, \
			XLLF_IER_OFFSET); \
	Reg |= ((Mask) & XLLF_INT_ALL_MASK);                    \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_IER_OFFSET, \
			Reg); \
}

/****************************************************************************/
/**
* @brief Disables selected FIFO interrupts.
*
* XLlFifo_IntDisable disables the interrupts specified in <i>Mask</i> for the
* FIFO specified by <i>InstancePtr</i>. The corresponding interrupt for each bit
* set to 1 in <i>Mask</i> is disabled. In other words, XLlFifo_IntDisable
* uses the "set a bit to clear it" scheme.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    Mask Contains a bit mask of the interrupts to disable. The
*               mask can be formed using a set of bitwise or'd values from the
*               <code>XLLF_INT_*_MASK</code> preprocessor symbols.
*
* @note
* C-style signature
*    void XLlFifo_IntDisable(XLlFifo *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XLlFifo_IntDisable(InstancePtr, Mask) \
{ \
	u32 Reg = XLlFifo_ReadReg((InstancePtr)->BaseAddress, \
			XLLF_IER_OFFSET); \
	Reg &= ~((Mask) & XLLF_INT_ALL_MASK);  \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_IER_OFFSET, \
			Reg); \
}

/****************************************************************************/
/**
* @brief Returns the pending FIFO interrupt mask.
*
* XLlFifo_IntPending returns a bit mask of the pending interrupts for the
* FIFO specified by <i>InstancePtr</i>. Each bit set to 1 in the return value
* represents a pending interrupt.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_IntPending returns a bit mask of the interrupts that are
*           pending. The mask is a set of bitwise or'd values from the
*           <code>XLLF_INT_*_MASK</code> preprocessor symbols.
*
* @note
* C-style signature
*    u32 XLlFifo_IntPending(XLlFifo *InstancePtr)
*
*****************************************************************************/
#ifdef DEBUG
extern u32 _xllfifo_ipie_value;
extern u32 _xllfifo_ipis_value;
#define XLlFifo_IntPending(InstancePtr) \
	(_xllfifo_ipie_value = XLlFifo_ReadReg( \
		(InstancePtr)->BaseAddress, XLLF_IER_OFFSET),  \
	_xllfifo_ipis_value = XLlFifo_ReadReg( \
		(InstancePtr)->BaseAddress, XLLF_ISR_OFFSET),  \
	(_xllfifo_ipie_value & _xllfifo_ipis_value))
#else
#define XLlFifo_IntPending(InstancePtr) \
	(XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_IER_OFFSET) &  \
	 XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_ISR_OFFSET))
#endif

/****************************************************************************/
/**
* @brief Clears selected pending FIFO interrupts.
*
* XLlFifo_IntClear clears pending interrupts specified in <i>Mask</i> for the
* FIFO specified by <i>InstancePtr</i>. The corresponding pending interrupt for
* each bit set to 1 in <i>Mask</i> is cleared. In other words,
* XLlFifo_IntClear uses the "set a bit to clear it" scheme.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    Mask Contains a bit mask of the pending interrupts to clear.
*               The mask can be formed using a set of bitwise or'd values from
*               the <code>XLLF_INT_*_MASK</code> preprocessor symbols.
*
* @note
* C-style signature
*    void XLlFifo_IntClear(XLlFifo *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XLlFifo_IntClear(InstancePtr, Mask) \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_ISR_OFFSET, \
			((Mask) & XLLF_INT_ALL_MASK))

/****************************************************************************/
/**
* @brief Resets the FIFO receive channel.
*
* XLlFifo_RxReset resets the receive channel of the FIFO specified by
* <i>InstancePtr</i>. XLlFifo_RxReset drops any bytes in the FIFO not yet
* retrieved.
*
* The calling software may want to test for the completion of the reset by
* reading the interrupt status (IS) register and testing for the Rx Reset
* complete (RRC) bit.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @note
* C-style signature
*    void XLlFifo_RxReset(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_RxReset(InstancePtr) \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_RDFR_OFFSET, \
			XLLF_RDFR_RESET_MASK)

/****************************************************************************/
/**
* @brief Reports whether the receive channel is empty.
*
* XLlFifo_IsRxEmpty returns true if the receive channel of the FIFO, specified
* by <i>InstancePtr</i>, is empty.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_IsRxEmpty returns TRUE when the receive channel of the
*           FIFO is empty. Otherwise, XLlFifo_IsRxEmpty returns FALSE.
*
* @note
* C-style signature
*    int XLlFifo_IsRxEmpty(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_IsRxEmpty(InstancePtr) \
	((XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_RDFO_OFFSET) == 0) \
							? TRUE : FALSE)


/*****************************************************************************/
/**
* @brief Returns receive occupancy in 32-bit words.
*
* XLlFifo_RxOccupancy returns the number of 32-bit words available (occupancy)
* to be read from the receive channel of the FIFO specified by
* <i>InstancePtr</i>.
*
* The xps_ll_fifo core uses the same fifo to store data values and frame length
* values. Upon initialization, the XLlFifo_RxOccupancy gives the value of
* 1, which means one length value (a reserved fifo location) and no data
* values.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_RxOccupancy returns the occupancy count for the specified
*           packet FIFO.
*
* @note
* C-style signature
*    u32 XLlFifo_RxOccupancy(XLlFifo *InstancePtr)
*
******************************************************************************/
#define XLlFifo_RxOccupancy(InstancePtr) \
	XStrm_RxOccupancy(&((InstancePtr)->RxStreamer))

/*****************************************************************************/
/**
* @brief Returns the next receive frame length in bytes.
*
* XLlFifo_RxGetLen notifies the hardware that the program is ready to receive
* the next frame from the receive channel of the FIFO, specified by
* <i>InstancePtr</i>.
*
* Note that the program must first call XLlFifo_RxGetLen before pulling data
* out of the receive channel of the FIFO with XLlFifo_Read.
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_RxGetLen returns the number of bytes available in the next
*           frame.
*
* @note
* C-style signature
*    u32 XLlFifo_RxGetLen(XLlFifo *InstancePtr)
*
******************************************************************************/
#define XLlFifo_RxGetLen(InstancePtr) \
	XStrm_RxGetLen(&((InstancePtr)->RxStreamer))

/*****************************************************************************/
/**
* @brief Reads bytes from the FIFO receive channel.
*
* XLlFifo_Read reads <i>Bytes</i> bytes from the receive channel of the FIFO
* referenced by <i>InstancePtr</i> to the block of memory, referenced by
* <i>BufPtr</i>.
*
* Care must be taken to ensure that the number of bytes read with one or more
* calls to XLlFifo_Read() does not exceed the number of bytes available given
* from the last call to XLlFifo_RxGetLen().
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
* @param[out]    BufPtr Specifies the memory address to place the data read.
* @param[in]    Bytes Specifies the number of bytes to read.
*
* @note
* Error handling is handled through hardware exceptions and interrupts.
*
* C-style signature
*    void XLlFifo_Read(XLlFifo *InstancePtr, void *BufPtr, unsigned Bytes)
*
******************************************************************************/
#define XLlFifo_Read(InstancePtr, BufPtr, Bytes) \
	XStrm_Read(&((InstancePtr)->RxStreamer), (BufPtr), (Bytes))

/****************************************************************************/
/**
* @brief Resets the FIFO transmit channel.
*
* XLlFifo_TxReset resets the transmit channel of the FIFO specified by
* <i>InstancePtr</i>. XLlFifo_TxReset drops any bytes in the FIFO not yet
* transmitted.
*
* The calling software may want to test for the completion of the reset by
* reading the interrupt status (IS) register and testing for the Tx Reset
* complete (TRC) bit.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @note
* C-style signature
*    void XLlFifo_TxReset(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_TxReset(InstancePtr) \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_TDFR_OFFSET, \
			XLLF_TDFR_RESET_MASK)

/****************************************************************************/
/**
* @brief Reports whether transmit is complete.
*
* XLlFifo_IsTxDone returns true if the transmission in the transmit channel
* of the FIFO, specified by <i>InstancePtr</i>, is complete. XLlFifo_IsTxDone
* works only if the TC bit in the IS register is cleared before sending a
* frame.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_IsTxDone returns TRUE when the transmit channel of the
*           FIFO is complete. Otherwise, XLlFifo_IsTxDone returns FALSE.
*
* @note
* C-style signature
*    int XLlFifo_IsTxDone(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_IsTxDone(InstancePtr) \
	((XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_ISR_OFFSET) & \
		XLLF_INT_TC_MASK) \
		? TRUE : FALSE)

/****************************************************************************/
/**
* @brief Reports whether receive is complete.
*
* XLlFifo_IsRxDone returns true if the reception in the receive channel
* of the FIFO, specified by <i>InstancePtr</i>, is complete. XLlFifo_IsRxDone
* works only if the RC bit in the ISR register is cleared before receiving a
* frame.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_IsRxDone returns TRUE when the receive channel of the
*           FIFO is complete. Otherwise, XLlFifo_IsRxDone returns FALSE.
*
* @note
* C-style signature
*    int XLlFifo_IsRxDone(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_IsRxDone(InstancePtr) \
	((XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_ISR_OFFSET) & \
		XLLF_INT_RC_MASK) \
		? TRUE : FALSE)

/****************************************************************************/
/**
* @brief Returns transmit vacancy in 32-bit words.
*
* XLlFifo_TxVacancy returns the number of unused 32 bit words available
* (vacancy) in the send channel of the FIFO specified by <i>InstancePtr</i>.
*
* The xps_ll_fifo core uses the same fifo to store data values and frame length
* values. Upon initialization, the XLlFifo_TxVacancy gives the value of
* FIFO_WIDTH - 1, which means one length value used (a reserved fifo location)
* and no data values yet present.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
*
* @return   XLlFifo_TxVacancy returns the vacancy count in 32-bit words for
*           the specified FIFO.
*
* @note
* C-style signature
*    u32 XLlFifo_TxVacancy(XLlFifo *InstancePtr)
*
*****************************************************************************/
#define XLlFifo_TxVacancy(InstancePtr) \
	XStrm_TxVacancy(&((InstancePtr)->TxStreamer))

/*****************************************************************************/
/**
* @brief Starts a transmit of the given byte length.
*
* XLlFifo_TxSetLen begins a hardware transfer of <i>Bytes</i> bytes out of the
* transmit channel of the FIFO specified by <i>InstancePtr</i>.
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
* @param[in]    Bytes Specifies the frame length in bytes.
*
* @note
* C-style signature
*    void XLlFifo_TxSetLen(XLlFifo *InstancePtr, u32 Bytes)
*
******************************************************************************/
#define XLlFifo_TxSetLen(InstancePtr, Bytes) \
	XStrm_TxSetLen(&((InstancePtr)->TxStreamer), (Bytes))

/*****************************************************************************/
/**
* @brief Writes bytes to the FIFO transmit channel.
*
* XLlFifo_Write writes <i>Bytes</i> bytes of the block of memory, referenced by
* <i>BufPtr</i>, to the transmit channel of the FIFO referenced by
* <i>InstancePtr</i>.
*
* Care must be taken to ensure that the number of bytes written with one or
* more calls to XLlFifo_Write() matches the number of bytes given in the next
* call to XLlFifo_TxSetLen().
*
* @param[in,out]    InstancePtr References the FIFO on which to operate.
* @param[in]    BufPtr Specifies the memory address of data to write.
* @param[in]    Bytes Specifies the number of bytes to write.
*
* @note
* Error handling is handled through hardware exceptions and interrupts.
*
* C-style signature
*    void XLlFifo_Write(XLlFifo *InstancePtr, void *BufPtr, unsigned Bytes)
*
******************************************************************************/
#define XLlFifo_Write(InstancePtr, BufPtr, Bytes) \
	XStrm_Write(&((InstancePtr)->TxStreamer), (BufPtr), (Bytes))


/*****************************************************************************/
/**
* @brief Writes the transmit destination address.
*
* XLlFifo_WriteTdr writes to the Transmit Destination Register (TDR)
*
* The TDR stores the destination address corresponding to the packet to  be
* transmitted. When presenting a transmit packet to the AXI4-Stream FIFO core
* the following sequence should be followed
*  - Write the destination address into TDR first,
*  - Write the packet data to the Transmit Data FIFO next
*  - Write the length of the packet into the Transmit Length Register.
*
* @param[in]    InstancePtr References the FIFO on which to operate.
* @param[in]    Tdest Transmit Destination address to be written to TDR.
*
* @note
* C-style signature
*    void XLlFifo_WriteTdr(XLlFifo *InstancePtr, u32 Tdest)
*
******************************************************************************/
#define XLlFifo_WriteTdr(InstancePtr, Tdest) \
	XLlFifo_WriteReg((InstancePtr)->BaseAddress, XLLF_TDR_OFFSET, \
			  Tdest & 0xF)

/*****************************************************************************/
/**
* @brief Reads the receive destination address.
*
* XLlFifo_ReadTdr returns the contents of the Receive Destination Register(RDR).
*
* The RDR contains destination address corresponding to the valid packet that
* is received. The RDR should only be read when a receive packet is available
* for processing (the receive occupancy is not zero).
* Once the RDR is read, the receive packet data should be read from the receive
* data FIFO before the RDR is read again. The RDR values are stored in the
* receive data FIFO by the AXI4-Stream FIFO core with the data of each packet.
* The RDR value for the subsequent packet to be processed is moved to the RDR
* when the previous RDR value has been read.
*
* @param[in]    	InstancePtr References the FIFO on which to operate.
*
* @return   	The Receive Destination address  read from the RDR.
*
* @note
* C-style signature
*    u32 XLlFifo_ReadRdr(XLlFifo *InstancePtr)
*
******************************************************************************/
#define XLlFifo_ReadRdr(InstancePtr) 				\
	XLlFifo_ReadReg((InstancePtr)->BaseAddress, XLLF_RDR_OFFSET)

/************************** Function Prototypes ******************************/
/*
 * Initialization functions xllfifo.c
 */
int XLlFifo_CfgInitialize(XLlFifo *InstancePtr,
			XLlFifo_Config *Config, UINTPTR EffectiveAddress);
void XLlFifo_Initialize(XLlFifo *InstancePtr, UINTPTR BaseAddress);
#ifndef SDT
XLlFifo_Config *XLlFfio_LookupConfig(u32 DeviceId);
#else
XLlFifo_Config *XLlFfio_LookupConfig(UINTPTR BaseAddress);
#endif
u32 XLlFifo_iRxOccupancy(XLlFifo *InstancePtr);
u32 XLlFifo_iRxGetLen(XLlFifo *InstancePtr);
u32 XLlFifo_iTxVacancy(XLlFifo *InstancePtr);
void XLlFifo_iTxSetLen(XLlFifo *InstancePtr, u32 Bytes);
u32 XLlFifo_RxGetWord(XLlFifo *InstancePtr);
void XLlFifo_TxPutWord(XLlFifo *InstancePtr, u32 Word);

#ifdef __cplusplus
}
#endif
#endif				/* XLLFIFO_H */
/** @} */
