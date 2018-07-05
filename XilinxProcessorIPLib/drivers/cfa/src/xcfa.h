/******************************************************************************
*
* (c) Copyright 2001-14 Xilinx, Inc. All rights reserved.
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
* @file xcfa.h
* @addtogroup cfa_v7_1
* @{
* @details
*
* This header file contains identifiers and register-level core functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx Color Filter Array Interpolation (CFA) core.
*
* The Color Filter Array Interpolation core reconstructs a color image from an
* RGB or CMY Bayer filtered sensor using a 5x5 interpolation aperture. The core
* is capable of a maximum resolution of 7680 columns by 7680 rows with 8, 10,
* or 12 bits per pixel and supports the bandwidth necessary for High-definition
* (1080p60) resolutions in all Xilinx FPGA device families.
* Higher resolutions can be supported in Xilinx high-performance
* device families.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the CFA core.
*
* XCfa_CfgInitialize() API is used to initialize the CFA core.
* The user needs to first call the XCfa_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XCfa_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XCfa_IntrHandler for handling
* the interrupt from the CFA core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XCfa_SetCallBack API.
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
* The XCfa driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00a xd     08/05/08 First release
* 3.00a gz     10/22/10 Updated for CFA V3.0
* 4.00a rc     09/11/11 Updated for CFA v4.0
* 5.00a se     12/01/11 Updated for CFA v5.0, replaced xio.h with xil_io.h.
* 6.0   adk    19/12/13 Updated as per the New Tcl API's.
* 7.0   adk    01/07/14 Changed the file name from cfa.h to xcfa.h
*                       Defined following handler types as enum
*                       XCFA_HANDLER_PROCSTART, XCFA_HANDLER_FRAMEDONE,
*                       XCFA_HANDLER_ERROR.
*
*                       Defined the BayerPhaseCombination enum
*                       for bayerphase combinations.
*
*                       Defined the following macros:
*                       XCFA_VSIZE_FIRST, XCFA_VSIZE_LAST
*                       XCFA_HSIZE_FIRST, XCFA_HSIZE_LAST
*                       XCfa_Start, XCfa_Stop, XCfa_IntrEnable,
*                       XCfa_IntrDisable, XCfa_StatusGetPending,
*                       XCfa_IntrGetPending, XCfa_IntrClear.
*
*                       Added the following function macros:
*                       XCfa_Enable, XCfa_Disable, XCfa_Reset, XCfa_SyncReset,
*                       XCfa_RegUpdateEnable, XCfa_RegUpdateDisable.
*
*                       Removed the following functional macros:
*                       CFA_Enable, CFA_Disable, CFA_Reset, CFA_FSyncReset,
*                       XCFA_RegUpdateEnable, XCFA_RegUpdateDisable,
*                       CFA_ClearReset, CFA_ClearStatus.
*
*                       Defined the following type definitions:
*                       XCfa_Config and XCfa structures.
*                       XCfa_CallBack and XCfa_ErrorCallBack.
*
*                       Changes in xcfa_hw.h:
*                       Added the register offsets and bit masks for the
*                       registers and added backward compatibility for macros.
*
*                       Changes in xcfa.c:
*                       Renamed this file as below:
*                       cfa.c -> xcfa.c
*                       Implemented the following functions:
*                       XCfa_CfgInitialize, XCfa_Setup, XCfa_GetVersion,
*                       XCfa_EnableDbgByPass, XCfa_IsDbgByPassEnabled,
*                       XCfa_DisableDbgBypass, XCfa_EnableDbgTestPattern,
*                       XCfa_IsDbgTestPatternEnabled,
*                       XCfa_DisableDbgTestPattern, XCfa_GetDbgFrameCount,
*                       XCfa_GetDbgLineCount, XCfa_GetDbgPixelCount,
*                       XCfa_SetActiveSize, XCfa_GetActiveSize,
*                       XCfa_SetBayerPhase, XCfa_GetBayerPhase,
*                       StubCallBack, StubErrCallBack.
*
*                       Changes in  xcfa_intr.c:
*                       Implemented the following functions:
*                       XCfa_IntrHandler, XCfa_SetCallBack.
*
*                       Changes in xcfa_selftest.c:
*                       Implemented XCfa_SelfTest function.
*
*                       Changes in xcfa_sinit.c :
*                       Implemented XCfa_LookupConfig function.
* 7.1   ms     01/16/17 Updated the parameter naming from
*                       XPAR_CFA_NUM_INSTANCES to XPAR_XCFA_NUM_INSTANCES
*                       to avoid  compilation failure for
*                       XPAR_CFA_NUM_INSTANCES as the tools are generating
*                       XPAR_XCFA_NUM_INSTANCES in the generated xcfa_g.c
*                       for fixing MISRA-C files. This is a fix for
*                       CR-966099 based on the update in the tools.
*       ms     01/23/17 Added xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
* </pre>
*
******************************************************************************/

