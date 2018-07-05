/******************************************************************************
*
* Copyright (C) 2007 - 2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xhwicap.h
* @addtogroup hwicap_v11_2
* @{
* @details
*
* The Xilinx XHwIcap driver supports the Xilinx Hardware Internal Configuration
* Access Port (HWICAP) device.
*
* The HWICAP device is used for reconfiguration of select FPGA resources
* as well as loading partial bitstreams from the system memory through the
* Internal Configuration Access Port (ICAP).
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the HWICAP device.
*
* XHwIcap_CfgInitialize() API is used to initialize the HWICAP device.
* The user needs to first call the XHwIcap_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XHwIcap_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XHwIcap_IntrHandler for handling
* the interrupt from the HWICAP device. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions. The callback functions are invoked by the interrupt handler based
* on the interrupt source.
*
* The driver supports interrupt mode only for writing to the ICAP device and
* is NOT supported for reading from the ICAP device.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XHwIcap driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
*
* @note
*
* There are a few items to be aware of when using this driver:
* 1) Only Virtex4, Virtex5, Virtex6,  Spartan6, 7 series and Zynq devices are
*    supported.
* 2) The ICAP port is disabled when the configuration mode, via the MODE pins,
* is set to Boundary Scan/JTAG. The ICAP is enabled in all other configuration
* modes and it is possible to configure the device via JTAG in all
* configuration modes.
* 3) Reading or writing to columns containing SRL16's or LUT RAM's can cause
* corruption of data in those elements. Avoid reading or writing to columns
* containing SRL16's or LUT RAM's.
* 4) Only the LUT and SRL are accesible, all other features of the slice are
* not available through this interface.
* 5) The Spartan6 devices access is 16-bit access and is 32 bit for all
* other devices.
* 6) In a Zynq device the ICAP needs to be selected using the
*    XDcfg_SelectIcapInterface API of the DevCfg driver (clear the PCAP_PR bit
*    of Control register in the Device Config Interface)  before it can be
*    accessed using the HwIcap.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/17/03 First release
* 1.01a bjb  04/10/06 V4 Support
* 2.00a sv   09/28/07 First release for the FIFO mode
* 2.01a ecm  04/08/08 Updated data structures to include the V5FXT parts.
* 3.00a sv   11/28/08 Added the API for initiating Abort while reading/writing
*		      from the ICAP.
* 3.01a sv   10/21/09 Corrected the IDCODE definitions for some of the
*                     V5 FX parts in xhwicap_l.h. Corrected the V5 BOOTSTS and
*                     CTL_1 Register definitions in xhwicap_i.h file as they
*                     were wrongly defined.
* 4.00a hvm  12/1/09  Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  04/02/10 Added S6 device support
* 5.01a hvm  07/06/10 In XHwIcap_DeviceRead function, a read bit mask
*		      verification is added after all the data bytes are read
*		      from READ FIFO.The Verification of the read bit mask
*		      at the begining of reading of bytes is removed.
*		      Removed the code that adds wrong data byte before the
*		      CRC bytes in the XHwIcap_DeviceWriteFrame function for S6
*		      (CR560534).
* 5.02a hvm  10/06/10 Updated to support AXI HWICAP
* 5.03a hvm  15/4/11 Updated with V6 CXT device definitions.
*
* 6.00a hvm  08/01/11 Added support for K7 devices.
* 7.00a bss  03/14/12 Added support for 8/16/32 ICAP Data Widths - CR 620085
*		      Added support for Lite Mode(no Write FIFO) - CR 601748
*		      Added Virtex7,Artix7 and Zynq Idcodes-CR647140,CR643295
*		      ReadId API is added to desync after lock up during
*			configuration CR 637538
* 8.00a bss  06/20/12 Deleted ReadId API in xhwicap_srp.c and Hang mask
*			definition in xhwicap_l.h as per CR 656162
* 8.01a bss  04/18/13  Updated xhwicap.c to fix compiler warnings. CR#704814
* 		       Added the define XHI_COR_1 for CR718042
* 9.0   adk  19/12/13 Updated as per the New Tcl API's
* 9.0   bss  02/20/14 Modified xhwicap.c, xhwicap_l.h, xhwicap_i.h and tcl
*		      to support Kintex8, kintexu and virtex72000T family
*		      devices.
* 10.0  bss  6/24/14  Removed support for families older than 7 series.
*		      Modified driver tcl not to generate family.h.
*		      Removed IDCODE lookup logic in XHwIcap_CfgInitialize
*		      in xhwicap.c.
*		      Removed IDCODE macros from xhwicap_i.h.
* 10.0  bss  7/10/14  Fix compilation failure for designs other than 32 bit
*		      data width of HWICAP in xhwicap.c.
* 10.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                      Changed the prototype of XHwIcap_CfgInitialize API.
* 10.1   nsk  01/06/16 Removed xhwicap_clb_srinv.h, xhwicap_clb_ff.h,
*                      xhwicap_clb_lut.h files
*                      Removed xhwicap_lut.c and xhwicap_ff.c examples
*                      Removed defines
*                      XHI_FAR_MAJOR_FRAME_MASK
*                      XHI_FAR_MINOR_FRAME_MASK
*                      XHI_FAR_MAJOR_FRAME_SHIFT
*                      XHI_FAR_MINOR_FRAME_SHIFT
*                      XHI_C0R_1
*                      Updated XHI_FAR_COLUMN_ADDR_MASK to 0x3FF
*                      Updated XHI_FAR_BLOCK_SHIFT to 23
*                      Updated XHI_FAR_TOP_BOTTOM_SHIFT to 22
*                      Updated XHI_FAR_ROW_ADDR_SHIFT to 17
*                      Updated XHI_NUM_FRAME_BYTES to 404
*                      Updated XHI_NUM_FRAME_WORDS to 101
*                      Updated XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME to 202
*                      CR# 909615.
* 10.2   mi   09/22/16 Fixed compilation warnings.
* 11.0  ms    01/23/17 Added xil_printf statement in main function for all
*                      examples to ensure that "Successfully ran" and "Failed"
*                      strings are available in all examples. This is a fix
*                      for CR-965028.
*       ms    03/17/17 Added readme.txt file in examples folder for doxygen
*                      generation.
* 11.2 Nava   02/08/19 The current version of the driver is not supported for
*                      families older than 7 series.So removed .o referenced
*                      function prototypes from the header file.
*
* </pre>
*
*****************************************************************************/
#ifndef XHWICAP_H_ /* prevent circular inclusions */
#define XHWICAP_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xhwicap_i.h"
#include "xhwicap_l.h"
#include <xstatus.h>
#include "xparameters.h"

