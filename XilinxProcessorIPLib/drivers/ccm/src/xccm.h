/******************************************************************************
*
* (c) Copyright 2011 - 2014 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xccm.h
* @addtogroup ccm_v6_1
* @{
* @details
* 
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx Color Correction Matrix (CCM) core instance.
*
* The Color Correction Matrix core offers a 3x3 matrix multiplication for a
* variety of color correction applications.
* CCM core provides following features:
*	- Fully programmable coefficient matrix.
*	- Offset compensation.
*	- Clipping and
*	- Clamping of the output.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the CCM core.
*
* XCcm_CfgInitialize() API is used to initialize the CCM core.
* The user needs to first call the XCcm_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XCcm_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XCcm_IntrHandler for handling
* the interrupt from the CCM core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XCcm_SetCallBack API.
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
* The XCcm driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 2.00a jo      05/1/10  Updated for CCM V2.0.
* 3.00a ren     09/11/11 Updated for CCM V3.0.
* 4.00a jj      12/18/12 Converted from xio.h to xil_io.h,translating
*                        basic types,MB cache functions, exceptions
*                        and assertions to xil_io format.
* 5.0   adk     19/12/13 Updated as per the New Tcl API's.
* 6.0   adk     03/06/14 Changed file name ccm.h to xccm.h.
*                        Moved register offsets and bit definitions to
*                        xccm_hw.h file.
*                        Defined the following handler types as enum values:
*                        XCCM_HANDLER_PROCSTART ,XCCM_HANDLER_FRAMEDONE and
*                        XCCM_HANDLER_ERROR.
*
*                        Added the following range macros:
*                        XCCM_VSIZE_FIRST, XCCM_VSIZE_LAST
*                        XCCM_HSIZE_FIRST, XCCM_HSIZE_LAST
*                        XCCM_COEF_FIRST,XCCM_COEF_LAST
*                        XCCM_OFFSET_FIRST,XCCM_OFFSET_LAST
*                        XCCM_CLAMP_FIRST,XCCM_CLAMP_LAST
*                        XCCM_CLIP_FIRST,XCCM_CLIP_LAST.
*
*                        Added the following structure type definitions:
*                        XCcm_Config and XCcm.
*
*                        Removed the following functional macros:
*                        CCM_Enable, CCM_Disable, CCM_RegUpdateEnable
*                        CCM_RegUpdateDisable, CCM_Reset, CCM_AutoSyncReset
*                        CCM_ClearReset.
*
*                        Added the following macros:
*                        XCcm_Enable, XCcm_Disable,XCcm_RegUpdateEnable,
*                        XCcm_SyncReset, XCcm_Reset, XCcm_IntrGetPending,
*                        XCcm_IntrEnable, XCcm_IntrDisable,
*                        XCcm_StatusGetPending, XCcm_IntrClear, XCcm_Start,
*                        XCcm_Stop.
*
*                        Modification history from xccm_hw.h
*                        First release.
*                        Added the register offsets and bit masks for the
*                        registers.
*                        Added backward compatibility macros.
*
*                        Modifications in the file xccm.c are:
*                        Changed filename ccm to xccm.c.
*                        Implemented the following functions:
*                        XCcm_CfgInitialize, XCcm_Setup, XCcm_GetVersion,
*                        XCcm_EnableDbgByPass, XCcm_IsDbgByPassEnabled,
*                        XCcm_DisableDbgByPass, XCcm_EnableDbgTestPattern,
*                        XCcm_IsDbgTestPatternEnabled,
*                        XCcm_DisableDbgTestPattern, XCcm_GetDbgFrameCount,
*                        XCcm_GetDbgLineCount, XCcm_GetDbgPixelCount,
*                        XCcm_SetActiveSize, XCcm_GetActiveSize,
*                        XCcm_SetCoefMatrix, XCcm_GetCoefMatrix,
*                        XCcm_SetRgbOffset, XCcm_GetRgbOffset,
*                        XCcm_SetClip, XCcm_GetClip,
*                        XCcm_SetClamp XCcm_GetClamp XCcm_FloatToFixedConv,
*                        and XCcm_FixedToFloatConv.
*
*                        Modifications in the file xccm_selftest.c are:
*                        Implemented XCcm_SelfTest function.
*
*                        Modifications in the file xccm_sinit.c are:
*                        Implemented XCcm_LookupConfig function.
*
*                        Modifications in the file xccm_intr.c are:
*                        Implemented the following functions:
*                        XCcm_IntrHandler
*                        XCcm_SetCallBack
* 6.1   ms     01/16/17  Updated the parameter naming from
*                        XPAR_CCM_NUM_INSTANCES to XPAR_XCCM_NUM_INSTANCES
*                        to avoid  compilation failure for
*                        XPAR_CCM_NUM_INSTANCES as the tools are generating
*                        XPAR_XCCM_NUM_INSTANCES in the generated xccm_g.c
*                        for fixing MISRA-C files. This is a fix for
*                        CR-966099 based on the update in the tools.
*       ms     03/17/17  Added readme.txt file in examples folder for doxygen
*                        generation.
*
* </pre>
*
******************************************************************************/