#ifndef XCFA_H_
#define XCFA_H_		/**< Prevent circular inclusions by using
			  *  protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xcfa_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
 *  @{
 */
/**
 *
 * These constants specify different types of handlers and used to
 * differentiate interrupt requests from core.
 *
 */
enum {
	XCFA_HANDLER_PROCSTART = 1,	/**< A processing start event interrupt
					  *  type */
	XCFA_HANDLER_FRAMEDONE,		/**< A frame done event interrupt
					  *  type */
	XCFA_HANDLER_ERROR		/**< An error condition interrupt
					  *  type */
} ;
/*@}*/

/** @name Bayer phase
 * @{
 */
/**
* These constants specify Bayer phase combinations of the core.
*/
enum XCfa_BayerPhaseCombination {
	XCFA_RGRG_COMBINATION,	/**< Red green combination */
	XCFA_GRGR_COMBINATION,	/**< Green red combination */
	XCFA_GBGB_COMBINATION,	/**< Green blue combination */
	XCFA_BGBG_COMBINATION,	/**< Blue green combination */
} ;
/*@}*/

/** @name Active size range macros
 * @{
 */
#define XCFA_VSIZE_FIRST	32	/**< Vertical Size starting value */
#define XCFA_VSIZE_LAST		7680	/**< Vertical Size ending value */
#define XCFA_HSIZE_FIRST	32	/**< Horizontal Size starting value */
#define XCFA_HSIZE_LAST		7680	/**< Horizontal Size ending value */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This function macro enables the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_Enable(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_Enable(InstancePtr) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XCFA_CONTROL_OFFSET), \
		((XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCFA_CONTROL_OFFSET))) | (XCFA_CTL_SW_EN_MASK)))

/*****************************************************************************/
/**
*
* This function macro disables the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_Disable(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_Disable(InstancePtr) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XCFA_CONTROL_OFFSET), \
		((XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCFA_CONTROL_OFFSET))) & (~(XCFA_CTL_SW_EN_MASK))))

/*****************************************************************************/
/**
*
* This function macro enables/starts the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_Start(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_Start	XCfa_Enable

/*****************************************************************************/
/**
*
* This function macro disables/stops the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_Stop(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_Stop	XCfa_Disable

/*****************************************************************************/
/**
*
* This function macro commits all the register value changes made so far
* by the software to the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_RegUpdateEnable(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_RegUpdateEnable(InstancePtr) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCFA_CONTROL_OFFSET), \
		((XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCFA_CONTROL_OFFSET))) | (XCFA_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This function macro prevents the CFA core from committing recent changes made
* so far by the software. When disabled, changes to other configuration
* registers are stored, but do not effect the behavior of the core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_RegUpdateDisable(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_RegUpdateDisable(InstancePtr) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCFA_CONTROL_OFFSET), \
		((XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCFA_CONTROL_OFFSET)))) & (~(XCFA_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This function macro resets a CFA core at the end of the frame being
* processed. It enables core automatically synchronizes to the SOF of the core
* to prevent image tearing. This function macro is differ from XCfa_Reset().
*
* On the next rising-edge of SOF following a call to XCfa_SyncReset(),
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_SyncReset(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_SyncReset(InstancePtr) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XCFA_CONTROL_OFFSET), (XCFA_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This macro resets CFA core instance. This reset effects the core
* immediately and may cause image tearing.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_Reset(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_Reset(InstancePtr) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XCFA_CONTROL_OFFSET), (XCFA_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This function macro returns the pending status of a CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	The pending interrupts of the CFA core. Use XCFA_IXR_*_MASK constants
*		defined in xcfa_hw.h to interpret this value.
*
* @note		C-style signature:
*		u32 XCfa_StatusGePending(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_StatusGetPending(InstancePtr) \
	XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XCFA_STATUS_OFFSET)) & (XCFA_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This function macro clears/acknowledges pending interrupts of the CFA core.
* in the Status register. Bit positions of 1 will be cleared.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XCFA_IXR_*_MASK constants defined in xcfa_hw.h to
*		create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_IntrClear(XCfa *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XCfa_IntrClear(InstancePtr, IntrType) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XCFA_STATUS_OFFSET), ((IntrType) & \
				((u32)(XCFA_IXR_ALLINTR_MASK))))

/*****************************************************************************/
/**
*
* This function macro enables the given individual interrupt(s) on the
* CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XCFA_IXR_*_MASK bits defined in xcfa_hw.h.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
*		C-style signature:
*		void XCfa_IntrEnable(XCfa *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XCfa_IntrEnable(InstancePtr, IntrType) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCFA_IRQ_EN_OFFSET), \
		(((IntrType) & (XCFA_IXR_ALLINTR_MASK)) | \
			(XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XCFA_IRQ_EN_OFFSET)))))

/*****************************************************************************/
/**
*
* This function macro disables the given individual interrupt(s) on the
* CFA core by updating Irq_Enable register.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XCFA_IXR_*_MASK bits defined in xcfa_hw.h.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType,
*		if enabled before this macro is called, will remain enabled.
*		C-style signature:
*		void XCfa_IntrDisable(XCfa *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XCfa_IntrDisable(InstancePtr, IntrType) \
	XCfa_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCFA_IRQ_EN_OFFSET), \
		((XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCFA_IRQ_EN_OFFSET))) & ((~(IntrType)) & \
				(XCFA_IXR_ALLINTR_MASK))))

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupts of the CFA core.
*
* @param	InstancePtr is a pointer to the XCfa instance to be worked on.
*
* @return	The pending interrupts of the CFA. Use XCFA_IXR_*_MASK
*		constants defined in xcfa_hw.h to interpret this value.
*		The returned value is a logical AND of the contents of the
*		STATUS Register and the IRQ_ENABLE Register.
*
* @note		C-style signature:
*		u32 XCfa_IntrGetPending(XCfa *InstancePtr)
*
******************************************************************************/
#define XCfa_IntrGetPending(InstancePtr) \
	XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XCFA_IRQ_EN_OFFSET)) & \
		((XCfa_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XCFA_STATUS_OFFSET))) & ((u32)(XCFA_IXR_ALLINTR_MASK)))

