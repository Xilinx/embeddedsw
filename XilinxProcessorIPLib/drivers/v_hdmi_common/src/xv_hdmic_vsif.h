/******************************************************************************
*
* Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xhdmic_vsif.h
*
* This is the main header file for Vendor Specific InfoFrames used in HDMI.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  yh     15/01/15 Initial release for 3D video support
* </pre>
*
******************************************************************************/

#ifndef XHDMIC_VSIF_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIC_VSIF_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xv_hdmic.h"
#include "xvidc.h"

/************************** Constant Definitions ******************************/

/** @name Vendor Specific InfoFrame Field Masks and Shifts.
 * @{
 */
#define XHDMIC_VSIF_VIDEO_FORMAT_SHIFT 5
#define XHDMIC_VSIF_VIDEO_FORMAT_MASK (0x7 << XHDMIC_VSIF_VIDEO_FORMAT_SHIFT)

#define XHDMIC_3D_STRUCT_SHIFT 4
#define XHDMIC_3D_STRUCT_MASK (0xF << XHDMIC_3D_STRUCT_SHIFT)

#define XHDMIC_3D_SAMP_METHOD_SHIFT 6
#define XHDMIC_3D_SAMP_METHOD_MASK (0x3 << XHDMIC_3D_SAMP_METHOD_SHIFT)

#define XHDMIC_3D_SAMP_POS_SHIFT 4
#define XHDMIC_3D_SAMP_POS_MASK (0x3 << XHDMIC_3D_SAMP_POS_SHIFT)

#define XHDMIC_3D_META_PRESENT_SHIFT 3
#define XHDMIC_3D_META_PRESENT_MASK (0x1 << XHDMIC_3D_META_PRESENT_SHIFT)

#define XHDMIC_3D_META_TYPE_SHIFT 5
#define XHDMIC_3D_META_TYPE_MASK (0x7 << XHDMIC_3D_META_TYPE_SHIFT)

#define XHDMIC_3D_META_LENGTH_SHIFT 0
#define XHDMIC_3D_META_LENGTH_MASK (0x1F << XHDMIC_3D_META_LENGTH_SHIFT)
/* @} */

/**************************** Type Definitions *******************************/

/**
 * HDMI Video Format
 */
typedef enum {
    XHDMIC_VSIF_VF_NOINFO = 0, /**<No additional HDMI video format is presented */
    XHDMIC_VSIF_VF_EXTRES = 1, /**<Extended resolution format present */
    XHDMIC_VSIF_VF_3D     = 2, /**<3D format indication present */
    XHDMIC_VSIF_VF_UNKNOWN
} XHdmiC_VSIF_Video_Format;

/**
 * 3D structure definitions as defined in the HDMI 1.4a specification
 */
typedef enum {
    XHDMIC_3D_STRUCT_FRAME_PACKING        = 0, /**<Frame packing */
    XHDMIC_3D_STRUCT_FIELD_ALTERNATIVE    = 1, /**<Field alternative */
    XHDMIC_3D_STRUCT_LINE_ALTERNATIVE     = 2, /**<Line alternative */
    XHDMIC_3D_STRUCT_SIDE_BY_SIDE_FULL    = 3, /**<Side-by-side (full) */
    XHDMIC_3D_STRUCT_L_DEPTH              = 4, /**<L + depth */
    XHDMIC_3D_STRUCT_L_DEPTH_GRAPH_GDEPTH = 5, /**<L + depth + graphics + graphics-depth */
    XHDMIC_3D_STRUCT_TOP_AND_BOTTOM       = 6, /**<Top-and-bottom */
    // 7 is reserved for future use
    XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF    = 8, /**<Side-by-side (half) */
    XHDMIC_3D_STRUCT_UNKNOWN
} XHdmiC_3D_Struct_Field;

/**
 * Sub-sampling methods for Side-by-side(half)
 */
typedef enum {
    XHDMIC_3D_SAMPLING_HORIZONTAL = 0, /**<Horizontal sub-sampling */
    XHDMIC_3D_SAMPLING_QUINCUNX   = 1, /**<Quincunx matrix */
    XHDMIC_3D_SAMPLING_UNKNOWN
} XHdmiC_3D_Sampling_Method;

/**
 * Sub-sampling positions for the sub-sampling methods
 */
typedef enum {
    XHDMIC_3D_SAMPPOS_OLOR = 0, /**<Odd/Left, Odd/Right */
    XHDMIC_3D_SAMPPOS_OLER = 1, /**<Odd/Left, Even/Right */
    XHDMIC_3D_SAMPPOS_ELOR = 2, /**<Even/Left, Odd/Right */
    XHDMIC_3D_SAMPPOS_ELER = 3, /**<Even/Left, Even/Right */
    XHDMIC_3D_SAMPPOS_UNKNOWN
} XHdmiC_3D_Sampling_Position;

/**
 * 3D Metadata types
 */
typedef enum {
    XHDMIC_3D_META_PARALLAX = 0, /**<Parallax information */
    XHDMIC_3D_META_UNKNOWN
} XHdmiC_3D_MetaData_Type;


// 8 is the maximum size for currently defined meta types (HDMI 1.4a)
#define XHDMIC_3D_META_MAX_SIZE 8 /**<Maximum 3D Metadata size in bytes */

/**
 * Structure for 3D meta data
 */
typedef struct {
    u8                     IsPresent;                    /**<Indicates 3D metadata presence */
    XHdmiC_3D_MetaData_Type Type;                         /**<Type */
    u8                     Length;                       /**<Length in bytes */
    u8                     Data[XHDMIC_3D_META_MAX_SIZE]; /**<Data */
} XHdmiC_3D_MetaData;

/**
 * Structure containing 3D information
 */
typedef struct {
    XVidC_3DInfo               Stream;
    XHdmiC_3D_MetaData          MetaData;   /**<3D Metadata */
} XHdmiC_3D_Info;

/**
 * Structure for holding the VSIF.
 * Format indicates the used union member.
 */
typedef struct {
    u8                      Version; /**<Version */
    u32                     IEEE_ID; /**<IEEE Registration Identifier */
    XHdmiC_VSIF_Video_Format Format;  /**<HDMI Video Format */

    union {
        u8            HDMI_VIC; /**<XHDMIC_VSIF_VF_EXTRES: HDMI Video Format Identification Code */
        XHdmiC_3D_Info Info_3D;  /**<XHDMIC_VSIF_VF_3D: 3D Information */
    };
} XHdmiC_VSIF;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Vendor Specific Infomation related functions */
int XV_HdmiC_VSIF_ParsePacket(XHdmiC_Aux *AuxPtr, XHdmiC_VSIF  *VSIFPtr);
XHdmiC_Aux XV_HdmiC_VSIF_GeneratePacket(XHdmiC_VSIF  *VSIFPtr);

void XV_HdmiC_VSIF_DisplayInfo(XHdmiC_VSIF  *VSIFPtr);
char* XV_HdmiC_VSIF_3DStructToString(XHdmiC_3D_Struct_Field Item);
char* XV_HdmiC_VSIF_3DSampMethodToString(XHdmiC_3D_Sampling_Method Item);
char* XV_HdmiC_VSIF_3DSampPosToString(XHdmiC_3D_Sampling_Position Item);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