/************************** Constant Definitions ****************************/
#define DEVICE_TYPE_7SERIES		1
#define DEVICE_TYPE_ULTRA		2
#define DEVICE_TYPE_ULTRA_PLUS		3

#define NUM_7SERIES_IDCODES		32
#define NUM_ULTRA_SERIES_IDCODES	13
#define NUM_ULTRA_PLUS_SERIES_IDCODES	12

#define PCAP_CR_OFFSET         0xFFCA3008 /**< PCAP CR Register */

/************************** Type Definitions ********************************/

/**************************** Type Definitions *******************************/

/**
 * The handler data type allows the user to define a callback function to
 * handle the asynchronous processing of the HwIcap driver. The application
 * using this driver is expected to define a handler of this type to support
 * interrupt driven mode. The handler executes in an interrupt context such
 * that minimal processing should be performed.
 *
 * @param 	CallBackRef is a callback reference passed in by the
 *		application layer when setting the callback functions, and
 *		passed back to the upper layer when the callback is invoked.
 *		Its type is unimportant to the driver component, so it is a
 *		void pointer.
 * @param 	StatusEvent indicates one or more status events that occurred.
 *		See the XHwIcap_SetInterruptHandler for details on the status
 *		events that can be passed in the callback.
 * @param	WordCount indicates how many words of data were successfully
 *		transferred.  This may be less than the number of words
 *		requested if there was an error.
 */
typedef void (*XHwIcap_StatusHandler) (void *CallBackRef, u32 StatusEvent,
					u32 WordCount);


/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Device ID  of device */
	UINTPTR BaseAddress;	/**< Register base address */
	int IcapWidth;		/**< Width of ICAP */
	int IsLiteMode;		/**< IsLiteMode, 0 not
					present, 1 present */

} XHwIcap_Config;

 /**
  * The XHwIcap driver instance data. The user is required to allocate a
  * variable of this type for every HwIcap device in the system. A pointer
  * to a variable of this type is then passed to the driver API functions.
  */