/**************************** Type Definitions *******************************/

/**
*
* This typedef contains configuration information for the CFA core.
* Each CFA core should have a configuration structure associated.
*
******************************************************************************/
typedef struct {
	u16 DeviceId;			/**< DeviceId is the unique ID of
					  *  the core */
	u32 BaseAddress;		/**< BaseAddress is the physical base
					  *  address of the core's
					  *  registers */
	u32 SlaveAxisVideoFormat;	/**< Slave Axis Video Format */
	u32 MasterAxisVideoFormat;	/**< Master Axis Video Format */
	u8 BayerPhase;			/**< Bayer Phase */
	u32 SlaveAxiClkFreqHz;		/**< Slave Clock Frequency */
	u32 ActiveRows;			/**< Active rows */
	u32 ActiveColumns;		/**< Active Columns */
	u32 MaxColumns;			/**< Maximum Columns */
	u16 HasIntcIf;			/**< Has Interrupt Control */
	u16 HasDebug;			/**< Has Debug GUI specified */
	u32 HorFilt;			/**< Optional Horizontal Filter */
	u32 FringeTol;			/**< Fringe Tolerance */
} XCfa_Config;

/*****************************************************************************/
/**
*
* Callback type for all interrupts except error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
******************************************************************************/
typedef void (*XCfa_CallBack)(void *CallBackRef);

/*****************************************************************************/
/**
*
* Callback type for error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCFA_IXR_*_MASK values
*		defined in xcfa_hw.h.
*
******************************************************************************/
typedef void (*XCfa_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/**
 *
 * The XCfa instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific
 * instance.
 */
typedef struct {
	XCfa_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Core instance is initialized */
	u16 HSize;			/**< Active video horizontal size */
	u16 VSize;			/**< Active video vertical size */

	/* IRQ callbacks here */
	XCfa_CallBack ProcStartCallBack;/**< Callback for processing start
 					  *  interrupt */
	void *ProcStartRef;		/**< To be passed to the process start
					  *  interrupt callback */
	XCfa_CallBack FrameDoneCallBack;/**< Callback for frame done
					  *  interrupt */
	void *FrameDoneRef;		/**< To be passed to the frame done
					  *  interrupt callback */
	XCfa_ErrorCallBack ErrCallBack; /**< Callback for error interrupt */
	void *ErrRef;			/**< To be passed to the error
					  *  interrupt callback */
} XCfa;

/************************** Function Prototypes ******************************/

/* Static lookup function implemented in xcfa_sinit.c */
XCfa_Config *XCfa_LookupConfig(u16 DeviceId);

/* Implemented in xcfa.c */
int XCfa_CfgInitialize(XCfa *InstancePtr, XCfa_Config *CfgPtr,
				u32 EffectiveAddr);
void XCfa_Setup(XCfa *InstancePtr);
void XCfa_EnableDbgByPass(XCfa *InstancePtr);
int XCfa_IsDbgByPassEnabled(XCfa *InstancePtr);
void XCfa_DisableDbgBypass(XCfa *InstancePtr);
void XCfa_EnableDbgTestPattern(XCfa *InstancePtr);
int XCfa_IsDbgTestPatternEnabled(XCfa *InstancePtr);
void XCfa_DisableDbgTestPattern(XCfa *InstancePtr);
u32 XCfa_GetVersion(XCfa *InstancePtr);
u32 XCfa_GetDbgFrameCount(XCfa *InstancePtr);
u32 XCfa_GetDbgLineCount(XCfa *InstancePtr);
u32 XCfa_GetDbgPixelCount(XCfa *InstancePtr);
void XCfa_SetActiveSize(XCfa *InstancePtr, u16 HSize, u16 VSize);
void XCfa_GetActiveSize(XCfa *InstancePtr, u16 *HSize, u16 *VSize);
void XCfa_SetBayerPhase(XCfa *InstancePtr,
			enum XCfa_BayerPhaseCombination BayerPhase);
u32 XCfa_GetBayerPhase(XCfa *InstancePtr);

/* Self - Test function in xcfa_selftest.c */
int XCfa_SelfTest(XCfa *InstancePtr);

/* Interrupt related functions in xcfa_intr.c */
void XCfa_IntrHandler(void *InstancePtr);
int XCfa_SetCallBack(XCfa *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef);

#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