#ifndef XCCM_H_
#define XCCM_H_		  /**< Prevent circular inclusions
			    *  by using protection macros */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xccm_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
 * @{
 */
/**
 * These constants specify different types of handler and used to differentiate
 * interrupt requests from core.
 *
 */
enum {
	XCCM_HANDLER_PROCSTART = 1, /**< A processing start event interrupt
				      *  type */
	XCCM_HANDLER_FRAMEDONE,	   /**< A frame done event interrupt type */
	XCCM_HANDLER_ERROR	   /**< An error condition interrupt type */
} ;
/*@}*/

/** @name Active Size Ranges
 * @{
 */
#define XCCM_ACT_SIZE_FIRST	32	/**< Active Size starting value */
#define XCCM_ACT_SIZE_LAST		7680	/**< Active Size ending value */

/*@}*/

/** @name Coefficient ranges
 * @{
 */
#define XCCM_COEF_FIRST		-8.0	/**< Coefficient starting
					  *  value */
#define XCCM_COEF_LAST		 8.0	/**< Coefficient ending
					  *  value */
/*@}*/

/** @name Offset ranges
 * @{
 */
#define XCCM_OFFSET_FIRST	0xFFFFFF00	/**< Offset starting value */
#define XCCM_OFFSET_LAST	255		/**< Offset ending value */
/*@}*/

/** @name Clamp ranges
 * @{
 */
#define  XCCM_CLAMP_FIRST	0	/**< Clamp starting value */
#define  XCCM_CLAMP_LAST	255	/**< Clamp ending value */
/*@}*/

/** @name Clip ranges
 * @{
 */
#define  XCCM_CLIP_FIRST	0	/**< Clip starting value */
#define  XCCM_CLIP_LAST		255	/**< Clip ending value */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_Enable(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_Enable(InstancePtr) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCCM_CONTROL_OFFSET), \
		((XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCCM_CONTROL_OFFSET))) | (XCCM_CTL_SW_EN_MASK)))

