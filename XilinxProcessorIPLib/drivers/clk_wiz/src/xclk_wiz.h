/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCLK_WIZ_H_		/* prevent circular inclusions */
#define XCLK_WIZ_H_

/*****************************************************************************/
/**
*
* @file xclk_wiz.h
* @addtogroup clk_wiz Overview
* @{
* @brief Defines the CLK_WIZ driver software API.
*
* This is the main header file of the Xilinx Clock Wizard driver.
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
* <b>Subsystem Driver Usage</b>
*
* <b>Memory Requirement</b>
*
* <b>Interrupt Service</b>
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
* 1.3 sd  4/09/20 Added versal support.
* 1.4 sd  5/22/20 Added zynqmp set rate.
* 1.6 sd  7/07/23 Added ST support.
* 1.8 sd  8/19/24 Added XClk_Wiz_SetRateHz.
* 1.11 sd  9/27/25 Added XClk_Wiz_GetInputRate.
* 1.11 vlt 12/30/25 Update Doxygen comments to include SDT flow details.
*      ss  7/10/26 Added XClk_Wiz_UpdateDivisors and XClk_Wiz_Reconfig APIs.
* 1.12 ss  8/7/26  Updated clkx5 get/set input and output frequency APIs and
*                  clk_in_sel register control APIs.
* 1.12 ss  8/18/26 Added PFD range validation to divisor calculations.
* 1.12 vn  09/3/26  Fix doxygen warnings.
* 1.12 tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/


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
#include "xplatform_info.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
/** @name Interrupt Types for setting Callbacks
 * @{
*/
/** Clock out-of-range callback type. */
#define XCLK_WIZ_HANDLER_CLK_OUTOF_RANGE	1
/** Clock-glitch callback type. */
#define XCLK_WIZ_HANDLER_CLK_GLITCH		2
/** Clock-stop callback type. */
#define XCLK_WIZ_HANDLER_CLK_STOP		3
/** Other-error callback type. */
#define XCLK_WIZ_HANDLER_CLK_OTHER_ERROR	4

/** Minimum Versal feedback multiplier. */
#define XCLK_M_MIN			4
/** Maximum Versal feedback multiplier. */
#define XCLK_M_MAX			432
/** Minimum Versal input divider. */
#define XCLK_D_MIN			1
/** Maximum Versal input divider. */
#define XCLK_D_MAX			123
/** Minimum Versal VCO frequency in MHz. */
#define XCLK_VCO_MIN			2160
/** Maximum Versal VCO frequency in MHz. */
#define XCLK_VCO_MAX			4320
/** Minimum Versal output divider. */
#define XCLK_O_MIN			2
/** Maximum Versal output divider. */
#define XCLK_O_MAX			511

/** Maximum UltraScale VCO frequency in MHz. */
#define XCLK_US_VCO_MAX			1600
/** Minimum UltraScale VCO frequency in MHz. */
#define XCLK_US_VCO_MIN			800
/** Minimum UltraScale feedback multiplier. */
#define XCLK_US_M_MIN			2
/** Maximum UltraScale feedback multiplier. */
#define XCLK_US_M_MAX			128
/** Maximum UltraScale input divider. */
#define XCLK_US_D_MAX			106
/** Minimum UltraScale input divider. */
#define XCLK_US_D_MIN			1
/** Maximum UltraScale output divider. */
#define XCLK_US_O_MAX			128
/** Minimum UltraScale output divider. */
#define XCLK_US_O_MIN			1

/** Minimum Versal PFD frequency in MHz. */
#define XCLK_PFD_MIN			10
/** Maximum Versal PFD frequency in MHz. */
#define XCLK_PFD_MAX			500
/** Minimum UltraScale PFD frequency in MHz. */
#define XCLK_US_PFD_MIN			10
/** Maximum UltraScale PFD frequency in MHz. */
#define XCLK_US_PFD_MAX			500

/** Hertz per megahertz. */
#define XCLK_MHZ			1000000
#define XCLK_KHZ			1000	/**< Hertz per kilohertz. */
/** @} */

