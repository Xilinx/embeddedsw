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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* This is main header file of the Xilinx MVI On-Screen-Display (OSD) device
* driver.
*
* An OSD is an image superimposed on a screen picture, commonly used by modern
* televisions, VCRs, and DVD players to display information such as volume,
* channel, and time.
*
* Xilinx MVI OSD has the following main features:
*
* - Read Video Data one of three sources
*	- VFBC/Frame Buffer
*	- VideoBus
*	- Graphics Controller
* - Alpha Compositing and Alpha Blending of up to 8 layers
* - Up to 8 priorities, one for each of the layers
* - Real-Time Graphics Controller
* - Write Composited Video Data to either
*	- VFBC/Frame Buffer, or
*	- VideoBus
*
* For a full description of OSD features, please see the hardware spec.
*
* <b>Interrupt Service </b>
*
* Four interrupt types are supported:
*
* - Vertical Blank Interval Start Interrupt
* - Vertical Blank Interval End Interrupt
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
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   08/18/08 First release
* 1.01a xd   07/30/10 Added device version support; Supported doxygen; Fixed
*                     CR #534952
* 1.02a xd   12/21/10 Removed endian conversion for text bank loading
* 1.03a cm   09/07/11 Updated XOSD_GetLayerAlpha(), XOSD_SetLayerAlpha(), 
*                     XOSD_SetBackgroundColor() and XOSD_GetBackgroundColor() 
*                     to allow 10 and 12 bit alpha and background colors. 
* 2.00a cjm  12/18/12 Converted from xio.h to xil_io.h, translating
*                     basic types, MB cache functions, exceptions and
*                     assertions to xil_io format. 
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
*                     
* </pre>
*
******************************************************************************/

#ifndef XOSD_H		 /* prevent circular inclusions */
#define XOSD_H		 /* by using protection macros */

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
#define XOSD_HANDLER_FRAMEDONE	1	 /**< Frame Done interrupt */
#define XOSD_HANDLER_VBISTART	2	 /**< Vertical Blank Interval Start
					   *  interrupt */
#define XOSD_HANDLER_VBIEND	3	 /**< Vertical Blank Interval End
					   *  interrupt */
#define XOSD_HANDLER_ERROR	4	 /**< Error condition interrupt */
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * OSD device configuration structure.
 * Each OSD device should have a configuration structure associated
 */
