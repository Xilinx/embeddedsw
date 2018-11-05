/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/**
*
* @file xhdcp1x_port.h
* @addtogroup hdcp1x_v4_2
* @{
*
* This header file contains the external declarations associated with the
* HDCP port driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 2.00  MG     01/20/16 Added callback handler to XHdcp1x_PortPhyIfAdaptor
* 3.10  yas    07/28/16 Added callback handler SetRepeater to
*                       XHdcp1x_PortPhyIfAdaptor
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_PORT_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xhdcp1x.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * This typedef defines the different types of handlers that can be registered
 * to service interrupt requests from the HDCP port instance
 */
typedef enum {
	XHDCP1X_PORT_HANDLER_AUTHENTICATE = 1, /**< An (re)auth request */
} XHdcp1x_PortHandlerType;

/**
 * The typedef defines the HDCP port adaptor table.  This contains a series of
 * functions that map the external interface of the HDCP port device to the
 * underlying physical interface that it is running over
 */
typedef struct XHdcp1x_PortPhyIfAdaptorS {
	int (*Init)(XHdcp1x *);			/**< Initialization function */
	int (*Enable)(XHdcp1x *);		/**< Enable function */
	int (*Disable)(XHdcp1x *);		/**< Disable function */
	int (*Read)(const XHdcp1x *, u8, void *, u32); /**< Reg read */
	int (*Write)(XHdcp1x *, u8, const void *, u32); /**< Reg write */
	int (*IsCapable)(const XHdcp1x *);	/**< Tests for HDCP capable */
	int (*IsRepeater)(const XHdcp1x *);	/**< Tests for repeater */
	int (*SetRepeater)(XHdcp1x *, u8);	/**< Sets repeater */
	int (*GetRepeaterInfo)(const XHdcp1x *, u16 *); /**< Gets repeater
							  *  info */
	void (*IntrHandler)(XHdcp1x *, u32); /**< Interrupt handler */
	void (*CallbackHandler)(void *CallbackRef); /**< Callback handler */
} XHdcp1x_PortPhyIfAdaptor;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* This macro converts from an unsigned integer to a little endian formatted
* buffer
*
* @param	buf the buffer to write to
* @param	uint the unsigned integer to convert
* @param	numbits the number of bits within the unsigned integer to use
*
* @return	None.
*
* @note		The value of the "uint" parameter is destroyed by a call to this
*		macro
*
******************************************************************************/
#define XHDCP1X_PORT_UINT_TO_BUF(buf, uint, numbits)			\
	if ((numbits) > 0) {						\
		int byte;						\
		for (byte = 0; byte <= (int)(((numbits) - 1) >> 3); byte++) { \
			buf[byte] = (uint8_t) (uint & 0xFFu);		\
			uint >>= 8;					\
		}							\
	}

/*****************************************************************************/
/**
* This macro converts from a little endian formatted buffer to an unsigned
* integer value
*
* @param	uint the unsigned integer to write
* @param	buf the buffer to convert
* @param	numbits the number of bits within the buffer to use
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XHDCP1X_PORT_BUF_TO_UINT(uint, buf, numbits)			\
	if ((numbits) > 0) {						\
		int byte;						\
		uint = 0;	 					\
		for (byte = (((numbits) - 1) >> 3); byte >= 0; byte--) { \
			uint <<= 8;					\
			uint  |= buf[byte];				\
		}							\
	}

/*****************************************************************************/
/**
* This macro sets a bit within a little endian formatted buffer
*
* @param	buf the buffer to write to
* @param	bitnum the bit to set
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XHDCP1X_PORT_BSET_IN_BUF(buf, bitnum)	\
	buf[(bitnum) >> 3] |=  (1u << ((bitnum) & 0x07u));

/*****************************************************************************/
/**
* This macro clears a bit within a little endian formatted buffer
*
* @param	buf the buffer to write to
* @param	bitnum the bit to clear
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XHDCP1X_PORT_BCLR_IN_BUF(buf, bitnum)	\
	buf[(bitnum) >> 3] &= ~(1u << ((bitnum) & 0x07u));

/*****************************************************************************/
/**
* This macro tests a bit within a little endian formatted buffer
*
* @param	buf the buffer containing the bit to test
* @param	bitnum the bit to test
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XHDCP1X_PORT_BTST_IN_BUF(buf, bitnum)	\
	(buf[(bitnum) >> 3] & (1u << ((bitnum) & 0x07u)))

/************************** Function Prototypes ******************************/

const XHdcp1x_PortPhyIfAdaptor *XHdcp1x_PortDetermineAdaptor(
		XHdcp1x *InstancePtr);
int XHdcp1x_PortSetCallback(XHdcp1x *InstancePtr, u32 HandlerType,
	XHdcp1x_Callback Callback, void *Parameter);

int XHdcp1x_PortEnable(XHdcp1x *InstancePtr);
int XHdcp1x_PortDisable(XHdcp1x *InstancePtr);
int XHdcp1x_PortIsCapable(const XHdcp1x *InstancePtr);
int XHdcp1x_PortIsRepeater(const XHdcp1x *InstancePtr);
int XHdcp1x_PortSetRepeater(XHdcp1x *InstancePtr, u8 RptrConf);
int XHdcp1x_PortGetRepeaterInfo(XHdcp1x *InstancePtr, u16 *Info);
int XHdcp1x_PortRead(const XHdcp1x *InstancePtr, u8 Offset, void *Buf,
	u32 BufSize);
int XHdcp1x_PortWrite(XHdcp1x *InstancePtr, u8 Offset, const void *Buf,
	u32 BufSize);

void XHdcp1x_PortHandleInterrupt(XHdcp1x *InstancePtr, u32 IntCause);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_PORT_H */
/** @} */