/*****************************************************************************/
/**
 * @brief Holds CLK_WIZ device configuration data.
 *
* The configuration structure for CLK_WIZ Controller
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
#ifndef SDT
	u32 DeviceId;	         /**< Device Id. */
#else
	char *Name;              /**< Name of the device. */
#endif
	UINTPTR BaseAddr;        /**< Base address of CLK_WIZ Controller. */
	u32 EnableClkMon;        /**< It enables the Clock Monitor. */
	u32 EnableUserClkWiz0;   /**< Enable user clk 0. */
	u32 EnableUserClkWiz1;   /**< Enable user clk 1. */
	u32 EnableUserClkWiz2;   /**< Enable user clk 2. */
	u32 EnableUserClkWiz3;   /**< Enable user clk 3. */
	double RefClkFreq;       /**< Frequency of Reference Clock. */
	double UserClkFreq0;   /**< Hold the  user clock frequency0. */
	double UserClkFreq1;   /**< Hold the  user clock frequency1. */
	double UserClkFreq2;   /**< Hold the  user clock frequency2. */
	double UserClkFreq3;   /**< Hold the  user clock frequency3. */
	double Precision;      /**< Holds the value of precision. */
	u8  EnablePll0;        /**< specify if this user clock is
				going as input to the PLL/MMCM */
	u8  EnablePll1;        /**< specify if this user clock is
				going as input to the PLL/MMCM */
#ifndef SDT
	double PrimInClkFreq;       /**< Input Clock. */
#else
	u64 PrimInClkFreq;       /**< Input Clock. */
#endif
	u32 NumClocks;		/**< Number of clocks. */
#ifdef SDT
	u32 IntId;		/**< Interrupt ID on the GIC. */
	UINTPTR IntrParent; 	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
} XClk_Wiz_Config;

/*****************************************************************************/
/**
* @brief Defines the CLK_WIZ interrupt callback type.
*
* Callback type for all interrupts defined.
*
* @param[in] CallBackRef Callback reference passed in by the upper layer when
*   setting the callback functions, and passed back to the upper layer when the
*   callback is invoked.
* @param[in] Mask Bit mask indicating the cause of the event. For current core
*   version, this parameter is "OR" of 0 or more XCLK_WIZ_ISR_*_MASK constants
*   defined in xclmon_hw.h.
*
 *****************************************************************************/
typedef void (*XClk_Wiz_CallBack) (void *CallBackRef, u32 Mask);

/**
 * @brief Holds instance data for a CLK_WIZ device.
 *
* The XClk_Wiz driver instance data.
* An instance must be allocated for each CLK_WIZ in use.
*/
typedef struct {

	XClk_Wiz_Config Config;		/**< GUI Configuration. */
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
	/** Callback for Clock stop. */
	XClk_Wiz_CallBack ClkStopCallBack;
	void *ClkStopRef;			/**< To be passed to the clock
						  *  stop call back */
	XClk_Wiz_CallBack ErrorCallBack; /**< Call back function
					   *  for rest all errors */
	void *ErrRef; /**< To be passed to the Error Call back. */
	u32 IsReady; /**< Driver is ready. */
	u32 MVal;	/**< Multiplier valuer. */
	u32 DVal;	/**< Divisor value. */
	u32  OVal;	/**< Output Value. */
	u64 MinErr;	/**< Min Error that is acceptable. */
} XClk_Wiz;

/************************** Macros Definitions *******************************/

/************************* Bit field operations ******************************/
/* Setting and resetting bits */
/*****************************************************************************/
/**
* @brief Sets a bit in a CLK_WIZ register.
*
* XCLK_WIZ_BIT_SET is used to set bit in a register.
*
* @param[in] BaseAddress Base address of IP.
*
* @param[in] RegisterOffset Offset where the register is present.
*
* @param[in] BitMask Mask of the bit to be set.
*
****************************************************************************/
static inline void XCLK_WIZ_BIT_SET(UINTPTR BaseAddress, u32 RegisterOffset, \
				    u32 BitMask)
{
	XClk_Wiz_WriteReg((BaseAddress), (RegisterOffset), \
			  (XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset)) | BitMask));
}