typedef struct {
	u16 DeviceId;	   /**< DeviceId is the unique ID  of the device */
	u32 BaseAddress;   /**< BaseAddress is the physical base address of
			     *  the device's registers */
	u16 LayerNum;	   /**< The number of Layers */
  u16 S_AXIS_VIDEO_DATA_WIDTH;
	u16 Layer0Type;	   /**< Type of Layer #0 */
	u16 Layer1Type;	   /**< Type of Layer #1 */
	u16 Layer2Type;	   /**< Type of Layer #2 */
	u16 Layer3Type;	   /**< Type of Layer #3 */
	u16 Layer4Type;	   /**< Type of Layer #4 */
	u16 Layer5Type;	   /**< Type of Layer #5 */
	u16 Layer6Type;	   /**< Type of Layer #6 */
	u16 Layer7Type;	   /**< Type of Layer #7 */

	/**< Layer 0 */
	u16 Layer0InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer0InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer0InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer0InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer0ColorLutSize;		  /**< Color Lut Size */
	u16 Layer0ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer0FontNumChars;		  /**< Font: Number of characters */
	u16 Layer0FontWidth;		  /**< Font: Width */
	u16 Layer0FontHeight;		  /**< Font: Height */
	u16 Layer0FontBitsPerPixel;	  /**< Font: Number of bits per pixel
					    */
	u16 Layer0FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer0TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer0TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 1 */
	u16 Layer1InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer1InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer1InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer1InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer1ColorLutSize;		  /**< Color Lut Size */
	u16 Layer1ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer1FontNumChars;		  /**< Font: Number of characters */
	u16 Layer1FontWidth;		  /**< Font: Width */
	u16 Layer1FontHeight;		  /**< Font: Height */
	u16 Layer1FontBitsPerPixel;	  /**< Font: Number of bits per pixel
					    */
	u16 Layer1FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer1TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer1TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 2 */
	u16 Layer2InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer2InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer2InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer2InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer2ColorLutSize;		  /**< Color Lut Size */
	u16 Layer2ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer2FontNumChars;		  /**< Font: Number of characters */
	u16 Layer2FontWidth;		  /**< Font: Width */
	u16 Layer2FontHeight;		  /**< Font: Height */
	u16 Layer2FontBitsPerPixel;	  /**< Font: Number of bits per
					    *  pixel */
	u16 Layer2FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer2TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer2TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 3 */
	u16 Layer3InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer3InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer3InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer3InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer3ColorLutSize;		  /**< Color Lut Size */
	u16 Layer3ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer3FontNumChars;		  /**< Font: Number of characters */
	u16 Layer3FontWidth;		  /**< Font: Width */
	u16 Layer3FontHeight;		  /**< Font: Height */
	u16 Layer3FontBitsPerPixel;	  /**< Font: Number of bits per pixel
					    */
	u16 Layer3FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer3TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer3TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 4 */
	u16 Layer4InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer4InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer4InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer4InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer4ColorLutSize;		  /**< Color Lut Size */
	u16 Layer4ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer4FontNumChars;		  /**< Font: Number of characters */
	u16 Layer4FontWidth;		  /**< Font: Width */
	u16 Layer4FontHeight;		  /**< Font: Height */
	u16 Layer4FontBitsPerPixel;	  /**< Font: Number of bits per
					    *  pixel */
	u16 Layer4FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer4TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer4TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 5 */
	u16 Layer5InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer5InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer5InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer5InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer5ColorLutSize;		  /**< Color Lut Size */
	u16 Layer5ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer5FontNumChars;		  /**< Font: Number of characters */
	u16 Layer5FontWidth;		  /**< Font: Width */
	u16 Layer5FontHeight;		  /**< Font: Height */
	u16 Layer5FontBitsPerPixel;	  /**< Font: Number of bits per
					    *  pixel */
	u16 Layer5FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer5TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer5TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 6 */
	u16 Layer6InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer6InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer6InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer6InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer6ColorLutSize;		  /**< Color Lut Size */
	u16 Layer6ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer6FontNumChars;		  /**< Font: Number of characters */
	u16 Layer6FontWidth;		  /**< Font: Width */
	u16 Layer6FontHeight;		  /**< Font: Height */
	u16 Layer6FontBitsPerPixel;	  /**< Font: Number of bits per
					    *  pixel */
	u16 Layer6FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer6TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer6TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

	/**< Layer 7 */
	u16 Layer7InstructionMemSize;	  /**< Instruction Memory Size */
	u16 Layer7InstructionBoxEnable;	  /**< Instruction Box Enable */
	u16 Layer7InstructionLineEnable;  /**< Instruction Line Enable */
	u16 Layer7InstructionTextEnable;  /**< Instruction Text Enable */
	u16 Layer7ColorLutSize;		  /**< Color Lut Size */
	u16 Layer7ColorLutMemoryType;	  /**< Color Lut Memory Type */
	u16 Layer7FontNumChars;		  /**< Font: Number of characters */
	u16 Layer7FontWidth;		  /**< Font: Width */
	u16 Layer7FontHeight;		  /**< Font: Height */
	u16 Layer7FontBitsPerPixel;	  /**< Font: Number of bits per
					    *  pixel */
	u16 Layer7FontAsciiOffset;	  /**< Font: ASCII offset of 1st
					    *  character */
	u16 Layer7TextNumStrings;	  /**< Text: Number of Strings */
	u16 Layer7TextMaxStringLength;	  /**< Text: Maximum length of a
					    *  String */

} XOSD_Config;

/**
 * The XOSD Layer info structure
 */
typedef struct {
	u16 LayerType;			/**< Type of the layer */
	u16 InstructionNum;		/**< The Number of Instructions */
	u16 InstructionBoxEnable;	/**< Instruction Box Enable */
	u16 InstructionLineEnable;	/**< Instruction Line Enable */
	u16 InstructionTextEnable;	/**< Instruction Text Enable */
	u16 ColorLutSize;		/**< Color Lut Size */
	u16 ColorLutMemoryType;		/**< Color Lut Memory Type */
	u16 FontNumChars;		/**< Font: Number of characters */
	u16 FontWidth;			/**< Font: Width */
	u16 FontHeight;			/**< Font: Height */
	u16 FontBitsPerPixel;		/**< Font: Number of bits per pixel */
	u16 FontAsciiOffset;		/**< Font: ASCII offset of 1st
					  *  character */
	u16 TextNumStrings;		/**< Text: Number of Strings */
	u16 TextMaxStringLength;	/**< Text: Maximum length of a
					  *  String */

} XOSD_Layer;

/**
 * Callback type for all interrupts except error interrupt.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the
 *	  upper layer when the callback is invoked.
 */
typedef void (*XOSD_CallBack) (void *CallBackRef);

/**
 * Callback type for Error interrupt.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *	  when setting the callback functions, and passed back to the
 *	  upper layer when the callback is invoked.
 * @param ErrorMask is a bit mask indicating the cause of the error. Its
 *	  value equals 'OR'ing one or more XOSD_IXR_* values defined in
 *	  xosd_hw.h
 */
typedef void (*XOSD_ErrorCallBack) (void *CallBackRef, u32 ErrorMask);