typedef struct {
	XHwIcap_Config HwIcapConfig; /**< Instance of the config struct. */
	u32 IsReady;		     /**< Device is initialized and ready */
	int IsPolled;		     /**< Device is in polled mode */
	u32 DeviceIdCode;	     /**< IDCODE of targeted device */
	u32 DeviceFamily;		 /**< Targeted device family */
	u32 BytesPerFrame;	     /**< Number of Bytes per minor Frame */
	u32 WordsPerFrame;	     /**< Number of Words per minor Frame */

#if XPAR_HWICAP_0_ICAP_DWIDTH == 8
	u8 *SendBufferPtr;
#elif XPAR_HWICAP_0_ICAP_DWIDTH == 16
	u16 *SendBufferPtr;
#else
	u32 *SendBufferPtr;
#endif
	u32 RequestedWords;	     /**< Number of Words to transfer  */
	u32 RemainingWords; 	     /**< Number of Words left to transfer  */
	int IsTransferInProgress;    /**< A transfer is in progress */
	XHwIcap_StatusHandler StatusHandler; /**< Interrupt handler callback */
	void *StatusRef;	     /**< Callback ref. for the interrupt
						* handler */

} XHwIcap;

/***************** Macro (Inline Functions) Definitions *********************/


/****************************************************************************/
/**
*
* Write data to the Write FIFO.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	Data is the 32-bit value to be written to the FIFO.
*
* @return	None.
*
* @note		C-style Signature:
* 		void XHwIcap_FifoWrite(XHwIcap *InstancePtr, u32 Data);
*
*****************************************************************************/
#define XHwIcap_FifoWrite(InstancePtr, Data) 				\
	(XHwIcap_WriteReg(((InstancePtr)->HwIcapConfig.BaseAddress),	\
		XHI_WF_OFFSET, (Data)))

/****************************************************************************/
/**
*
* Read data from the Read FIFO.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	The 32-bit Data read from the FIFO.
*
* @note		C-style Signature:
* 		u32 XHwIcap_FifoRead(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_FifoRead(InstancePtr) 					\
(XHwIcap_ReadReg(((InstancePtr)->HwIcapConfig.BaseAddress), XHI_RF_OFFSET))

/****************************************************************************/
/**
*
* Set the number of words to be read from the Icap in the Size register.
*
* The Size Register holds the number of 32 bit words to transfer from the
* the Icap to the Read FIFO of the HwIcap device.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	Data is the size in words.
*
* @return	None.
*
* @note		C-style Signature:
*		void XHwIcap_SetSizeReg(XHwIcap *InstancePtr, u32 Data);
*
*****************************************************************************/
#define XHwIcap_SetSizeReg(InstancePtr, Data) \
	(XHwIcap_WriteReg(((InstancePtr)->HwIcapConfig.BaseAddress), \
		XHI_SZ_OFFSET, (Data)))

/****************************************************************************/
/**
*
* Get the contents of the Control register.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	A 32-bit value representing the contents of the Control
*		register.
*
* @note		u32 XHwIcap_GetControlReg(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_GetControlReg(InstancePtr) \
 (XHwIcap_ReadReg(((InstancePtr)->HwIcapConfig.BaseAddress), XHI_CR_OFFSET))


/****************************************************************************/
/**
*
* Set the Control Register to initiate a configuration (write) to the device.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	None.
*
* @note		C-style Signature:
*		void XHwIcap_StartConfig(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_StartConfig(InstancePtr) \
 (XHwIcap_WriteReg(((InstancePtr)->HwIcapConfig.BaseAddress), XHI_CR_OFFSET, \
 	(XHwIcap_GetControlReg(InstancePtr) & 				      \
 	(~ XHI_CR_READ_MASK)) | XHI_CR_WRITE_MASK))


/****************************************************************************/
/**
*
* Set the Control Register to initiate a ReadBack from the device.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	None.
*
* @note		C-style Signature:
*		void XHwIcap_StartReadBack(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_StartReadBack(InstancePtr) \
 (XHwIcap_WriteReg(((InstancePtr)->HwIcapConfig.BaseAddress) , XHI_CR_OFFSET, \
 	(XHwIcap_GetControlReg(InstancePtr) & 				      \
 	(~ XHI_CR_WRITE_MASK)) | XHI_CR_READ_MASK))


/****************************************************************************/
/**
*
* Get the contents of the status register.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	A 32-bit value representing the contents of the status register.
*
* @note		u32 XHwIcap_GetStatusReg(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_GetStatusReg(InstancePtr) \
(XHwIcap_ReadReg(((InstancePtr)->HwIcapConfig.BaseAddress), XHI_SR_OFFSET))

/****************************************************************************/
/**
*
* This macro checks if the last Read/Write of the data to the Read/Write FIFO
* of the HwIcap device is completed.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return
*		- TRUE if the Read/Write to the FIFO's is completed.
*		- FALSE if the Read/Write to the FIFO's is NOT completed..
*
* @note		C-Style signature:
*		int XHwIcap_IsTransferDone(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_IsTransferDone(InstancePtr)			\
	((InstancePtr->IsTransferInProgress) ? FALSE : TRUE)

/****************************************************************************/
/**
*
* This macro checks if the last Read/Write to the ICAP device in the FPGA
* is completed.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return
*		- TRUE if the last Read/Write(Config) to the ICAP is NOT
*		completed.
*		- FALSE if the Read/Write(Config) to the ICAP is completed..
*
* @note		C-Style signature:
*		int XHwIcap_IsDeviceBusy(XHwIcap *InstancePtr);
*
*****************************************************************************/
#define XHwIcap_IsDeviceBusy(InstancePtr)			\
	((XHwIcap_GetStatusReg(InstancePtr) & XHI_SR_DONE_MASK) ? \
				FALSE : TRUE)

