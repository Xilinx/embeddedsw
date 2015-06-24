/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
/**
*
* @file xscaler.h
*
* This is the main header file of the Xilinx MVI Video Scaler device driver.
* The Scaler device converts a specified rectangular area of an input digital
* video image from one original sampling grid to a desired target sampling
* grid.
*
* Video Scaler Device v3.00a features are as follows:
*
* - Target Clock Frequencies:
*	- S3ADSP(-4), S6(-2) Families: 150MHz
*	- V5(-1), V6(-1) Families: 225MHz
* - 8, 10 or 12-bit video data supported
* - YC (4:2:2), YC (4:2:0), RGB (4:4:4) chroma formats supported
* - 1080P/60 supported for 4:2:2 and 4:4:4 (RGB formats (not 4:2:0)
* - Serial or parallel options (single or multiple filter engines) available to
*   suit high or low bandwidth requirements
* - Supports spatial resolutions up to 4096x4096
* - 2-12 taps per dimension
* - Up to 16 user-loadable sets of 16-bit coefficients
* - Up to 64 phases per coefficient set
* - 16-bit intermediate bitwidth
* - Programmable (dynamic) scaling factor in both H and V dimensions
*	- Max 12x resolution change either up or down ~V allows for conversion
*	  between QCIF and 1080p
*	- 24-bit input fixed point scaling factors: 4 bits integer, 20-bit
*	  fraction
* - Independent H and V scaling factors
* - Optional coefficient sharing between Y and C filter operations (where
*   appropriate)
* - Optional coefficient sharing between H and V filter operations (where
*   appropriate)
* - Programmable (dynamic) start phase (independent H, V start-phase values),
*   range -0.99 to +0.99
* - Programmable (dynamic) subject area size
* - Programmable (dynamic) target area size
* - Coefficient set selectable during operation (eg on V-sync)
* - Coefficient range -2.0 to +1.99
* - 3 Control interface options
*	- pCore, with drivers
*	- General Purpose Processor GPP
*	- Constant
* - Coefficient preload (via .coe file) functionality for all above modes.
* - Full EDK GUI for scaler customization under XPS
* - 2 Video interface options
*	- Live video source
*	- Memory source
* - Interrupts
*
*
* For a full description of Scaler features, please see the hardware spec.
*
* An example is shipped with the driver to demonstrate how to use the APIs
* this driver provides to access and control the Video Scaler device.
*
*
* <b>Limitation</b>
*
* - Function XScaler_CalcCoeffs() only calculates coefficient values if this
*   driver is *NOT* used on Linux platform. In Linux case, the math library is
*   not available in the kernel and this function only clears the coefficient
*   buffer passed in.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a xd     05/14/09 First release
* 2.00a xd     12/14/09 Updated Doxygen document tags
* 3.00a xd     07/29/10 Added core version & sharable coefficient bank
*                       support
* 6.0   adk    12/19/13 Updated as per the New Tcl API's
* 7.0   adk    08/22/14 Removed XSCL_HANDLER_ERROR and XSCL_HANDLER_EVENT macros.
*                       Removed ErrorCallBack and its ref ErrorRef from XScaler
*                       structure.
*                       Modified EventCallBack as CallBack and
*                       EventRef as CallBackRef.
*                       Modified XSCL_STSDONE to XSCL_STATUS_OFFSET,
*                       XSCL_STS to XSCL_STATUS_OFFSET, XSCL_STSERR to
*                       XSCL_ERROR_OFFSET.
*                       Removed the following functional macros
*                       XScaler_IntrGetPending,
*                       XScaler_IntrEnableGlobal and XScaler_IntrDisableGlobal.
*                       uncommented interrupt related macros.
*                       Modified prototypes of the following functions by removing IntrType
*                       parameter as there was only one interrupt :XScaler_IntrEnable,
*                       XScaler_IntrDisable and XScaler_IntrClear.
*
*                       Modifications from xscalar_hw.h file are:
*                       Appended register offset macros with _OFFSET and
*                       Bit definition with _MASK.
*                       Provided backward compatibility for changed macros.
*                       Defined the following macros XSCL_CTL_MEMRD_EN_MASK.
*                       Modified XSCL_CTL_ENABLE to XSCL_CTL_SW_EN_MASK,
*                       XSCL_RESET_RESET_MASK to XSCL_CTL_RESET_MASK,
*                       XSCL_CTL_REGUPDATE to XSCL_CTL_RUE_MASK,
*                       XSCL_STSDONE_DONE and XSCL_STS_COEF_W_RDY_MASK to
*                       XSCL_IXR_COEF_W_RDY_MASK.
*                       Added XSCL_ERR_*_MASK s.
*                       Removed XSCL_GIER_GIE_MASK.
*                       Removed following macros as they were not defined in
*                       latest product guide(v 8.1):
*                       XSCL_STSERR_CODE*_MASK, XSCL_IXR_OUTPUT_FRAME_DONE_MASK,
*                       XSCL_IXR_COEF_FIFO_READY_MASK, XSCL_IXR_INPUT_ERROR_MASK
*                       XSCL_IXR_COEF_WR_ERROR_MASK,
*                       XSCL_IXR_REG_UPDATE_DONE_MASK,
*                       XSCL_IXR_OUTPUT_ERROR_MASK, XSCL_IXR_EVENT_MASK,
*                       XSCL_IXR_ERROR_MASK, XSCL_IXR_ALLINTR_MASK,
*                       XSCL_HSF_INT_MASK, XSCL_VSF_INT_MASK,
*                       XSCL_COEFFVALUE_BASE_SHIFT and XSCL_COEFVALUE_BASE_MASK.
*                       Modified bits of the following macros:
*                       XSCL_HSF_FRAC_MASK and XSCL_VSF_FRAC_MASK.
*
*                       Modifications from xscalar.c file are:
*                       Modified prototype of XScaler_GetVersion API.
*                       and functionality of StubCallBack. Modified assert
*                       conditions in functions XScaler_CfgInitialize,
*                       XScaler_SetPhaseNum, XScaler_LoadCoeffBank.
*                       Removed error callback from XScaler_CfgInitialize
*                       function.
*                       Uncommented XScaler_Reset in XScaler_CfgInitialize
*                       function.
*
*                       Modifications from file xscalar_coefs.c file are
*                       Removed typedef unsigned short s16 as it was already
*                       defined in xil_types.h.
*                       Modified coefs_struct to Coefs_Struct.
*                       Updated doxygen document tags.
*                       XScaler_coef_table is made as a global variable.
*                       Memory allocated was freed after usage.
*
*                       Modifications from xscalar_intr.c file are
*                       XScaler_IntrHandler and XScaler_SetCallBack APIs were
*                       modified
*
*                       Added XScaler_LookupConfig in xscalar_sinit.c file and
*
* </pre>
*
******************************************************************************/

