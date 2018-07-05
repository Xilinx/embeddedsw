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
* @file xosd.h
* @addtogroup osd_v4_0
* @{
* @details
*
* This is main header file of the Xilinx On-Screen-Display (OSD) core.
*
* An OSD is an image superimposed on a screen picture, commonly used by modern
* televisions, VCRs, and DVD players to display information such as volume,
* channel, and time.
*
* Xilinx OSD core has the following main features:
*
* - Read Video Data from one of three sources as
*	- VFBC/Frame Buffer,
*	- VideoBus and
*	- Graphics Controller.
* - Alpha Compositing and Alpha Blending of up to 8 layers.
* - Up to 8 priorities, one for each of the layers.
* - Real-Time Graphics Controller.
* - Write Composited Video Data to either
*	- VFBC/Frame Buffer, or
*	- VideoBus.
*
* For a full description of OSD features, please see the hardware spec.
*
* <b>Interrupt Service </b>
*
* Three interrupt types are supported:
*
* - Processing Start Interrupt
* - Frame Done Interrupt
* - Error Interrupt
*
* <b>Software Initialization </b>
*
* Please follow the example provided with this driver for the steps
* to use this driver.
*
* <b>Cache Coherency</b>
*
* <b>Alignment</b>
*
* <b>Limitations</b>
*
* <b>BUS Interface</b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a xd     08/18/08 First release
* 1.01a xd     07/30/10 Added device version support; Supported Doxygen; Fixed
*                       CR #534952
* 1.02a xd     12/21/10 Removed endian conversion for text bank loading
* 1.03a cm     09/07/11 Updated XOSD_GetLayerAlpha(), XOSD_SetLayerAlpha(),
*                       XOSD_SetBackgroundColor() and XOSD_GetBackgroundColor()
*                       to allow 10 and 12 bit alpha and background colors.
* 2.00a cjm    12/18/12 Converted from xio.h to xil_io.h, translating
*                       basic types, MB cache functions, exceptions and
*                       assertions to xil_io format.
* 3.0   adk    19/12/13 Updated as per the New Tcl API's.
* 4.0   adk    02/18/14 Converted defined macros to enum types.
*                       Removed interrupt types XOSD_HANDLER_VBISTART and
*                       XOSD_HANDLER_VBIEND.
*                       Added interrupt type: XOSD_HANDLER_PROCSTART.
*                       Renamed the following function macros:
*                       XOSD_Enable -> XOsd_Enable,
*                       XOSD_Disable -> XOsd_Disable,
*                       XOSD_RegUpdateEnable -> XOsd_RegUpdateEnable,
*                       XOSD_RegUpdateDisable -> XOsd_RegUpdateDisable,
*                       XOSD_Reset -> XOsd_Reset,
*                       XOSD_IntrEnable -> XOsd_IntrEnable,
*                       XOSD_IntrDisable -> XOsd_IntrDisable,
*                       XOSD_IntrGetPending -> XOsd_IntrGetPending,
*                       XOSD_IntrClear -> XOsd_IntrClear.
*
*                       Added the following function macros:
*                       XOsd_SyncReset, XOsd_StatusGetPending, XOsd_FSyncReset.
*
*                       Removed the following function macros:
*                       XOSD_IntrEnableGlobal, XOSD_IntrDisableGlobal.
*
*                       Renamed S_AXIS_VIDEO_DATA_WIDTH ->
*                                                      SlaveAxisVideoDataWidth.
*                       Removed struct members from core structure:
*                       VbiStartCallBack, VbiStartRef, VbiEndCallBack,
*                       VbiEndRef.
*
*                       Added struct members in core structure:
*                       ProcStartCallBack, ProcStartRef.
*
*                       Added the following function prototypes:
*                       XOsd_SelfTest.
*
*                       Removed the following function prototypes:
*                       XOSD_SetBlankPolarity.
*
*                       Changes in xosd_sinit.c:
*                       Renamed the following functions:
*                       XOSD_LookupConfig - > XOsd_LookupConfig
*
*                       Changes in xosd_selftest.c:
*                       Implemented the following functions:
*                       XOsd_SelfTest.
*
*                       Changes in xosd_intr.c:
*                       Renamed the following functions:
*                       XOSD_IntrHandler - > XOsd_IntrHandler.
*                       XOSD_SetCallBack -> XOsd_SetCallBack.
*                       Removed the following handlers:
*                       XOSD_HANDLER_VBISTART, XOSD_HANDLER_VBIEND.
*                       Added new handler XOSD_HANDLER_PROCSTART.
*                       Added Doxygen support, adherence to Xilinx
*                       coding guidelines.
*
*                       Changes in xosd_hw.h:
*                       Suffixed "_OFFSET" to all register offset macros.
*                       Added register offsets, bit masks for the registers and
*                       added backward compatibility for macros.
*
*                       Removed following macros:
*                       XOSD_GIER_GIE_MASK, XOSD_IXR_GAO_MASK
*                       XOSD_IXR_GIE_MASK, XOSD_IXR_OOE_MASK,
*                       XOSD_IXR_IUE_MASK, XOSD_IXR_VBIE_MASK,
*                       XOSD_IXR_VBIS_MASK, XOSD_IXR_FE_MASK, XOSD_IXR_FD_MASK,
*                       XOSD_IXR_ALLIERR_MASK.
*
*                       Changes from xosd.c:
*                       Renamed S_AXIS_VIDEO_DATA_WIDTH ->
*                                                      SlaveAxisVideoDataWidth.
*                       Removed from XOsd_CfgInitialize:
*                       VbiStartCallBack, VbiStartRef, VbiEndCallBack,
*                       VbiEndRef.
*
*                       Added in XOsd_CfgInitialize:
*                       ProcStartCallBack, ProcStartRef.
*
*                       Renamed the following function prototypes:
*                       XOSD_CfgInitialize -> XOsd_CfgInitialize,
*                       XOSD_SetScreenSize -> XOsd_SetActiveSize,
*                       XOSD_GetScreenSize -> XOsd_GetActiveSize,
*                       XOSD_SetBackgroundColor -> XOsd_SetBackgroundColor,
*                       XOSD_GetBackgroundColor -> XOSD_GetBackgroundColor,
*                       XOSD_SetLayerDimension -> XOsd_SetLayerDimension,
*                       XOSD_GetLayerDimension -> XOsd_GetLayerDimension,
*                       XOSD_SetLayerAlpha -> XOsd_SetLayerAlpha,
*                       XOSD_GetLayerAlpha -> XOsd_GetLayerAlpha,
*                       XOSD_SetLayerAlpha -> XOsd_SetLayerAlpha,
*                       XOSD_GetLayerAlpha -> XOsd_GetLayerAlpha,
*                       XOSD_SetLayerPriority -> XOsd_SetLayerPriority,
*                       XOSD_GetLayerPriority -> XOsd_GetLayerPriority,
*                       XOSD_EnableLayer -> XOsd_EnableLayer,
*                       XOSD_DisableLayer -> XOsd_DisableLayer,
*                       XOSD_LoadColorLUTBank - > XOsd_LoadColorLUTBank,
*                       XOSD_LoadCharacterSetBank -> XOsd_LoadCharacterSetBank,
*                       XOSD_LoadTextBank - > XOsd_LoadTextBank,
*                       XOSD_SetActiveBank -> XOsd_SetActiveBank,
*                       XOSD_CreateInstruction -> XOsd_CreateInstruction,
*                       XOSD_LoadInstructionList -> XOsd_LoadInstructionList,
*                       XOSD_LookupConfig -> XOsd_LookupConfig,
*                       XOSD_IntrHandler -> XOsd_IntrHandler,
*                       XOSD_SetCallBack -> XOsd_SetCallBack.
*
*                       Changed the prototype of XOSD_GetVersion and renamed it as
*                       XOsd_GetVersion
*
*                       Removed the following function implementation:
*                       XOSD_SetBlankPolarity.
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
* </pre>
*
******************************************************************************/

#ifndef XOSD_H_
#define XOSD_H_		/**< Prevent circular inclusions by
			  *  using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xosd_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt types for setting up callbacks
 *  @{
 */
enum {
	XOSD_HANDLER_PROCSTART = 1,	/**< A processing start event interrupt
					  *  type */
	XOSD_HANDLER_FRAMEDONE,		/**< A frame done event interrupt
					  *  type */
	XOSD_HANDLER_ERROR		/**< An error condition interrupt
					  *  type */
};
/*@}*/

/** @name Compatibility Macros
 *  @{
 */
#define XOSD_Config			XOsd_Config
#define XOSD_Layer			XOsd_Layer
#define XOSD_CallBack			XOsd_CallBack
#define XOSD_ErrorCallBack		XOsd_ErrorCallBack
#define XOSD				XOsd
#define XOSD_Instruction		XOsd_Instruction
#define XOSD_Enable			XOsd_Enable
#define XOSD_Disable			XOsd_Disable
#define XOSD_RegUpdateEnable		XOsd_RegUpdateEnable
#define XOSD_RegUpdateDisable		XOsd_RegUpdateDisable
#define XOSD_Reset			XOsd_Reset
#define XOSD_IntrEnable			XOsd_IntrEnable
#define XOSD_IntrDisable		XOsd_IntrDisable
#define XOSD_IntrGetPending		XOsd_IntrGetPending
#define XOSD_IntrClear			XOsd_IntrClear
#define XOSD_CfgInitialize		XOsd_CfgInitialize
#define XOSD_SetScreenSize		XOsd_SetActiveSize
#define XOSD_GetScreenSize		XOsd_GetActiveSize
#define XOSD_SetBackgroundColor		XOsd_SetBackgroundColor
#define XOSD_GetBackgroundColor		XOsd_GetBackgroundColor
#define XOSD_SetLayerDimension		XOsd_SetLayerDimension
#define XOSD_GetLayerDimension		XOsd_GetLayerDimension
#define XOSD_SetLayerAlpha		XOsd_SetLayerAlpha
#define XOSD_GetLayerAlpha		XOsd_GetLayerAlpha
#define XOSD_SetLayerPriority		XOsd_SetLayerPriority
#define XOSD_GetLayerPriority		XOsd_GetLayerPriority
#define XOSD_EnableLayer		XOsd_EnableLayer
#define XOSD_DisableLayer		XOsd_DisableLayer
#define XOSD_LoadColorLUTBank		XOsd_LoadColorLUTBank
#define XOSD_LoadCharacterSetBank	XOsd_LoadCharacterSetBank
#define XOSD_LoadTextBank		XOsd_LoadTextBank
#define XOSD_SetActiveBank		XOsd_SetActiveBank
#define XOSD_CreateInstruction		XOsd_CreateInstruction
#define XOSD_GetVersion			XOsd_GetVersion
#define XOSD_LoadInstructionList	XOsd_LoadInstructionList
#define XOSD_LookupConfig		XOsd_LookupConfig
#define XOSD_IntrHandler		XOsd_IntrHandler
#define XOSD_SetCallBack		XOsd_SetCallBack
/*@}*/

/**************************** Type Definitions *******************************/

/**
* OSD core configuration structure.
* Each OSD core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID
				  *  of the core */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the OSD core registers */
	u16 LayerNum;		/**< The number of Layers */
	u16 SlaveAxisVideoDataWidth;	/**< Slave Axis Video Data Width */
	u16 Layer0Type;		/**< Type of Layer #0 */
	u16 Layer1Type;		/**< Type of Layer #1 */
	u16 Layer2Type;		/**< Type of Layer #2 */
	u16 Layer3Type;		/**< Type of Layer #3 */
	u16 Layer4Type;		/**< Type of Layer #4 */
	u16 Layer5Type;		/**< Type of Layer #5 */
	u16 Layer6Type;		/**< Type of Layer #6 */
	u16 Layer7Type;		/**< Type of Layer #7 */

	/**< Layer 0 */
	u16 Layer0InstructionMemSize;	/**< Instruction Memory Size */
	u16 Layer0InstructionBoxEnable;	/**< Instruction Box Enable */
	u16 Layer0InstructionLineEnable;/**< Instruction Line Enable */
	u16 Layer0InstructionTextEnable;/**< Instruction Text Enable */
	u16 Layer0ColorLutSize;		/**< Color Look Up Table (LUT) Size */
	u16 Layer0ColorLutMemoryType;	/**< Color LUT Memory Type */
	u16 Layer0FontNumChars;		/**< Font: Number of characters */
	u16 Layer0FontWidth;		/**< Font: Width */
	u16 Layer0FontHeight;		/**< Font: Height */
	u16 Layer0FontBitsPerPixel;	/**< Font: Number of bits per pixel */
	u16 Layer0FontAsciiOffset;	/**< Font: ASCII offset of 1st
					  *  character */
	u16 Layer0TextNumStrings;	/**< Text: Number of Strings */
	u16 Layer0TextMaxStringLength;	/**< Text: Maximum length of a
					  *  String */

	/**< Layer 1 */
	u16 Layer1InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer1InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer1InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer1InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer1ColorLutSize;			/**< Color LUT Size */
	u16 Layer1ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer1FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer1FontWidth;			/**< Font: Width */
	u16 Layer1FontHeight;			/**< Font: Height */
	u16 Layer1FontBitsPerPixel;		/**< Font: Number of bits
						  *  per pixel */
	u16 Layer1FontAsciiOffset;		/**< Font: ASCII offset of 1st
						  *  character */
	u16 Layer1TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer1TextMaxStringLength;		/**< Text: Maximum length of
						  *  a String */

	/**< Layer 2 */
	u16 Layer2InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer2InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer2InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer2InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer2ColorLutSize;			/**< Color LUT Size */
	u16 Layer2ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer2FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer2FontWidth;			/**< Font: Width */
	u16 Layer2FontHeight;			/**< Font: Height */
	u16 Layer2FontBitsPerPixel;		/**< Font: Number of bits
						  *  per pixel */
	u16 Layer2FontAsciiOffset;		/**< Font: ASCII offset of 1st
						  *  character */
	u16 Layer2TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer2TextMaxStringLength;		/**< Text: Maximum length of
						  *  a String */

	/**< Layer 3 */
	u16 Layer3InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer3InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer3InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer3InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer3ColorLutSize;			/**< Color LUT Size */
	u16 Layer3ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer3FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer3FontWidth;			/**< Font: Width */
	u16 Layer3FontHeight;			/**< Font: Height */
	u16 Layer3FontBitsPerPixel;		/**< Font: Number of bits
						  *  per pixel */
	u16 Layer3FontAsciiOffset;		/**< Font: ASCII offset of
						  *  1st character */
	u16 Layer3TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer3TextMaxStringLength;		/**< Text: Maximum length of
						  *  a String */

	/**< Layer 4 */
	u16 Layer4InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer4InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer4InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer4InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer4ColorLutSize;			/**< Color LUT Size */
	u16 Layer4ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer4FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer4FontWidth;			/**< Font: Width */
	u16 Layer4FontHeight;			/**< Font: Height */
	u16 Layer4FontBitsPerPixel;		/**< Font: Number of bits
						  *  per pixel */
	u16 Layer4FontAsciiOffset;		/**< Font: ASCII offset of 1st
						  *  character */
	u16 Layer4TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer4TextMaxStringLength;		/**< Text: Maximum length of
						  *  a String */

	/**< Layer 5 */
	u16 Layer5InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer5InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer5InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer5InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer5ColorLutSize;			/**< Color LUT Size */
	u16 Layer5ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer5FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer5FontWidth;			/**< Font: Width */
	u16 Layer5FontHeight;			/**< Font: Height */
	u16 Layer5FontBitsPerPixel;		/**< Font: Number of bits per
						  *  pixel */
	u16 Layer5FontAsciiOffset;		/**< Font: ASCII offset of
						  *  1st character */
	u16 Layer5TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer5TextMaxStringLength;		/**< Text: Maximum length of
						  *  a String */

	/**< Layer 6 */
	u16 Layer6InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer6InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer6InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer6InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer6ColorLutSize;			/**< Color LUT Size */
	u16 Layer6ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer6FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer6FontWidth;			/**< Font: Width */
	u16 Layer6FontHeight;			/**< Font: Height */
	u16 Layer6FontBitsPerPixel;		/**< Font: Number of bits
						  *  per pixel */
	u16 Layer6FontAsciiOffset;		/**< Font: ASCII offset of
						  *  1st character */
	u16 Layer6TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer6TextMaxStringLength;		/**< Text: Maximum length of
						  *  a String */

	/**< Layer 7 */
	u16 Layer7InstructionMemSize;		/**< Instruction Memory Size */
	u16 Layer7InstructionBoxEnable;		/**< Instruction Box Enable */
	u16 Layer7InstructionLineEnable;	/**< Instruction Line Enable */
	u16 Layer7InstructionTextEnable;	/**< Instruction Text Enable */
	u16 Layer7ColorLutSize;			/**< Color LUT Size */
	u16 Layer7ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 Layer7FontNumChars;			/**< Font: Number of
						  *  characters */
	u16 Layer7FontWidth;			/**< Font: Width */
	u16 Layer7FontHeight;			/**< Font: Height */
	u16 Layer7FontBitsPerPixel;		/**< Font: Number of bits
						  *  per pixel */
	u16 Layer7FontAsciiOffset;		/**< Font: ASCII offset of
						  *  1st character */
	u16 Layer7TextNumStrings;		/**< Text: Number of Strings */
	u16 Layer7TextMaxStringLength;		/**< Text: Maximum length
						  *  of a String */
} XOsd_Config;

/**
* The OSD core layer info structure
*/
typedef struct {

	u16 LayerType;			/**< Type of the layer */
	u16 InstructionNum;		/**< The Number of Instructions */
	u16 InstructionBoxEnable;	/**< Instruction Box Enable */
	u16 InstructionLineEnable;	/**< Instruction Line Enable */
	u16 InstructionTextEnable;	/**< Instruction Text Enable */
	u16 ColorLutSize;		/**< Color LUT Size */
	u16 ColorLutMemoryType;		/**< Color LUT Memory Type */
	u16 FontNumChars;		/**< Font: Number of characters */
	u16 FontWidth;			/**< Font: Width */
	u16 FontHeight;			/**< Font: Height */
	u16 FontBitsPerPixel;		/**< Font: Number of bits per pixel */
	u16 FontAsciiOffset;		/**< Font: ASCII offset of
					  *  1st character */
	u16 TextNumStrings;		/**< Text: Number of Strings */
	u16 TextMaxStringLength;	/**< Text: Maximum length of
					  *  a String */
} XOsd_Layer;

/*****************************************************************************/
/**
*
* This function callback type for all interrupts except error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions and passed back
*		to the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
typedef void (*XOsd_CallBack)(void *CallBackRef);

/*****************************************************************************/
/**
*
* This function callback type for error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the
*		upper layer when setting the callback functions, and passed
*		back to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XOSD_IXR_* values defined in
*		xosd_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
typedef void (*XOsd_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/**
* The XOsd instance data. An instance must be allocated for each
* OSD core in use.
*/
typedef struct {
	XOsd_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Core instance is initialized */
	u32 InstructionInExternalMem;	/**< Flag indicating if
					  * the instruction list is from
					  * external memory */
	u32 ScreenHeight;		/**< Screen Height of the OSD output */
	u32 ScreenWidth;		/**< Screen Width of the OSD output */

	XOsd_Layer Layers[(XOSD_MAX_NUM_OF_LAYERS)];	/**< Properties of
							  *  layers */

	/*
	 * IRQ Callbacks
	 */
	XOsd_CallBack ProcStartCallBack; /**< Call back for Processing
					   *  Start interrupt */
	void *ProcStartRef;		 /**< To be passed to the
					  * Process Start
					  *  interrupt callback */
	XOsd_CallBack FrameDoneCallBack; /**< Call back for Frame Done
					   *  interrupt */
	void *FrameDoneRef;		 /**< To be passed to the Frame
					   *  Done interrupt callback */
	XOsd_ErrorCallBack ErrCallBack;  /**< Call back for Error interrupt */
	void *ErrRef;			 /**< To be passed to the Error
					   *  interrupt callback */
} XOsd;

/**
* The XOsd_Instruction data structure for holding one GC instruction.
*/
typedef u32 XOsd_Instruction[XOSD_INS_SIZE];

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This function macro enables an OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_Enable(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_Enable(InstancePtr) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_CTL_OFFSET), \
		((XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XOSD_CTL_OFFSET))) | (XOSD_CTL_EN_MASK)))