/*****************************************************************************/
/**
*
* This macro enables the global interrupt in the Global Interrupt Enable
* Register (GIER) so that the interrupt output from the HwIcap device is
* enabled. Interrupts enabled using XHwIcap_IntrEnable() will not occur until
* the global interrupt enable bit is set by using this macro.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
*
* @return	None.
*
* @note		C-Style signature:
*		void XHwIcap_IntrGlobalEnable(InstancePtr)
*
******************************************************************************/
#define XHwIcap_IntrGlobalEnable(InstancePtr)				\
	XHwIcap_WriteReg((InstancePtr)->HwIcapConfig.BaseAddress,	\
				XHI_GIER_OFFSET, XHI_GIER_GIE_MASK)

/*****************************************************************************/
/**
*
* This macro disables the global interrupt in the Global Interrupt Enable
* Register (GIER) so that the interrupt output from the HwIcap device is
* disabled.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
*
* @return	None.
*
* @note		C-Style signature:
*		void XHwIcap_IntrGlobalDisable(InstancePtr)
*
******************************************************************************/
#define XHwIcap_IntrGlobalDisable(InstancePtr)				\
	XHwIcap_WriteReg((InstancePtr)->HwIcapConfig.BaseAddress,	\
				XHI_GIER_OFFSET, 0x0)

/*****************************************************************************/
/**
*
* This macro returns the interrupt status read from Interrupt Status
* Register(IPISR). Use the XHI_IPIXR_* constants defined in xhwicap_l.h
* to interpret the returned value.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
*
* @return	The contents read from the Interrupt Status Register.
*
* @note		C-Style signature:
*		u32 XHwIcap_IntrGetStatus(InstancePtr)
*
******************************************************************************/
#define XHwIcap_IntrGetStatus(InstancePtr)				\
	XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, 	\
				XHI_IPISR_OFFSET)

/*****************************************************************************/
/**
*
* This macro disables the specified interrupts in the Interrupt Enable
* Register. It is non-destructive in that the register is read and only the
* interrupts specified is changed.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
* @param	IntrMask is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XHI_IPIXR_*_MASK bits defined in xhwicap_l.h.
*
* @return	None.
*
* @note		Signature:
*		void XHwIcap_IntrDisable(XHwIcap *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XHwIcap_IntrDisable(InstancePtr, IntrMask)           \
XHwIcap_WriteReg((InstancePtr)->HwIcapConfig.BaseAddress, 	\
			XHI_IPIER_OFFSET, \
	XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, \
		XHI_IPIER_OFFSET) & (~ (IntrMask & XHI_IPIXR_ALL_MASK)));\
		(InstancePtr)->IsPolled = TRUE;

/*****************************************************************************/
/**
*
* This macro enables the specified interrupts in the Interrupt Enable
* Register. It is non-destructive in that the register is read and only the
* interrupts specified is changed.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
* @param	IntrMask is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XHI_IPIXR_*_MASK bits defined in xhwicap_l.h.
*
* @return	None.
*
* @note		Signature:
*		void XHwIcap_IntrEnable(XHwIcap *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XHwIcap_IntrEnable(InstancePtr, IntrMask) \
	XHwIcap_WriteReg((InstancePtr)->HwIcapConfig.BaseAddress, 	\
			XHI_IPIER_OFFSET, \
	(XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, \
		XHI_IPIER_OFFSET) | ((IntrMask) & XHI_IPIXR_ALL_MASK))); \
		(InstancePtr)->IsPolled = FALSE;

/*****************************************************************************/
/**
*
* This macro returns the interrupt status read from Interrupt Enable
* Register(IIER). Use the XHI_IPIXR_* constants defined in xhwicap_l.h
* to interpret the returned value.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
*
* @return	The contents read from the Interrupt Enable Register.
*
* @note		C-Style signature:
*		u32 XHwIcap_IntrGetEnabled(InstancePtr)
*
******************************************************************************/
#define XHwIcap_IntrGetEnabled(InstancePtr)				\
	XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, 	\
			XHI_IPIER_OFFSET)