/****************************************************************************/
/**
* @brief Clears a bit in a CLK_WIZ register.
*
* XCLK_WIZ_BIT_RESET is used to reset bit in a register.
*
* @param[in] BaseAddress Base address of IP.
*
* @param[in] RegisterOffset Offset where the register is present.
*
* @param[in] BitMask Mask of the bit to be set.
*
****************************************************************************/
static inline void XCLK_WIZ_BIT_RESET(UINTPTR BaseAddress, u32 RegisterOffset,
				      u32 BitMask)
{
	XClk_Wiz_WriteReg((BaseAddress), (RegisterOffset), \
			  (XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset) ) & \
			   ~(BitMask)));
}

/****************************************************************************/
/**
* @brief Reads a bit field from a CLK_WIZ register.
*
* XCLK_WIZ_GET_BITFIELD_VALUE is used to get the value of bitfield from
* register.
*
* @param[in] BaseAddress Base address of IP.
*
* @param[in] RegisterOffset Offset where the register is present.
*
* @param[in] BitMask Mask of the bit to be set.
*
* @param[in] BitShift Number of bits to shift right.
*
* @return	Bit Field Value in u32 format
*
****************************************************************************/
static inline u32 XCLK_WIZ_GET_BITFIELD_VALUE(UINTPTR BaseAddress,
	u32 RegisterOffset, u32 BitMask, u32 BitShift)
{
	return ((XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset)) \
		 & (BitMask)) >> (BitShift));
}

/****************************************************************************/
/**
* @brief Writes a bit field in a CLK_WIZ register.
*
* XCLK_WIZ_SET_BITFIELD_VALUE is used to set the value of bitfield from
* register
*
* @param[in] BaseAddress Base address of IP.
*
* @param[in] RegisterOffset Offset where the register is present.
*
* @param[in] BitMask Mask of the bit to be set.
*
* @param[in] BitShift Number of bits to shift left.
*
* @param[in] Value To be set. Passed in u32 format.
*
****************************************************************************/
static inline void XCLK_WIZ_SET_BITFIELD_VALUE(UINTPTR BaseAddress, \
	u32 RegisterOffset, u32 BitMask, u32 BitShift, u32 Value)
{
	XClk_Wiz_WriteReg((BaseAddress), (RegisterOffset), \
			  ((XClk_Wiz_ReadReg((BaseAddress), (RegisterOffset)) & \
			    ~(BitMask)) | ((Value) << (BitShift))));
}

/****************************************************************************/
/**
* @brief Enables CLK_WIZ interrupts from a mask.
*
* XClk_Wiz_IntrEnable is used to set interrupt mask to enable interrupts. This
* is done before enabling the core.
*
* @param[in] InstancePtr Pointer to the CLK_WIZ Instance to be worked on.
*
* @param[in]	Mask Interrupts to be enabled.
*
****************************************************************************/
static inline void XClk_Wiz_IntrEnable(XClk_Wiz *InstancePtr, u32 Mask)
{
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, \
			  (XCLK_WIZ_IER_OFFSET), \
			  (Mask) & (XCLK_WIZ_IER_ALLINTR_MASK));
}

/****************************************************************************/
/**
* @brief Returns the CLK_WIZ interrupt-enable mask.
*
* XClk_Wiz_GetIntrEnable is used to find out which interrupts are
* registered for system
*
* @param[in] InstancePtr Pointer to the CLK_WIZ Instance to be worked on.
*
* @return	Bit Mask in u32 format
*
****************************************************************************/
static inline u32 XClk_Wiz_GetIntrEnable(XClk_Wiz *InstancePtr)
{
	return XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, \
				XCLK_WIZ_IER_OFFSET);
}

/****************************************************************************/
/**
* @brief Disables CLK_WIZ interrupts from a mask.
*
* XClk_Wiz_IntrDisable is used to disable interrupts. This is after disabling
* the core.
*
* @param[in] InstancePtr Pointer to the CLK_WIZ Instance to be worked on.
*
* @param[in]	Mask Interrupts to be disabled.
*
****************************************************************************/
static inline void XClk_Wiz_IntrDisable(XClk_Wiz *InstancePtr, u32 Mask)
{
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, \
			  (XCLK_WIZ_IER_OFFSET), \
			  ~((Mask) & (XCLK_WIZ_IER_ALLINTR_MASK)));
}