#ifndef XSCALER_H		/* prevent circular inclusions */
#define XSCALER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "xil_assert.h"
#include "xscaler_hw.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/


/** @name Minimum and Maximum Tap Numbers
 *	@{
 */
#define XSCL_MIN_TAP_NUM	2	/**< Minimum Tap Number */
#define XSCL_MAX_TAP_NUM	12	/**< Maximum Tap Number */
/*@}*/

/** @name Minimum and Maximum Phase Numbers
 *	@{
 */
#define XSCL_MIN_PHASE_NUM	2	/**< Minimum Phase Number */
#define XSCL_MAX_PHASE_NUM	64	/**< Maximum Phase Number */

/*@}*/

/** @name Coefficient Precision
 *	@{
 */
#define XSCL_COEFF_PRECISION	16	/**< Coefficient Precision */
/*@}*/

/** @name Maximum Coefficient Set Number
 *	@{
 */
#define XSCL_MAX_COEFF_SET_NUM	16	/**< Maximum Coefficient Set Number */
/*@}*/

/** @name The number of coefficient Bins
 *	@{
 */
#define XSCL_NUM_COEF_BINS	19	/**< The number of coefficient Bins */
/*@}*/

/** @name The size of a coefficient Bin in 16-bit integers.
 *	@{
 */
#define XSCL_COEF_BIN_SIZE XScaler_CoefTapOffset(XSCL_MAX_TAP_NUM + 1)
/*@}*/

/** @name Shrink factor constants
 *	@{
 */
#define XSCL_SHRINK_FACTOR	0x100000 /**< For calculating HSF/VSF values */
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for a Scaler device.
 * Each Scaler device should have a configuration structure associated
 */
