/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file xclk_wiz.h
*
* @addtogroup clk_wiz_v1_2
* @{
* @details
*
* This is main header file of the Xilinx Clock Wizard driver
*
* <b>Clock wizard Overview</b>
* The Clock monitor feature is a part of Clocking Wizard IP.
* It allows a user to monitor the clock in  a given system for clock loss,
* out of range.In Zynq or Zynq Ultrascale, the clock monitored can be either
* a PS or a PL clock. In FPGAs, the clock monitored can be an arbitrary clock.
*
* <b>Clock wizard Features</b>
*  -  Clock Stop – The clock is flat lined.
*  -  Clock Glitch – Variation in the duty cycle of the clock.
*  -  Overrun  - The number of transitions in the clock are more than expected
*  -  Underrun - The number of transitions in the clock are less than expected.
*  -  Overrun and Underrun are termed as “out of range” errors.
*
* <b>Clock Monitor Configurations</b>
*  -  The GUI in IPI allows for the following configurations
*  -  Enable 4 User clocks
*  -  Enable User clock PLL.
*  -  Select reference clock frequency
*  -  Select 4 user clock frequencies
*
* <b>Pre-Requisite's</b>
*
*
* <b>Subsystem Driver Usage</b>
*
*
* <b>Memory Requirement</b>
*
*
* <b>Interrupt Service</b>
*
*
* <b>Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b>Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier.  By default, asserts are turned on and
* it is recommended that application developers leave asserts on during
* development.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 02/12/16 Initial version for Clock Wizard
* 1.1 ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*              	   strings are available in all examples. This is a fix
*                  for CR-965028.
* 1.2 ms  03/02/17 Fixed compilation errors in xclk_wiz_intr.c, xclk_wiz_g.c
*                  and warnings in xclk_wiz.c files. Fix for CR-970507.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
* </pre>
*
******************************************************************************/

#ifndef XCLK_WIZ_H_   /* prevent circular inclusions */
#define XCLK_WIZ_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifdef __MICROBLAZE__
#include "xenv.h"
#else
#include "xil_types.h"
#include "xil_cache.h"
#endif

#include "xclk_wiz_hw.h"

/************************** Constant Definitions *****************************/
/** @name Interrupt Types for setting Callbacks
 * @{
*/
#define XCLK_WIZ_HANDLER_CLK_OUTOF_RANGE	1
#define XCLK_WIZ_HANDLER_CLK_GLITCH		2
#define XCLK_WIZ_HANDLER_CLK_STOP		3
#define XCLK_WIZ_HANDLER_CLK_OTHER_ERROR	4

/*@}*/

/*****************************************************************************/
/**
* The configuration structure for CLK_WIZ Controller
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
	u32 DeviceId;	         /**< Device Id */
	UINTPTR BaseAddr;        /**< Base address of CLK_WIZ Controller */
	u32 EnableClkMon;        /**< It enables the Clock Monitor*/
	u32 EnableUserClkWiz0;   /**< Enable user clk 0 */
	u32 EnableUserClkWiz1;   /**< Enable user clk 1 */
	u32 EnableUserClkWiz2;   /**< Enable user clk 2 */
	u32 EnableUserClkWiz3;   /**< Enable user clk 3 */
	double RefClkFreq;       /**< Frequency of Reference Clock */
	double UserClkFreq0;   /**< Hold the  user clock frequency0 */
	double UserClkFreq1;   /**< Hold the  user clock frequency1 */
	double UserClkFreq2;   /**< Hold the  user clock frequency2 */
	double UserClkFreq3;   /**< Hold the  user clock frequency3 */
	double Precision;      /**< Holds the value of precision */
	u8  EnablePll0;        /**< specify if this user clock is
				going as input to the PLL/MMCM */
	u8  EnablePll1;        /**< specify if this user clock is
				going as input to the PLL/MMCM */
} XClk_Wiz_Config;

/*****************************************************************************/
/**
*
* Callback type for all interrupts defined.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XCLK_WIZ_ISR_*_MASK constants defined in xclmon_hw.h.
*
* @return	None
*
* @note		None
*
 *****************************************************************************/
typedef void (*XClk_Wiz_CallBack) (void *CallBackRef, u32 Mask);

/**
* The XClk_Wiz driver instance data.
* An instance must be allocated for each CLK_WIZ in use.
*/
typedef struct {

	XClk_Wiz_Config Config;		/**< GUI Configuration */
	u32  ClkWizIntrStatus;	/**< Clock Stop, Clock Overrun,
				  *  Clock Underrun, Clock Glitch
				  * Interrupt Status */
	u32  ClkIntrEnable;	/**< Interrupt enable for
				  *  Clock Stop, Clock Overrun,
				  *  Clock Underrun, Clock Glitch */
	XClk_Wiz_CallBack ClkOutOfRangeCallBack;/**< Callback for Clock out
						  *  of range Under flow
						  * .or over flow */
	void *ClkOutOfRangeRef;	 /**< To be passed to the clock
				   *  out of range call back */
	XClk_Wiz_CallBack ClkGlitchCallBack; /**< Callback for
						   *  Clock Glitch */
	void *ClkGlitchRef;		/**< To be passed to the
					  *  clock glitch call back */
	XClk_Wiz_CallBack ClkStopCallBack;	/**< Callback for Clock stop */
	void *ClkStopRef;			/**< To be passed to the clock
						  *  stop call back */
	XClk_Wiz_CallBack ErrorCallBack; /**< Call back function
					   *  for rest all errors */
	void *ErrRef; /**< To be passed to the Error Call back */
	u32 IsReady; /**< Driver is ready */
} XClk_Wiz;