/****************************************************************************/
/**
* @brief Returns pending CLK_WIZ interrupt status.
*
* XClk_Wiz_IntrGetIrq is used to find out which events have triggered the
* interrupt
*
* @param[in] InstancePtr Pointer to the CLK_WIZ Instance to be worked on.
*
* @return	Bit Mask in u32 format.
*
****************************************************************************/
static inline u32 XClk_Wiz_IntrGetIrq(XClk_Wiz *InstancePtr)
{
	return XClk_Wiz_ReadReg((InstancePtr)->Config.BaseAddr, \
				(XCLK_WIZ_ISR_OFFSET));
}

/****************************************************************************/
/**
* @brief Acknowledges CLK_WIZ interrupt status bits.
*
* XClk_Wiz_IntrAckIrq is acknowledge the events.
*
* @param[in] InstancePtr Pointer to the CLK_WIZ Instance to be worked on.
*
* @param[in] Value Bit Mask for ack of interrupts.
*
****************************************************************************/
static inline void XClk_Wiz_IntrAckIrq(XClk_Wiz *InstancePtr, u32 Value)
{
	XClk_Wiz_WriteReg((InstancePtr)->Config.BaseAddr, \
			  (XCLK_WIZ_ISR_OFFSET), \
			  ((Value) & (XCLK_WIZ_ISR_ALLINTR_MASK)));
}

/************************** Function Prototypes ******************************/

#ifndef SDT
XClk_Wiz_Config *XClk_Wiz_LookupConfig(u32 DeviceId);
#else
XClk_Wiz_Config *XClk_Wiz_LookupConfig(UINTPTR BaseAddress);
#endif

u32 XClk_Wiz_SetRate(XClk_Wiz *InstancePtr, u64 SetRate);

u32 XClk_Wiz_SetRateHz(XClk_Wiz  *InstancePtr, u64 SetRate);

u32 XClk_Wiz_CfgInitialize(XClk_Wiz *InstancePtr, XClk_Wiz_Config *Config,
			   UINTPTR EffectiveAddr);

void XClk_Wiz_GetInterruptSettings(XClk_Wiz  *InstancePtr);

int XClk_Wiz_SetCallBack(XClk_Wiz *InstancePtr, u32 HandleType,
			 void *CallBackFunc, void *CallBackRef);

u32 XClk_Wiz_EnableClock(XClk_Wiz  *InstancePtr, u32 ClockId);

u32 XClk_Wiz_DisableClock(XClk_Wiz  *InstancePtr, u32 ClockId);

void XClk_Wiz_SetInputRate(XClk_Wiz  *InstancePtr, double Rate);

u32 XClk_Wiz_WaitForLock(XClk_Wiz  *InstancePtr);

s32 XClk_Wiz_GetRate(XClk_Wiz  *InstancePtr, u32 ClockId, u64 *Rate);

u32 XClk_Wiz_GetInputRate(XClk_Wiz  *InstancePtr);

s32 XClk_Wiz_SetLeafRateHz(XClk_Wiz  *InstancePtr, u32 ClockId, u64 SetRate);

void XClk_Wiz_SetMinErr(XClk_Wiz  *InstancePtr, u64 Minerr);

u32 XClk_Wiz_UpdateDivisors(XClk_Wiz  *InstancePtr, u32 Mvalue, u32 Dvalue, u32 Ovalue);

u32 XClk_Wiz_Reconfig(XClk_Wiz  *InstancePtr);

u32 XClk_Wiz_SetClkInSel(XClk_Wiz  *InstancePtr, u32 Clkinputnumber);

u32 XClk_Wiz_GetClkInSel(XClk_Wiz  *InstancePtr, u32 *Clkinputnumber);

u32 XClk_Wiz_GetInFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkinputnumber, u32 *RateKhz);

u32 XClk_Wiz_SetInFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkinputnumber, u32 RateKhz);

u32 XClk_Wiz_GetOutFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkoutputnumber, u32 *RateKhz);

u32 XClk_Wiz_SetOutFreqkHz(XClk_Wiz  *InstancePtr, u32 Clkoutputnumber, u32 RateKhz);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