/*****************************************************************************/
/**
*
* This function macro disables an OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_Disable(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_Disable(InstancePtr) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_CTL_OFFSET), \
		((XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XOSD_CTL_OFFSET))) & (~(XOSD_CTL_EN_MASK))))

/*****************************************************************************/
/**
*
* This function macro commits all the register value changes made so far by
* the software to the OSD core instance.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_RegUpdateEnable(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_RegUpdateEnable(InstancePtr) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_CTL_OFFSET), \
		((XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XOSD_CTL_OFFSET))) | (XOSD_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This function macro prevents the OSD core from committing recent changes
* made so far by the software. When disabled, changes to other configuration
* registers are stored, but do not effect the behavior of the core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_RegUpdateDisable(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_RegUpdateDisable(InstancePtr) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_CTL_OFFSET), \
		((XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XOSD_CTL_OFFSET))) & (~(XOSD_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This function macro resets an OSD core This effects the core immediately, and
* may cause image tearing.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_Reset(XOsd *InstancePtr)
*		This bit automatically clears when reset complete.
*
******************************************************************************/
#define XOsd_Reset(InstancePtr) \
{ \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_CTL_OFFSET), \
		(XOSD_CTL_SW_RST_MASK)); \
	(InstancePtr)->InstructionInExternalMem = (u32)0x0; \
	(InstancePtr)->ScreenHeight = (u32)0x0; \
	(InstancePtr)->ScreenWidth = (u32)0x0; \
}