/**
 * The XOSD driver instance data. An instance must be allocated for each
 * OSD device in use.
 */
typedef struct {
	XOSD_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
				  *  initialized */
	int InstructionInExternalMem;	/**< Flag indicating if the instruction
					     list is from external memory */
	u32 ScreenHeight;		/**< Screen Height of the OSD output */
	u32 ScreenWidth;		/**< Screen Width of the OSD output */

	XOSD_Layer Layers[XOSD_MAX_NUM_OF_LAYERS]; /**< Properties of layers */

	XOSD_CallBack VbiStartCallBack; /**< Call back for Vertical Blank
					  *  Interval (VBI) Start interrupt */
	void *VbiStartRef;		/**< To be passed to the VBI Start
					  *  interrupt callback */

	XOSD_CallBack VbiEndCallBack;	/**< Call back for Vertical Blank
					  *  Interval (VBI) End interrupt */
	void *VbiEndRef;		/**< To be passed to the VBI End
					  *  interrupt callback */

	XOSD_CallBack FrameDoneCallBack;/**< Call back for Frame Done interrupt
					  */
	void *FrameDoneRef;		/**< To be passed to the Frame Done
					  *  interrupt callback */

	XOSD_ErrorCallBack ErrCallBack; /**< Call back for Error interrupt */
	void *ErrRef;			/**< To be passed to the Error
					  *  interrupt callback */

} XOSD;

/**
 * The XOSD_Instruction data structure for holding one GC instruction.
 */
typedef u32 XOSD_Instruction[XOSD_INS_SIZE];

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_Enable(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_Enable(InstancePtr) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_CTL, \
		XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_CTL) | \
			XOSD_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_Disable(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_Disable(InstancePtr) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_CTL, \
		XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_CTL) & \
			(~XOSD_CTL_EN_MASK))

/*****************************************************************************/
/**
*
* This macro tell an OSD device to pick up the register updates.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_RegUpdateEnable(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_RegUpdateEnable(InstancePtr) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_CTL, \
		XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_CTL) | \
			XOSD_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro tell an OSD device to ignore the register updates.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_RegUpdateDisable(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_RegUpdateDisable(InstancePtr) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_CTL, \
		XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_CTL) & \
			(~XOSD_CTL_RUE_MASK))

/*****************************************************************************/
/**
*
* This macro resets an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_Reset(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_Reset(InstancePtr) \
{ \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_RST, \
			  XOSD_RST_RESET); \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_CTL, 0); \
	(InstancePtr)->InstructionInExternalMem = 0; \
	(InstancePtr)->ScreenHeight = 0; \
	(InstancePtr)->ScreenWidth = 0; \
}

/*****************************************************************************/
/**
*
* This macro enables the global interrupt on an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_IntrEnableGlobal(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_IntrEnableGlobal(InstancePtr) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_GIER, \
			  XOSD_GIER_GIE_MASK)

/*****************************************************************************/
/**
*
* This macro disables the global interrupt on an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XOSD_IntrDisableGlobal(XOSD *InstancePtr);
*
******************************************************************************/
#define XOSD_IntrDisableGlobal(InstancePtr) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_GIER, 0)

/*****************************************************************************/
/**
*
* This macro enables the given individual interrupt(s) on an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @param  IntrType is the type of the interrupts to enable. Use OR'ing of
*	  XOSD_IXR_* constants defined in xosd_hw.h to create this parameter
*	  value.
*
* @return None
*
* @note
*
* The existing enabled interrupt(s) will remain enabled.
*
* C-style signature:
*	 void XOSD_IntrEnable(XOSD *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XOSD_IntrEnable(InstancePtr, IntrType) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_IER, \
		((IntrType) & XOSD_IXR_ALLINTR_MASK) | \
		XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_IER))

/*****************************************************************************/
/**
*
* This macro disables the given individual interrupt(s) on an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @param  IntrType is the type of the interrupts to disable. Use OR'ing of
*	  XOSD_IXR_* constants defined in xosd_hw.h to create this parameter
*	  value.
*
* @return None
*
* @note
*
* Any other interrupt not covered by parameter IntrType, if enabled before
* this macro is called, will remain enabled.
*
* C-style signature:
*	 void XOSD_IntrDisable(XOSD *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XOSD_IntrDisable(InstancePtr, IntrType) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_IER, \
		((~(IntrType)) & XOSD_IXR_ALLINTR_MASK) & \
		XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_IER))

/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @return The pending interrupts of the OSD. Use XOSD_IXR_* constants
*	  defined in xosd_hw.h to interpret this value.
*
* @note
*
* C-style signature:
*	 u32 XOSD_IntrGetPending(XOSD *InstancePtr)
*
******************************************************************************/
#define XOSD_IntrGetPending(InstancePtr) \
	(XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_IER) & \
	XOSD_ReadReg((InstancePtr)->Config.BaseAddress, XOSD_ISR) & \
	XOSD_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of an OSD device.