/************************** Macros Definitions *******************************/

/************************* Bit field operations ******************************/
/* Setting and resetting bits */
/*****************************************************************************/
/**
*
* XCLK_WIZ_BIT_SET is used to set bit in a register.
*
* @param	BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCLK_WIZ_BIT_SET(UINTPTR BaseAddress,u32 RegisterOffset,\
							u32 BitMask) {
	XClk_Wiz_WriteReg((BaseAddress), (RegisterOffset), \
	(XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset)) | BitMask));
}

/****************************************************************************/
/**
*
* XCLK_WIZ_BIT_RESET is used to reset bit in a register.
*
* @param	BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCLK_WIZ_BIT_RESET(UINTPTR BaseAddress, u32 RegisterOffset,
							u32 BitMask) {
	XClk_Wiz_WriteReg((BaseAddress), (RegisterOffset), \
	(XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset) ) & \
			 ~(BitMask)));
}

/****************************************************************************/
/**
*
* XCLK_WIZ_GET_BITFIELD_VALUE is used to get the value of bitfield from
* register.
*
* @param	BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK or _SHIFT
*
* @return	Bit Field Value in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XCLK_WIZ_GET_BITFIELD_VALUE(UINTPTR BaseAddress,
		u32 RegisterOffset, u32 BitMask, u32 BitShift) {
	return ((XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset)) \
		  & (BitMask)) >> (BitShift));
}

/****************************************************************************/
/**
*
* XCLK_WIZ_SET_BITFIELD_VALUE is used to set the value of bitfield from
* register
*
* @param	BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK or _SHIFT
*
* @param	Value is to be set. Passed in u32 format.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCLK_WIZ_SET_BITFIELD_VALUE(UINTPTR BaseAddress, \
		u32 RegisterOffset, u32 BitMask, u32 BitShift, u32 Value) {
	XClk_Wiz_WriteReg((BaseAddress), (RegisterOffset), \
	((XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset)) & \
	  ~(BitMask)) | ((Value) << (BitShift))));
}

/****************************************************************************/
/**
*
* XClk_Wiz_IntrEnable is used to set interrupt mask to enable interrupts. This
* is done before enabling the core.
*
* @param	InstancePtr is a pointer to the CLK_WIZ Instance to be
*		worked on.
*
* @param	Mask Interrupts to be enabled.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XClk_Wiz_IntrEnable(XClk_Wiz *InstancePtr, u32 Mask) {
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, \
			(XCLK_WIZ_IER_OFFSET), \
			(Mask) & (XCLK_WIZ_IER_ALLINTR_MASK));
}

/****************************************************************************/
/**
*
* XClk_Wiz_GetIntrEnable is used to find out which interrupts are
* registered for system
*
* @param	InstancePtr is a pointer to the CLK_WIZ Instance to be
*		worked on.
*
* @return	Bit Mask in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XClk_Wiz_GetIntrEnable(XClk_Wiz *InstancePtr) {
	return XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, \
				XCLK_WIZ_IER_OFFSET);
}

/****************************************************************************/
/**
*
* XClk_Wiz_IntrDisable is used to disable interrupts. This is after disabling
* the core.
*
* @param	InstancePtr is a pointer to the CLK_WIZ Instance to be
*		worked on.
*
* @param	Mask Interrupts to be disabled.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XClk_Wiz_IntrDisable(XClk_Wiz *InstancePtr, u32 Mask) {
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, \
			(XCLK_WIZ_IER_OFFSET), \
			~((Mask) & (XCLK_WIZ_IER_ALLINTR_MASK)));
}

/****************************************************************************/
/**
*
* XClk_Wiz_IntrGetIrq is used to find out which events have triggered the
* interrupt
*
* @param	InstancePtr is a pointer to the CLK_WIZ Instance to be
*		worked on.
*
* @return	Bit Mask in u32 format.
*
* @note		None
*
****************************************************************************/
static inline u32 XClk_Wiz_IntrGetIrq(XClk_Wiz *InstancePtr) {
	return XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr,\
				(XCLK_WIZ_ISR_OFFSET));
}

/****************************************************************************/
/**
*
* XClk_Wiz_IntrAckIrq is acknowledge the events.
*
* @param	InstancePtr is a pointer to the CLK_WIZ Instance to be
*		worked on.
*
* @param	Value is Bit Mask for ack of interrupts.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XClk_Wiz_IntrAckIrq(XClk_Wiz *InstancePtr, u32 Value) {
		XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, \
				(XCLK_WIZ_ISR_OFFSET), \
				((Value) & (XCLK_WIZ_ISR_ALLINTR_MASK)));
}

/************************** Function Prototypes ******************************/

XClk_Wiz_Config *XClk_Wiz_LookupConfig(u32 DeviceId);

u32 XClk_Wiz_CfgInitialize(XClk_Wiz *InstancePtr, XClk_Wiz_Config *Config,
			UINTPTR EffectiveAddr);

void XClk_Wiz_GetInterruptSettings(XClk_Wiz  *InstancePtr);

int XClk_Wiz_SetCallBack(XClk_Wiz *InstancePtr, u32 HandleType,
			void *CallBackFunc, void *CallBackRef);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