/*****************************************************************************/
/**
*
* This macro disables the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_Disable(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_Disable(InstancePtr) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCCM_CONTROL_OFFSET), \
		((XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCCM_CONTROL_OFFSET))) & (~(XCCM_CTL_SW_EN_MASK))))

/*****************************************************************************/
/**
*
* This macro enables/starts the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be
*		worked on
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_Start(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_Start	XCcm_Enable

/*****************************************************************************/
/**
*
* This macro disables/stops the CCM core.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_Stop(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_Stop 	XCcm_Disable

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far by
* the software to the CCM core instance.
*
* This macro only works when the CCM core is enabled.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
* 		void XCcm_RegUpdateEnable(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_RegUpdateEnable(InstancePtr) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XCCM_CONTROL_OFFSET), \
		((XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCCM_CONTROL_OFFSET))) | (XCCM_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This macro prevents the CCM core instance from committing recent
* changes made so far by the software. When disabled, changes to other
* configuration registers are stored, but do not effect the behavior of the
* core.
*
* This macro only works when the CCM core is enabled.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_RegUpdateDisable(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_RegUpdateDisable(InstancePtr) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XCCM_CONTROL_OFFSET), \
		((XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCCM_CONTROL_OFFSET))) & (~(XCCM_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This macro resets a CCM core at the end of the frame being
* processed. It enables core automatically synchronizes to the SOF of the core
* to prevent image tearing. This macro is differ from XCcm_Reset().
*
* On the next rising-edge of SOF following a call to XCcm_SyncReset,
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
* 		void XCcm_SyncReset(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_SyncReset(InstancePtr) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCCM_CONTROL_OFFSET), \
		(XCCM_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This macro resets a CCM core. This reset effects the core
* immediately and may cause image tearing.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
* 		void XCcm_Reset(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_Reset(InstancePtr) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XCCM_CONTROL_OFFSET), \
				(XCCM_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This macro enables individual interrupts of the CCM core by updating
* the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
* @param	IntrType is the type of the interrupts to enable. Use OR'ing of
*		XCCM_IXR_*_MASK constants defined in xccm_hw.h to create this
*		parameter value.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
* 		C-style signature:
*		void XCcm_IntrEnable(XCcm *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XCcm_IntrEnable(InstancePtr, IntrType) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCCM_IRQ_EN_OFFSET), \
		(((IntrType) & (XCCM_IXR_ALLINTR_MASK)) | \
			(XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XCCM_IRQ_EN_OFFSET)))))

/*****************************************************************************/
/**
*
* This macro disables individual interrupts of the CCM core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
* @param	IntrType is the type of the interrupts to disable. Use OR'ing
*		of XCCM_IXR_*_MASK constants defined in xccm_hw.h to create
*		this parameter value.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType,
* 		if enabled before this macro is called, will remain enabled.
* 		C-style signature:
* 		void XCcm_IntrDisable(XCcm *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XCcm_IntrDisable(InstancePtr, IntrType) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCCM_IRQ_EN_OFFSET), \
		((XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCCM_IRQ_EN_OFFSET))) & ((~(IntrType)) & \
				(XCCM_IXR_ALLINTR_MASK))))

/*****************************************************************************/
/**
*
* This macro returns the pending interrupt status of the CCM core
* by reading from Status register.
*
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	The pending interrupts of the CCM. Use XCCM_IXR_*_MASK
*		constants defined in xccm_hw.h to interpret this value.
*
* @note		C-style signature:
*		u32 XCcm_StatusGetPending(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_StatusGetPending(InstancePtr) \
	XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XCCM_STATUS_OFFSET)) & (XCCM_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of the CCM core for the
* interrupts that have been enabled.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
*
* @return	The pending interrupts of the CCM. Use XCCM_IXR_*_MASK
*		constants defined in xccm_hw.h to interpret this value.
*		The returned value is a logical AND of the contents of the
*		STATUS Register and the IRQ_ENABLE Register.
*
* @note		C-style signature:
*		u32 XCcm_IntrGetPending(XCcm *InstancePtr)
*
******************************************************************************/
#define XCcm_IntrGetPending(InstancePtr) \
	XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
					(XCCM_IRQ_EN_OFFSET)) & \
		((XCcm_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCCM_STATUS_OFFSET))) & (XCCM_IXR_ALLINTR_MASK))

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of the CCM core
* in the Status register. Bit positions to 1 will be cleared.
*
* @param	InstancePtr is a pointer to the XCcm instance to be worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
* 		Use OR'ing of XCCM_IXR_*_MASK constants defined in xccm_hw.h to
* 		create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_IntrClear(XCcm *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XCcm_IntrClear(InstancePtr, IntrType) \
	XCcm_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XCCM_STATUS_OFFSET), ((IntrType) & (XCCM_IXR_ALLINTR_MASK)))

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a CCM core.
* Each CCM core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID  of the
				  *  device */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the device's registers */
	u32 SlaveAxisVideoFormat;	/**< Slave Axis Video Format */
	u32 MasterAxisVideoFormat;	/**< Master Axis Video Format */
	u32 MaxColumns;			/**< Maximum Columns */
	u32 ActiveColumns;		/**< Active Columns */
	u32 ActiveRows;			/**< Active rows */
	u16 HasDebug;			/**< Has Debug GUI specified */
	u16 HasIntcIf;			/**< Has Interrupt Control */
	u16 Clip;			/**< Clip value */
	u16 Clamp;			/**< Clamp value */
	u32 K11;			/**< Element of Coefficient matrix */
	u32 K12;			/**< Element of Coefficient matrix */
	u32 K13;			/**< Element of Coefficient matrix */
	u32 K21;			/**< Element of Coefficient matrix */
	u32 K22;			/**< Element of Coefficient matrix */
	u32 K23;			/**< Element of Coefficient matrix */
	u32 K31;			/**< Element of Coefficient matrix */
	u32 K32;			/**< Element of Coefficient matrix */
	u32 K33;			/**< Element of Coefficient matrix */
	u32 ROffset;			/**< Red color offset value */
	u32 GOffset;			/**< Green color offset value */
	u32 BOffset;			/**< Blue color offset value */
	u32 SlaveAxiClkFreqHz;		/**< Slave Clock Frequency in Hz */
} XCcm_Config;