/*****************************************************************************/
/**
*
* This macro clears the specified interrupts in the Interrupt Status
* Register (IPISR).
*
* @param	InstancePtr is a pointer to the HwIcap instance.
* @param	IntrMask contains the interrupts to be cleared.
*
* @return	None.
*
* @note		Signature:
*		void XHwIcap_DisableIntr(XHwIcap *InstancePtr, u32 IntrMask)
*
******************************************************************************/
#define XHwIcap_IntrClear(InstancePtr, IntrMask)           \
	XHwIcap_WriteReg((InstancePtr)->HwIcapConfig.BaseAddress, 	\
			XHI_IPISR_OFFSET, \
		XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, \
		XHI_IPISR_OFFSET) | ((IntrMask) & XHI_IPIXR_ALL_MASK))

/*****************************************************************************/
/**
*
* This macro returns the vacancy of the Write FIFO. This indicates the
* number of words that can be written to the Write FIFO before it becomes
* full.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
*
* @return	The contents read from the Write FIFO Vacancy Register.
*
* @note		C-Style signature:
*		u32 XHwIcap_GetWrFifoVacancy(InstancePtr)
*
******************************************************************************/
#define XHwIcap_GetWrFifoVacancy(InstancePtr)				\
 XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, XHI_WFV_OFFSET)

/*****************************************************************************/
/**
*
* This macro returns the occupancy  of the Read FIFO.
*
* @param	InstancePtr is a pointer to the HwIcap instance.
*
* @return	The contents read from the Read FIFO Occupancy Register.
*
* @note		C-Style signature:
*		u32 XHwIcap_GetRdFifoOccupancy(InstancePtr)
*
******************************************************************************/
#define XHwIcap_GetRdFifoOccupancy(InstancePtr)		\
 XHwIcap_ReadReg((InstancePtr)->HwIcapConfig.BaseAddress, XHI_RFO_OFFSET)

/************************** Function Prototypes *****************************/

/*
 * Functions in the xhwicap.c
 */
int XHwIcap_CfgInitialize(XHwIcap *InstancePtr, XHwIcap_Config *ConfigPtr,
				UINTPTR EffectiveAddr);
int XHwIcap_DeviceWrite(XHwIcap *InstancePtr, u32 *FrameBuffer, u32 NumWords);
int XHwIcap_DeviceRead(XHwIcap *InstancePtr, u32 *FrameBuffer, u32 NumWords);
void XHwIcap_Reset(XHwIcap *InstancePtr);
void XHwIcap_FlushFifo(XHwIcap *InstancePtr);
void XHwIcap_Abort(XHwIcap *InstancePtr);

/*
 * Functions in xhwicap_sinit.c.
 */
XHwIcap_Config *XHwIcap_LookupConfig(u16 DeviceId);

/*
 * Functions in the xhwicap_srp.c
 */
int XHwIcap_CommandDesync(XHwIcap *InstancePtr);
int XHwIcap_CommandCapture(XHwIcap *InstancePtr);
u32 XHwIcap_GetConfigReg(XHwIcap *InstancePtr, u32 ConfigReg, u32 *RegData);

/*
 *  Function in xhwicap_selftest.c
 */
 int XHwIcap_SelfTest(XHwIcap *InstancePtr);

/*
 *  Function in xhwicap_intr.c
 */
void XHwIcap_IntrHandler(void *InstancePtr);
void XHwIcap_SetInterruptHandler(XHwIcap * InstancePtr, void *CallBackRef,
			   XHwIcap_StatusHandler FuncPtr);

/*
 * Functions in the xhwicap_device_read_frame.c
 */
int XHwIcap_DeviceReadFrame(XHwIcap *InstancePtr, long Top,
				long Block, long HClkRow,
				long MajorFrame, long MinorFrame,
				u32 *FrameBuffer);

/*
 * Functions in the xhwicap_device_write_frame.c
 */
int XHwIcap_DeviceWriteFrame(XHwIcap *InstancePtr, long Top,
				long Block, long HClkRow,
				long MajorFrame, long MinorFrame,
				u32 *FrameData);

/************************** Variable Declarations ***************************/

#ifdef __cplusplus
}
#endif

#endif

/** @} */