*
* @param  InstancePtr is a pointer to the OSD device instance to be worked on.
*
* @param  IntrType is the pending interrupts to clear/acknowledge. Use OR'ing
*	  of XOSD_IXR_* constants defined in xosd_hw.h to create this
*	  parameter value.
*
* @return None
*
* @note
*
* C-style signature:
*	 void XOSD_IntrClear(XOSD *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XOSD_IntrClear(InstancePtr, IntrType) \
	XOSD_WriteReg((InstancePtr)->Config.BaseAddress, XOSD_ISR, \
			  (IntrType) & XOSD_IXR_ALLINTR_MASK)

/************************** Function Prototypes ******************************/

/*
 * Initialization and control functions in xosd.c
 */

/* Initialization */
int XOSD_CfgInitialize(XOSD *InstancePtr, XOSD_Config *CfgPtr,
			   u32 EffectiveAddr);

/* Set Vertical and Horizontal Blank Input Polarity Types */
void XOSD_SetBlankPolarity(XOSD *InstancePtr, int VerticalBlankPolarity,
			   int HorizontalBlankPolarity);

/* Set/Get Screen Size of the OSD Output */
void XOSD_SetScreenSize(XOSD *InstancePtr, u32 Width, u32 Height);
void XOSD_GetScreenSize(XOSD *InstancePtr, u32 *WidthPtr, u32 *HeightPtr);

/* Set/Get Background color */
void XOSD_SetBackgroundColor(XOSD *InstancePtr, u16 Red, u16 Blue, u16 Green);
void XOSD_GetBackgroundColor(XOSD *InstancePtr, u16 *RedPtr, u16 *BluePtr,
				 u16 *GreenPtr);

/* Layer related functions */
void XOSD_SetLayerDimension(XOSD *InstancePtr, u8 LayerIndex, u16 XStart,
				u16 YStart, u16 XSize, u16 YSize);
void XOSD_GetLayerDimension(XOSD *InstancePtr, u8 LayerIndex, u16 *XStartPtr,
				u16 *YStartPtr, u16 *XSizePtr, u16 *YSizePtr);

void XOSD_SetLayerAlpha(XOSD *InstancePtr, u8 LayerIndex,
			u16 GlobalAlphaEnble, u16 GlobalAlphaValue);
void XOSD_GetLayerAlpha(XOSD *InstancePtr, u8 LayerIndex,
				u16 *GlobalAlphaEnblePtr,
				u16 *GlobalAlphaValuePtr);

void XOSD_SetLayerPriority(XOSD *InstancePtr, u8 LayerIndex, u8 Priority);
void XOSD_GetLayerPriority(XOSD *InstancePtr, u8 LayerIndex, u8 *PriorityPtr);

void XOSD_EnableLayer(XOSD *InstancePtr, u8 LayerIndex);
void XOSD_DisableLayer(XOSD *InstancePtr, u8 LayerIndex);

/* Graphics Controller related functions */
void XOSD_LoadColorLUTBank(XOSD *InstancePtr, u8 GcIndex, u8 BankIndex,
			   u32 *ColorData);
void XOSD_LoadCharacterSetBank(XOSD *InstancePtr, u8 GcIndex, u8 BankIndex,
				   u32 *CharSetData);
void XOSD_LoadTextBank(XOSD *InstancePtr, u8 GcIndex, u8 BankIndex,
				   u32 *TextData);
void XOSD_SetActiveBank(XOSD *InstancePtr, u8 GcIndex, u8 ColorBankIndex,
				u8 CharBankIndex, u8 TextBankIndex,
				u8 InstructionBankIndex);

/* Create and load instruction(s) */
void XOSD_CreateInstruction(XOSD *InstancePtr, u32 *InstructionPtr,
				u8 GcIndex, u16 ObjType, u8 ObjSize,
				u16 XStart, u16 YStart, u16 XEnd, u16 YEnd,
				u8 TextIndex, u8 ColorIndex);
void XOSD_LoadInstructionList(XOSD *InstancePtr, u8 GcIndex, u8 BankIndex,
				u32 *InstSetPtr, u32 InstNum);

/* Version functions */
void XOSD_GetVersion(XOSD *InstancePtr, u16 *Major, u16 *Minor, u16 *Revision);

/*
 * Initialization functions in xosd_sinit.c
 */
XOSD_Config *XOSD_LookupConfig(u16 DeviceId);

/*
 * Interrupt related functions in xosd_intr.c
 */
void XOSD_IntrHandler(void *InstancePtr);
int XOSD_SetCallBack(XOSD *InstancePtr, u32 IntrType,
			 void *CallBackFunc, void *CallBackRef);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