/*****************************************************************************/
/**
*
* This function macro enables the given individual interrupt(s) on the
* OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting.This mask is formed by OR'ing of
*		XOSD_IXR_*_MASK bits defined in xosd_hw.h.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
*		C-style signature:
*		void XOsd_IntrEnable(XOsd *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XOsd_IntrEnable(InstancePtr, IntrType) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_IER_OFFSET), \
		(((IntrType) & (XOSD_IXR_ALLINTR_MASK)) | \
			(XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XOSD_IER_OFFSET)))))

/*****************************************************************************/
/**
*
* This function macro disables the given individual interrupt(s) on the
* OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting.This mask is formed by OR'ing of
*		XOSD_IXR_*_MASK bits defined in xosd_hw.h.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType,
*		if enabled before this macro is called, will remain enabled.
*		C-style signature:
*		void XOsd_IntrDisable(XOsd *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XOsd_IntrDisable(InstancePtr, IntrType) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_IER_OFFSET), \
		(((~(IntrType)) & (XOSD_IXR_ALLINTR_MASK)) & \
			(XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XOSD_IER_OFFSET)))))

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupts of the OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	The pending interrupts of the OSD core. Use XOSD_IXR_*_MASK
*		constants defined in xosd_hw.h to interpret this value.
*
* @note		C-style signature:
*		u32 XOsd_IntrGetPending(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_IntrGetPending(InstancePtr) \
	XOsd_ReadReg((InstancePtr)->Config.BaseAddress, (XOSD_IER_OFFSET)) & \
		(XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XOSD_STATUS_OFFSET))) & (XOSD_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This function macro clears/acknowledges pending interrupts of the
* OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XOSD_IXR_*_MASK constants defined in xosd_hw.h to
*		create this parameter value.
*
* @return	None
*
* @note		C-style signature:
*		void XOsd_IntrClear(XOsd *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XOsd_IntrClear(InstancePtr, IntrType) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XOSD_STATUS_OFFSET), ((IntrType) & (XOSD_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This function macro returns the pending status of the OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	The pending interrupts of the OSD. Use XOSD_IXR_* constants
*		defined in xosd_hw.h to interpret this value.
*
* @note		C-style signature:
*		u32 XOsd_StatusGePending(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_StatusGetPending(InstancePtr) \
	XOsd_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XOSD_STATUS_OFFSET)) & (XOSD_IXR_ALLINTR_MASK)


/*****************************************************************************/
/**
*
* This function macro resets a OSD core at the end of the frame being
* processed. It enables core automatically synchronizes to the SOF of the core
* to prevent image tearing. This function macro is differ from XOsd_Reset().
*
* On the next rising-edge of SOF following a call to XOsd_SyncReset(),
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_SyncReset(XOsd *InstancePtr)
*		This bit automatically clears when reset complete.
*
******************************************************************************/
#define XOsd_SyncReset(InstancePtr) \
	XOsd_WriteReg((InstancePtr)->Config.BaseAddress, (XOSD_CTL_OFFSET), \
		(XOSD_CTL_FSYNC_MASK))