typedef struct {
	u16 DeviceId;	   /**< DeviceId is the unique ID  of the device */
	u32 BaseAddress;   /**< BaseAddress is the physical base address of the
			     *  device's registers */
	u16 VertTapNum;	   /**< The number of Vertical Taps */
	u16 HoriTapNum;	   /**< The number of Horizontal Taps */
	u16 MaxPhaseNum;   /**< The maximum number of Phases */
	u8 CoeffSetNum;	   /**< The number of coefficient sets implemented */
	u8 ChromaFormat;   /**< Chroma Format */
	u8 SeparateYcCoef; /**< Separate Chroma/Luma Coefficient banks */
	u8 SeparateHvCoef; /**< Separate Horizontal/Vertical Coefficient
				banks. Common only possible if num_h_taps =
				num_v_taps */

} XScaler_Config;

/**
 * Callback type for interrupts
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the
 *	  upper layer when the callback is invoked.
 * @param EventMask indicates which events are happening. They could be
 *	  either normal events or errors. The value is created by "OR'ing"
 *	  XSCL_IXR_* constants defined in xscaler_hw.h
 */
typedef void (*XScaler_CallBack) (void *CallBackRef);

/**
 * The XScaler driver instance data. An instance must be allocated for each
 * Scaler device in use.
 */
typedef struct {
	XScaler_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance are
					     initialized */

	XScaler_CallBack CallBack;	/**< Call back for
					     interrupt */
	void *CallBackRef;			/**< To be passed to the
					     interrupt callback */
} XScaler;

/**
 * The XScalerAperture data structure for Aperture and scale factor control.
 * The scale factor values are calculated using the field in this structure
 */
typedef struct {
	u32 InFirstLine;	/**< The first line index in the input video */
	u32 InLastLine;		/**< The last line index in the input video */
	u32 InFirstPixel;	/**< The first pixel index in the input video*/
	u32 InLastPixel;	/**< The last pixel index in the input video */
	u32 OutVertSize;	/**< Vertical size of the output video */
	u32 OutHoriSize;	/**< Horizontal size of the output video */
	u32 SrcVertSize;	/**< Vertical size of the source video */
	u32 SrcHoriSize;	/**< Horizontal size of the source video */
} XScalerAperture;

/**
 * The XScalerCoeffBank data structure for loading a Bank in a Coefficient Set,
 * which contains 4 banks.
 */
typedef struct {
	u16 SetIndex;		/**< Coefficient Set Index (0 based). */
	s16 *CoeffValueBuf;	/**< Pointer to a coefficient value data buffer
				  */
	u16 PhaseNum;		/**< The number of the phases associated w/
				  *  the bank */
	u16 TapNum;		/**< The number of the Tap associated w/ the
				  *  bank */
} XScalerCoeffBank;

/**
 * The XScalerStartFraction data structure for Luma and Chroma Start Fraction
 * setting
 */