/******************************************************************************/
/**
*
* Callback type for all interrupts except error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*/
typedef void (*XCcm_CallBack)(void *CallBackRef);

/******************************************************************************/
/**
*
* Callback type for Error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
* 		layer when setting the callback functions, and passed back to
* 		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XCCM_IXR_*_MASK values defined
*		in xccm_hw.h.
*/
typedef void (*XCcm_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/******************************************************************************/
/**
*
* The XCcm driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XCcm_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
					  *  are initialized */
	u16 HSize;			/**< Active Video Horizontal Size */
	u16 VSize;			/**< Active Video Vertical Size */

	/* IRQ Callbacks */
 	XCcm_CallBack ProcStartCallBack;/**< Call back for Processing
					  *  Start interrupt */
	void *ProcStartRef;		/**< To be passed to the Process Start
					  * interrupt callback */
	XCcm_CallBack FrameDoneCallBack;/**< Call back for Frame Done
					  * interrupt */
	void *FrameDoneRef;		/**< To be passed to the Frame Done
					  *  interrupt callback */
	XCcm_ErrorCallBack ErrCallBack; /**< Call back for Error interrupt  */
	void *ErrRef;			/**< To be passed to the Error
					  *  interrupt callback */
} XCcm;

/**
* This typedef contains matrix coefficients of Color Correction Matrix(CCM)
* core which can be accessed for setting coefficients by using XCcm_SetCoefMatrix
* and for getting coefficient values by using XCcm_GetCoefMatrix.
*/
typedef struct {
	float K11;	/**< Matrix Coefficient */
	float K12;	/**< Matrix Coefficient */
	float K13;	/**< Matrix Coefficient */
	float K21;	/**< Matrix Coefficient */
	float K22;	/**< Matrix Coefficient */
	float K23;	/**< Matrix Coefficient */
	float K31;	/**< Matrix Coefficient */
	float K32;	/**< Matrix Coefficient */
	float K33;	/**< Matrix Coefficient */
} XCcm_Coefs;

/************************** Function Prototypes ******************************/

 /* Initialization functions in xccm_sinit.c */
XCcm_Config *XCcm_LookupConfig(u16 DeviceId);

 /* Initialization and control functions implemented in xccm.c */
int XCcm_CfgInitialize(XCcm *InstancePtr, XCcm_Config *CfgPtr,
						u32 EffectiveAddr);
u32 XCcm_GetVersion(XCcm *InstancePtr);
void XCcm_Setup(XCcm *InstancePtr);
void XCcm_EnableDbgByPass(XCcm *InstancePtr);
int XCcm_IsDbgByPassEnabled(XCcm *InstancePtr);
void XCcm_DisableDbgByPass(XCcm *InstancePtr);
void XCcm_EnableDbgTestPattern(XCcm *InstancePtr);
int XCcm_IsDbgTestPatternEnabled(XCcm *InstancePtr);
void XCcm_DisableDbgTestPattern(XCcm *InstancePtr);
u32 XCcm_GetDbgFrameCount(XCcm *InstancePtr);
u32 XCcm_GetDbgLineCount(XCcm *InstancePtr);
u32 XCcm_GetDbgPixelCount(XCcm *InstancePtr);
void XCcm_SetActiveSize(XCcm *InstancePtr, u16 HSize, u16 VSize);
void XCcm_GetActiveSize(XCcm *InstancePtr, u16 *HSize, u16 *VSize);
void XCcm_SetCoefMatrix(XCcm *InstancePtr, XCcm_Coefs *CoefValues);
void XCcm_GetCoefMatrix(XCcm *InstancePtr, XCcm_Coefs *CoefValues);
void XCcm_SetRgbOffset(XCcm *InstancePtr, s32 ROffset, s32 GOffset,
			s32 BOffset);
void XCcm_GetRgbOffset(XCcm *InstancePtr, s32 *ROffset, s32 *GOffset,
			s32 *BOffset);
void XCcm_SetClip(XCcm *InstancePtr, u32 Clip);
u32 XCcm_GetClip(XCcm *InstancePtr);
void XCcm_SetClamp(XCcm *InstancePtr, u32 Clamp);
u32 XCcm_GetClamp(XCcm *InstancePtr);

/* Self - Test function implemented in xccm_selftest.c */
int XCcm_SelfTest(XCcm *InstancePtr);

/* Interrupt related functions implemented in xccm_intr.c */
void XCcm_IntrHandler(void *InstancePtr);
int XCcm_SetCallBack(XCcm *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