/*****************************************************************************/
/**
*
* This macro resets the OSD core at the end of the frame being
* processed. It enables core automatically synchronizes to the SOF of the core
* to prevent image tearing. This function macro is differ from XOsd_Reset().
*
* On the next rising-edge of SOF following a call to XOsd_SyncReset(),
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
******************************************************************************/
#define XOsd_FSyncReset		XOsd_SyncReset

/*****************************************************************************/
/**
*
* This function macro stops an OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_Start(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_Start	XOsd_Enable

/*****************************************************************************/
/**
*
* This function macro stops an OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_Stop(XOsd *InstancePtr)
*
******************************************************************************/
#define XOsd_Stop	XOsd_Disable


/************************** Function Prototypes ******************************/

/* Initialization and control functions in xosd.c */

/* Initialization */
int XOsd_CfgInitialize(XOsd *InstancePtr, XOsd_Config *CfgPtr,
			u32 EffectiveAddr);

/* Set/Get Active Size of the OSD Output */
void XOsd_SetActiveSize(XOsd *InstancePtr, u32 Width, u32 Height);
void XOsd_GetActiveSize(XOsd *InstancePtr, u32 *WidthPtr, u32 *HeightPtr);

/* Set/Get Background color */
void XOsd_SetBackgroundColor(XOsd *InstancePtr, u16 Red, u16 Blue, u16 Green);
void XOsd_GetBackgroundColor(XOsd *InstancePtr, u16 *RedPtr, u16 *BluePtr,
				u16 *GreenPtr);