typedef struct {
	s32 LumaLeftHori;  /**< Horizontal accumulator at rectangle left edge
				for Luma */
	s32 LumaTopVert;   /**< Vertical accumulator at rectangle top edge for
				Luma */
	s32 ChromaLeftHori;/**< Horizontal accumulator at rectangle left edge
				for Chroma */
	s32 ChromaTopVert; /**< Vertical accumulator at rectangle top edge for
				Chroma */
} XScalerStartFraction;

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Macros for operating a Scaler device
 *	@{
 */

/*****************************************************************************/
/**
*
* This macro enables a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be worked
*		on.
*
* @return None.
*
* @note		C-style signature:
*		void XScaler_Enable(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_Enable(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_CTL_OFFSET), \
			((XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XSCL_CTL_OFFSET))) | (XSCL_CTL_SW_EN_MASK)))


/*****************************************************************************/
/**
*
* This macro disables a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*	 	void XScaler_Disable(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_Disable(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_CTL_OFFSET), \
			((XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XSCL_CTL_OFFSET)) & (~(XSCL_CTL_SW_EN_MASK)))))

/*****************************************************************************/
/**
*
* This macro checks if a Scaler device is enabled.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	- TRUE if the Scaler device is enabled.
*		- FALSE otherwise.
*
* @note		C-style signature:
*		boolean XScaler_IsEnabled(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_IsEnabled(InstancePtr) \
	XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_CTL_OFFSET)) & (XSCL_CTL_SW_EN_MASK) ? TRUE : FALSE

/*****************************************************************************/
/**
*
* This macro checks if a Scaler operation is finished
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return
*		- TRUE if the Scaler operation is finished.
*		- FALSE otherwise.
*
* @note		C-style signature:
* 		boolean XScaler_CheckDone(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_CheckDone(InstancePtr) \
	XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
					(XSCL_STATUS_OFFSET)) & \
		(XSCL_STS_COEF_W_RDY_MASK) ? TRUE : FALSE

/*****************************************************************************/
/**
*
* This macro tells a Scaler device to pick up the register value changes made
* so far.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return None.
*
* @note 	C-style signature:
*		void XScaler_EnableRegUpdate(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_EnableRegUpdate(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_CTL_OFFSET), \
			((XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XSCL_CTL_OFFSET))) | (XSCL_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This macro tells a Scaler device not to pick up the register value changes
* until XScaler_EnableRegUpdate() is invoked again. This is very useful when
* multiple registers need to be updated. All register updates could be made
* with no tight time constraints with the help of this macro.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XScaler_DisableRegUpdate(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_DisableRegUpdate(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_CTL_OFFSET), \
			((XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XSCL_CTL_OFFSET))) & (~(XSCL_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This macro checks if a Scaler device is ready to accept the coefficients
* the software is going to load.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return
*		- TRUE if the Scaler device is ready for the coefficient load.
*		- FALSE otherwise.
*
* @note		C-style signature:
*		boolean XScaler_CoeffLoadReady(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_CoeffLoadReady(InstancePtr) \
	XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
					(XSCL_STATUS_OFFSET)) & \
		(XSCL_STS_COEF_W_RDY_MASK) ? TRUE : FALSE

/*****************************************************************************/
/**
*
* This macro checks the error status of a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	The error type, if any. Use XSCL_STSERR_* defined in
*		xscaler_hw.h to interpret the value.
*
* @note		C-style signature:
*		u32 XScaler_GetError(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_GetError(InstancePtr) \
	XScaler_ReadReg((InstancePtr)->Config.BaseAddress, (XSCL_ERROR_OFFSET))

/*****************************************************************************/
/**
*
* This macro resets a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XScaler_Reset(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_Reset(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_CTL_OFFSET), (XSCL_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This macro checks if the reset on a Scaler device is done.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return
*		- TRUE if the reset is done;
*		- FALSE otherwise.
*
* @note		C-style signature:
*		boolean XScaler_IsResetDone(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_IsResetDone(InstancePtr) \
	XScaler_ReadReg((InstancePtr)->Config.BaseAddress, \
					(XSCL_CTL_OFFSET)) & \
					(XSCL_CTL_RESET_MASK) ? FALSE : TRUE

/*****************************************************************************/
/**
 * This macro calculates the N-th Triangular number: 1 + 2 + ... + N
 *
 * @param	N indicates the positive integer number to calculate the N-th
 *		Triangular number.
 *
 * @return	The N-th triangular number.
 *
 * @note	C-style signature:
 *		u32 XScaler_TriangularNumber(u32 N);
 *
 *****************************************************************************/
#define XScaler_TriangularNumber(N) ((N) * ((N) + 1) / 2)

/*****************************************************************************/
/**
 * This macro calculates the offset of a coefficient Tap from the beginning of
 * a coefficient Bin.
 *
 * @param	Tap indicates the index of the coefficient tap in the
 *		coefficient Bin.
 *
 * @return	The offset of the coefficient TAP from the beginning of a
 *		coefficient Bin.
 * @note	C-style signature:
 *		u32 XScaler_CoefTapOffset(u32 Tap);
 *
 *****************************************************************************/
#define XScaler_CoefTapOffset(Tap) \
	((XScaler_TriangularNumber((Tap) - 1) - 1) * \
		(XScaler_TriangularNumber(16) - 1 + 32 + 64))

/*****************************************************************************/
/**
 * This macro calculates the offset of the first coefficient Phase from the
 * beginning of a coefficient Tap given the currently used Phase and Tap
 * numbers for scaling operation.
 *
 * @param	Tap indicates the number of Taps used for the scaling operation.
 * @param	Phase indicates the number of Phases used for the scaling
 *		operation.
 *
 * @return	The offset of the first coefficient Phase from the beginning of
 *		a coefficient Tap.
 * @note	C-style signature:
 *		u32 XScaler_CoefPhaseOffset(u32 Tap, u32 Phase);
 *
 *****************************************************************************/
#define XScaler_CoefPhaseOffset(Tap, Phase) \
	(((Phase) < 32) ? \
		(Tap) * (XScaler_TriangularNumber((Phase) - 1) - 1) : \
		((Phase) == 32) ? \
			(Tap) * (XScaler_TriangularNumber(16) - 1) : \
			(Tap) * (XScaler_TriangularNumber(16) - 1 + 32))


/*****************************************************************************/
/**
*
* This macro enables the Coef_FIFO_Ready interrupt on a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XScaler_IntrEnable(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_IntrEnable(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_IRQ_EN_OFFSET),(XSCL_IXR_COEF_W_RDY_MASK)) \

/*****************************************************************************/
/**
*
* This macro disables the Coef_FIFO_Ready interrupt on a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XScaler_IntrDisable(XScaler *InstancePtr);
*
******************************************************************************/
#define XScaler_IntrDisable(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_IRQ_EN_OFFSET), 0)

/*****************************************************************************/
/**
*
* This macro clears/acknowledges Coef_FIFO_Ready interrupt of a Scaler device.
*
* @param	InstancePtr is a pointer to the Scaler device instance to be
*		worked on.
*
* @return	None
*
* @note		C-style signature:
*		void XScaler_IntrClear(XScaler *InstancePtr)
*
******************************************************************************/
#define XScaler_IntrClear(InstancePtr) \
	XScaler_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XSCL_STATUS_OFFSET), (XSCL_IXR_COEF_W_RDY_MASK))