/* Layer related functions */
void XOsd_SetLayerDimension(XOsd *InstancePtr, u8 LayerIndex, u16 XStart,
				u16 YStart, u16 XSize, u16 YSize);
void XOsd_GetLayerDimension(XOsd *InstancePtr, u8 LayerIndex, u16 *XStartPtr,
				u16 *YStartPtr, u16 *XSizePtr, u16 *YSizePtr);

void XOsd_SetLayerAlpha(XOsd *InstancePtr, u8 LayerIndex,
			u16 GlobalAlphaEnable, u16 GlobalAlphaValue);
void XOsd_GetLayerAlpha(XOsd *InstancePtr, u8 LayerIndex,
			u16 *GlobalAlphaEnablePtr, u16 *GlobalAlphaValuePtr);

void XOsd_SetLayerPriority(XOsd *InstancePtr, u8 LayerIndex, u8 Priority);
void XOsd_GetLayerPriority(XOsd *InstancePtr, u8 LayerIndex, u8 *PriorityPtr);

void XOsd_EnableLayer(XOsd *InstancePtr, u8 LayerIndex);
void XOsd_DisableLayer(XOsd *InstancePtr, u8 LayerIndex);

/* Graphics Controller related functions */
void XOsd_LoadColorLUTBank(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
			u32 ColorData[]);