/*@}*/

/************************** Function Prototypes ******************************/

/*
 * Initialization and control functions in xscaler.c
 */

/* Initialization */
int XScaler_CfgInitialize(XScaler *InstancePtr, XScaler_Config *CfgPtr,
				u32 EffectiveAddr);

/* Aperture & Scale */
int  XScaler_SetAperture(XScaler *InstancePtr, XScalerAperture *AperturePtr);
void XScaler_GetAperture(XScaler *InstancePtr, XScalerAperture *AperturePtr);

/* Phase */
void XScaler_SetPhaseNum(XScaler *InstancePtr, u16 VertPhaseNum,
				u16 HoriPhaseNum);
void XScaler_GetPhaseNum(XScaler *InstancePtr, u16 *VertPhaseNumPtr,
				u16 *HoriPhaseNumPtr);

/* Start Fractional value setting */
void XScaler_SetStartFraction(XScaler *InstancePtr,
				  XScalerStartFraction *StartFractionPtr);
void XScaler_GetStartFraction(XScaler *InstancePtr,
				  XScalerStartFraction *StartFractionPtr);

/* Coefficient functions */
s16 *XScaler_CoefValueLookup(u32 InSize, u32 OutSize, u32 Tap, u32 Phase);
void XScaler_LoadCoeffBank(XScaler *InstancePtr,
				XScalerCoeffBank *CoeffBankPtr);
void XScaler_SetActiveCoeffSet(XScaler *InstancePtr,
				   u8 VertSetIndex,
				   u8 HoriSetIndex);
void XScaler_GetActiveCoeffSet(XScaler *InstancePtr,
				   u8 *VertSetIndexPtr,
				   u8 *HoriSetIndexPtr);
void XScaler_GetCoeffBankSharingInfo(XScaler *InstancePtr,
					u8 *ChromaFormat,
					u8 *ChromaLumaShareCoeff,
					u8 *HoriVertShareCoeff);
u32 XScaler_GetVersion(XScaler *InstancePtr);

/*
 * Initialization functions in xscaler_sinit.c
 */
XScaler_Config *XScaler_LookupConfig(u16 DeviceId);

/*
 * Interrupt related functions in xscaler_intr.c
 */
void XScaler_IntrHandler(void *InstancePtr);
void XScaler_SetCallBack(XScaler *InstancePtr,
				void *CallBackFunc, void *CallBackRef);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