void XOsd_LoadCharacterSetBank(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
			u32 CharSetData[]);
void XOsd_LoadTextBank(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
			u32 TextData[]);
void XOsd_SetActiveBank(XOsd *InstancePtr, u8 GcIndex, u8 ColorBankIndex,
		u8 CharBankIndex, u8 TextBankIndex, u8 InstructionBankIndex);

/* Create and load instruction(s) */
void XOsd_CreateInstruction(XOsd *InstancePtr, u32 InstructionPtr[],
		u8 GcIndex, u16 ObjType, u8 ObjSize, u16 XStart,
		u16 YStart, u16 XEnd, u16 YEnd, u8 TextIndex, u8 ColorIndex);
void XOsd_LoadInstructionList(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
		u32 InstSetPtr[], u32 InstNum);

/* Version functions */
u32 XOsd_GetVersion(XOsd *InstancePtr);

/* Initialization functions in xosd_sinit.c */
XOsd_Config *XOsd_LookupConfig(u16 DeviceId);

/* Interrupt related functions in xosd_intr.c */
void XOsd_IntrHandler(void *InstancePtr);
int XOsd_SetCallBack(XOsd *InstancePtr, u32 HandlerType, void *CallBackFunc,
		void *CallBackRef);

/* SelfTest related function in xosd_selftest.c */
int XOsd_SelfTest(XOsd *InstancePtr);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
