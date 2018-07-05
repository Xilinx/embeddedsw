/******************************************************************************
*
* Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xhdmic_vsif.c
*
* Contains function definitions related to Vendor Specific InfoFrames used
* in HDMI. Please see xhdmic_vsif.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  yh     15/01/15 Initial release for 3D video support
* 1.01  YH  2017/07/19   Clean up Print Statement line ending to "\r\n"
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_hdmic_vsif.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/*************************** Function Prototypes *****************************/
static int XV_HdmiC_VSIF_Extract3DInfo(u8 *VSIFRaw, XHdmiC_3D_Info *InstancePtr);
static XHdmiC_3D_Struct_Field XV_HdmiC_VSIF_Conv3DInfoTo3DStruct(XVidC_3DInfo *InfoPtr);
static XHdmiC_3D_Sampling_Method XV_HdmiC_VSIF_Conv3DInfoTo3DSampMethod(XVidC_3DInfo *InfoPtr);
static XHdmiC_3D_Sampling_Position XV_HdmiC_VSIF_Conv3DInfoTo3DSampPos(XVidC_3DInfo *InfoPtr);
static XVidC_3DFormat XV_HdmiC_VSIF_Conv3DStructTo3DFormat(XHdmiC_3D_Struct_Field Value);
static XVidC_3DSamplingMethod XV_HdmiC_VSIF_Conv3DSampMethodTo3DSampMethod(XHdmiC_3D_Sampling_Method Value);
static XVidC_3DSamplingPosition XV_HdmiC_VSIF_Conv3DSampPosTo3DSampPos(XHdmiC_3D_Sampling_Position Value);

/************************** Variable Definitions *****************************/

/*************************** Function Definitions ****************************/

/*****************************************************************************/
/**
*
* This function extracts the 3D format from XVidC_3DInfo
* and returns a XV_HdmiC_3D_Struct_Field type.
*
* @param InfoPtr is a pointer to the XVidC_3DInfo instance.
*
* @return The extracted value.
*
******************************************************************************/
static XHdmiC_3D_Struct_Field XV_HdmiC_VSIF_Conv3DInfoTo3DStruct(XVidC_3DInfo *InfoPtr)
{
    switch(InfoPtr->Format) {
        case XVIDC_3D_FRAME_PACKING :
            return XHDMIC_3D_STRUCT_FRAME_PACKING;
            break;

        case XVIDC_3D_FIELD_ALTERNATIVE :
            return XHDMIC_3D_STRUCT_FIELD_ALTERNATIVE;
            break;

        case XVIDC_3D_LINE_ALTERNATIVE :
            return XHDMIC_3D_STRUCT_LINE_ALTERNATIVE;
            break;

        case XVIDC_3D_SIDE_BY_SIDE_FULL :
            return XHDMIC_3D_STRUCT_SIDE_BY_SIDE_FULL;
            break;

        case XVIDC_3D_TOP_AND_BOTTOM_HALF :
            return XHDMIC_3D_STRUCT_TOP_AND_BOTTOM;
            break;

        case XVIDC_3D_SIDE_BY_SIDE_HALF :
            return XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF;
            break;

        default :
            return XHDMIC_3D_STRUCT_UNKNOWN;
            break;
    }
}

/*****************************************************************************/
/**
*
* This function extracts the sampling method info from XVidC_3DInfo
* and returns a XV_HdmiC_3D_Sampling_Method type.
*
* @param InfoPtr is a pointer to the XVidC_3DInfo instance.
*
* @return The extracted value.
*
******************************************************************************/
static XHdmiC_3D_Sampling_Method XV_HdmiC_VSIF_Conv3DInfoTo3DSampMethod(XVidC_3DInfo *InfoPtr)
{
    switch(InfoPtr->Sampling.Method) {
        case XVIDC_3D_SAMPLING_HORIZONTAL :
            return XHDMIC_3D_SAMPLING_HORIZONTAL;
            break;

        case XVIDC_3D_SAMPLING_QUINCUNX :
            return XHDMIC_3D_SAMPLING_QUINCUNX;
            break;

        default :
            return XHDMIC_3D_SAMPLING_UNKNOWN;
            break;
    }
}

/*****************************************************************************/
/**
*
* This function extracts the sampling position info from XVidC_3DInfo
* and returns a XV_HdmiC_3D_Sampling_Position type.
*
* @param InfoPtr is a pointer to the XVidC_3DInfo instance.
*
* @return The extracted value.
*
******************************************************************************/
static XHdmiC_3D_Sampling_Position XV_HdmiC_VSIF_Conv3DInfoTo3DSampPos(XVidC_3DInfo *InfoPtr)
{
    switch(InfoPtr->Sampling.Position) {
        case XVIDC_3D_SAMPPOS_OLOR :
            return XHDMIC_3D_SAMPPOS_OLOR;
            break;

        case XVIDC_3D_SAMPPOS_OLER :
            return XHDMIC_3D_SAMPPOS_OLER;
            break;

        case XVIDC_3D_SAMPPOS_ELOR :
            return XHDMIC_3D_SAMPPOS_ELOR;
            break;

        case XVIDC_3D_SAMPPOS_ELER :
            return XHDMIC_3D_SAMPPOS_ELER;
            break;

        default :
            return XHDMIC_3D_SAMPPOS_UNKNOWN;
            break;
    }
}

/*****************************************************************************/
/**
*
* This function converts a XV_HdmiC_3D_Struct_Field type
* to a XVidC_3DFormat type.
*
* @param Value is the value to convert.
*
* @return The converted value.
*
******************************************************************************/
static XVidC_3DFormat XV_HdmiC_VSIF_Conv3DStructTo3DFormat(XHdmiC_3D_Struct_Field Value)
{
    switch(Value) {
        case XHDMIC_3D_STRUCT_FRAME_PACKING :
            return XVIDC_3D_FRAME_PACKING;
            break;

        case XHDMIC_3D_STRUCT_FIELD_ALTERNATIVE :
            return XVIDC_3D_FIELD_ALTERNATIVE;
            break;

        case XHDMIC_3D_STRUCT_LINE_ALTERNATIVE :
            return XVIDC_3D_LINE_ALTERNATIVE;
            break;

        case XHDMIC_3D_STRUCT_SIDE_BY_SIDE_FULL :
            return XVIDC_3D_SIDE_BY_SIDE_FULL;
            break;

        case XHDMIC_3D_STRUCT_TOP_AND_BOTTOM :
            return XVIDC_3D_TOP_AND_BOTTOM_HALF;
            break;

        case XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF :
            return XVIDC_3D_SIDE_BY_SIDE_HALF;
            break;

        default :
            return XVIDC_3D_UNKNOWN;
            break;
    }
}

/*****************************************************************************/
/**
*
* This function converts a XV_HdmiC_3D_Sampling_Method type
* to a XVidC_3DSamplingMethod type.
*
* @param Value is the value to convert.
*
* @return The converted value.
*
******************************************************************************/
static XVidC_3DSamplingMethod XV_HdmiC_VSIF_Conv3DSampMethodTo3DSampMethod(XHdmiC_3D_Sampling_Method Value)
{
    switch(Value) {
        case XHDMIC_3D_SAMPLING_HORIZONTAL :
            return XVIDC_3D_SAMPLING_HORIZONTAL;
            break;

        case XHDMIC_3D_SAMPLING_QUINCUNX :
            return XVIDC_3D_SAMPLING_QUINCUNX;
            break;

        default :
            return XVIDC_3D_SAMPLING_UNKNOWN;
            break;
    }
}

/*****************************************************************************/
/**
*
* This function converts a XV_HdmiC_3D_Sampling_Position type
* to a XVidC_3DSamplingPosition type.
*
* @param Value is the value to convert.
*
* @return The converted value.
*
******************************************************************************/
static XVidC_3DSamplingPosition XV_HdmiC_VSIF_Conv3DSampPosTo3DSampPos(XHdmiC_3D_Sampling_Position Value)
{
    switch(Value) {
        case XHDMIC_3D_SAMPPOS_OLOR :
            return XVIDC_3D_SAMPPOS_OLOR;
            break;

        case XHDMIC_3D_SAMPPOS_OLER :
            return XVIDC_3D_SAMPPOS_OLER;
            break;

        case XHDMIC_3D_SAMPPOS_ELOR :
            return XVIDC_3D_SAMPPOS_ELOR;
            break;

        case XHDMIC_3D_SAMPPOS_ELER :
            return XVIDC_3D_SAMPPOS_ELER;
            break;

        default :
            return XVIDC_3D_SAMPPOS_UNKNOWN;
            break;
    }
}

/*****************************************************************************/
/**
*
* This function parses a Vendor Specific InfoFrame (VSIF).
*
* @param AuxPtr is a pointer to the XV_HdmiTx_Rx_Aux instance.
*
* @param VSIFPtr is a pointer to the XV_HdmiC_VSIF instance.
*
* @return
*  - XST_SUCCESS if operation was successful
*  - XST_FAILURE if an error was detected during parsing
*
******************************************************************************/
int XV_HdmiC_VSIF_ParsePacket(XHdmiC_Aux *AuxPtr, XHdmiC_VSIF  *VSIFPtr)
{
    u8 *pData;
    u32 temp;
    int Index;
    int Status = XST_FAILURE;

    /* Verify arguments */
    Xil_AssertNonvoid(AuxPtr != NULL);
    Xil_AssertNonvoid(VSIFPtr != NULL);

    pData = &AuxPtr->Data.Byte[0];

    /* Clear the instance */
    (void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

    /* Set packet version */
    VSIFPtr->Version = AuxPtr->Header.Byte[1];

    /* IEEE Registration Identifier */
    for (Index = 0; Index < 3; Index++){
        temp = pData[Index+1];
        VSIFPtr->IEEE_ID |= (temp << (Index * 8));
    }

    /* HDMI Video Format */
    temp = (pData[4] & XHDMIC_VSIF_VIDEO_FORMAT_MASK) >> XHDMIC_VSIF_VIDEO_FORMAT_SHIFT;
    if (temp >= XHDMIC_VSIF_VF_UNKNOWN) {
        VSIFPtr->Format = XHDMIC_VSIF_VF_UNKNOWN;
    }
    else {
        VSIFPtr->Format = (XHdmiC_VSIF_Video_Format)temp;
    }

    switch(VSIFPtr->Format) {
        /* HDMI VIC */
        case XHDMIC_VSIF_VF_EXTRES :
            VSIFPtr->HDMI_VIC = pData[5];
            Status = XST_SUCCESS;
            break;

        /* 3D Information */
        case XHDMIC_VSIF_VF_3D :
            Status = XV_HdmiC_VSIF_Extract3DInfo(pData, &VSIFPtr->Info_3D);
            break;

        /* No additional information */
        case XHDMIC_VSIF_VF_NOINFO :
            Status = XST_SUCCESS;
            break;

        default :

            break;
    }

    return Status;
}

/*****************************************************************************/
/**
*
* This function extracts the 3D information from the
* Vendor Specific InfoFrame (VSIF).
*
* @param VSIFRaw is a pointer to the VSIF payload.
*
* @param InstancePtr is a pointer to the XV_HdmiC_3D_Info instance.
*
* @return
*  - XST_SUCCESS if operation was successful
*  - XST_FAILURE if an error was detected during parsing
*
******************************************************************************/
int XV_HdmiC_VSIF_Extract3DInfo(u8 *VSIFRaw, XHdmiC_3D_Info *InstancePtr)
{
    u8 *pData;
    u8 temp;

    /* Verify arguments */
    Xil_AssertNonvoid(VSIFRaw != NULL);
    Xil_AssertNonvoid(InstancePtr != NULL);

    /* 3D info starts at byte PB5 */
    pData = &VSIFRaw[5];

    /* Clear the instance */
    (void)memset((void *)InstancePtr, 0, sizeof(XHdmiC_3D_Info));

    /* Set default values for the items that are not always set */
    InstancePtr->Stream.Sampling.Method = XV_HdmiC_VSIF_Conv3DSampMethodTo3DSampMethod(XHDMIC_3D_SAMPLING_UNKNOWN);
    InstancePtr->Stream.Sampling.Position = XV_HdmiC_VSIF_Conv3DSampPosTo3DSampPos(XHDMIC_3D_SAMPPOS_UNKNOWN);

    /* Detect 3D Metadata presence */
    if (*pData & XHDMIC_3D_META_PRESENT_MASK)
        InstancePtr->MetaData.IsPresent = TRUE;
    else
        InstancePtr->MetaData.IsPresent = FALSE;

    /* Extract the 3D_Structure */
    temp = (*pData & XHDMIC_3D_STRUCT_MASK) >> XHDMIC_3D_STRUCT_SHIFT;
    if (temp >= XHDMIC_3D_STRUCT_UNKNOWN || temp == 7) {
        InstancePtr->Stream.Format = XV_HdmiC_VSIF_Conv3DStructTo3DFormat(XHDMIC_3D_STRUCT_UNKNOWN);
    }
    else {
        InstancePtr->Stream.Format = XV_HdmiC_VSIF_Conv3DStructTo3DFormat((XHdmiC_3D_Struct_Field)temp);
    }

    /* Extract the 3D_Ext_Data */
    if (temp >= XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF) {
        /* Go to next byte */
        pData++;

        /* Extract the sampling method */
        temp = (*pData & XHDMIC_3D_SAMP_METHOD_MASK) >> XHDMIC_3D_SAMP_METHOD_SHIFT;
        if (temp >= XHDMIC_3D_SAMPLING_UNKNOWN)
            InstancePtr->Stream.Sampling.Method = XV_HdmiC_VSIF_Conv3DSampMethodTo3DSampMethod(XHDMIC_3D_SAMPLING_UNKNOWN);
        else
            InstancePtr->Stream.Sampling.Method = XV_HdmiC_VSIF_Conv3DSampMethodTo3DSampMethod((XHdmiC_3D_Sampling_Method)temp);

        /* Extract the sampling position */
        temp = (*pData & XHDMIC_3D_SAMP_POS_MASK) >> XHDMIC_3D_SAMP_POS_SHIFT;
        if (temp >= XHDMIC_3D_SAMPPOS_UNKNOWN)
            InstancePtr->Stream.Sampling.Position = XV_HdmiC_VSIF_Conv3DSampPosTo3DSampPos(XHDMIC_3D_SAMPPOS_UNKNOWN);
        else
            InstancePtr->Stream.Sampling.Position = XV_HdmiC_VSIF_Conv3DSampPosTo3DSampPos((XHdmiC_3D_Sampling_Position)temp);
    }

    /* Extract the 3D_Metadata */
    if (InstancePtr->MetaData.IsPresent) {
        /* Go to next byte */
        pData++;

        /* Extract the 3D Metadata type */
        temp = (*pData & XHDMIC_3D_META_TYPE_MASK) >> XHDMIC_3D_META_TYPE_SHIFT;
        if (temp >= XHDMIC_3D_META_UNKNOWN)
            InstancePtr->MetaData.Type = XHDMIC_3D_META_UNKNOWN;
        else
            InstancePtr->MetaData.Type = (XHdmiC_3D_MetaData_Type)temp;

        /* Extract the 3D Metadata length */
        InstancePtr->MetaData.Length = (*pData & XHDMIC_3D_META_LENGTH_MASK) >> XHDMIC_3D_META_LENGTH_SHIFT;

        /* Extract the 3D Metadata */
        int i;
        for (i = 0; i<InstancePtr->MetaData.Length; i++){
            if (i < XHDMIC_3D_META_MAX_SIZE)
                InstancePtr->MetaData.Data[i] = *(++pData);
            else
                return XST_FAILURE;
        }
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function generates a Vendor Specific InfoFrame (VSIF).
*
* @param VSIFPtr is a pointer to the XV_HdmiC_VSIF instance.
*
* @param AuxPtr is a pointer to the XV_HdmiTx_Tx_Aux instance.
*
* @return
*  - XST_SUCCESS if operation was successful
*  - XST_FAILURE if an error was detected during generation
*
******************************************************************************/
XHdmiC_Aux XV_HdmiC_VSIF_GeneratePacket(XHdmiC_VSIF  *VSIFPtr)
{
    u8 Index = 0;
    u8 ByteCount = 0;
    u8 Crc = 0;
    XHdmiC_Aux Aux;
//    XHdmiC_Aux *AuxPtr;

    XHdmiC_3D_Struct_Field Format;
    XHdmiC_3D_Sampling_Method SampMethod;
    XHdmiC_3D_Sampling_Position SampPos;

    /* Verify arguments */
//    Xil_AssertNonvoid(VSIFPtr != NULL);
//    Xil_AssertNonvoid(AuxPtr != NULL);

    /* Header, Packet type*/
    Aux.Header.Byte[0] = AUX_VSIF_TYPE;

    /* Version */
    Aux.Header.Byte[1] = VSIFPtr->Version;

    /* Checksum (this will be calculated by the HDMI TX IP) */
    Aux.Header.Byte[3] = 0;

    /* Data */

    /* IEEE Registration ID */
    Aux.Data.Byte[++ByteCount] = VSIFPtr->IEEE_ID;
    Aux.Data.Byte[++ByteCount] = VSIFPtr->IEEE_ID >> 8;
    Aux.Data.Byte[++ByteCount] = VSIFPtr->IEEE_ID >> 16;

    Aux.Data.Byte[++ByteCount] = (VSIFPtr->Format << XHDMIC_VSIF_VIDEO_FORMAT_SHIFT) & XHDMIC_VSIF_VIDEO_FORMAT_MASK;

    switch (VSIFPtr->Format) {
        /* Extended resolution format present */
        case XHDMIC_VSIF_VF_EXTRES :
            /* HDMI_VIC */
            Aux.Data.Byte[++ByteCount] = VSIFPtr->HDMI_VIC;
            break;

        /* 3D format indication present */
        case XHDMIC_VSIF_VF_3D :
            /* 3D_Structure */
            Format = XV_HdmiC_VSIF_Conv3DInfoTo3DStruct(&VSIFPtr->Info_3D.Stream);
            Aux.Data.Byte[++ByteCount] = (Format << XHDMIC_3D_STRUCT_SHIFT) & XHDMIC_3D_STRUCT_MASK;

            /* 3D_Meta_present*/
            Aux.Data.Byte[ByteCount] |= (VSIFPtr->Info_3D.MetaData.IsPresent << XHDMIC_3D_META_PRESENT_SHIFT) & XHDMIC_3D_META_PRESENT_MASK;

            /* 3D_Ext_Data */
            if (Format >= XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF) {
                SampMethod = XV_HdmiC_VSIF_Conv3DInfoTo3DSampMethod(&VSIFPtr->Info_3D.Stream);
                Aux.Data.Byte[++ByteCount] = (SampMethod << XHDMIC_3D_SAMP_METHOD_SHIFT) & XHDMIC_3D_SAMP_METHOD_MASK;
                SampPos = XV_HdmiC_VSIF_Conv3DInfoTo3DSampPos(&VSIFPtr->Info_3D.Stream);
                Aux.Data.Byte[ByteCount] |= (SampPos << XHDMIC_3D_SAMP_POS_SHIFT) & XHDMIC_3D_SAMP_POS_MASK;
            }

            /* 3D Metadata */
            if (VSIFPtr->Info_3D.MetaData.IsPresent) {
                /* 3D_Metadata_type */
                Aux.Data.Byte[++ByteCount] = (VSIFPtr->Info_3D.MetaData.Type << XHDMIC_3D_META_TYPE_SHIFT) & XHDMIC_3D_META_TYPE_MASK;
                /* 3D_Metadata_length */
                Aux.Data.Byte[ByteCount] |= (VSIFPtr->Info_3D.MetaData.Length << XHDMIC_3D_META_LENGTH_SHIFT) & XHDMIC_3D_META_LENGTH_MASK;

                /* 3D_Metadata */
                for (Index = 0; Index < VSIFPtr->Info_3D.MetaData.Length; Index++) {
                    Aux.Data.Byte[++ByteCount] = VSIFPtr->Info_3D.MetaData.Data[Index];
                }
            }

            break;

        default :
            break;
    }

    /* Set the payload length */
    Aux.Header.Byte[2] = ByteCount;

    /* Calculate checksum */
    /* Header */
    for (Index = 0; Index < 3; Index++) {
        Crc += Aux.Header.Byte[Index];
    }

    /* Data */
    for (Index = 1; Index <= ByteCount; Index++) {
        Crc += Aux.Data.Byte[Index];
    }

    /* Set the checksum */
    Aux.Data.Byte[0] = 256 - Crc;

    return Aux;
}

/*****************************************************************************/
/**
*
* This function displays the contents of an XV_HdmiC_VSIF instance.
*
* @param VSIFPtr is a pointer to the XV_HdmiC_VSIF instance.
*
* @return None.
*
******************************************************************************/
void XV_HdmiC_VSIF_DisplayInfo(XHdmiC_VSIF  *VSIFPtr)
{
    switch (VSIFPtr->Format) {
        /* Extended resolution format present */
        case XHDMIC_VSIF_VF_EXTRES :
            /* HDMI_VIC */
            xil_printf("HDMI_VIC : %d\r\n", VSIFPtr->HDMI_VIC);
            break;

        /* 3D format indication present */
        case XHDMIC_VSIF_VF_3D :
            /* 3D_Structure */
            xil_printf("3D Format : %s\r\n", XV_HdmiC_VSIF_3DStructToString(XV_HdmiC_VSIF_Conv3DInfoTo3DStruct(&VSIFPtr->Info_3D.Stream)));

            /* 3D_Ext_Data */
            if (XV_HdmiC_VSIF_Conv3DInfoTo3DStruct(&VSIFPtr->Info_3D.Stream) >= XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF) {
                xil_printf("Sampling Method : %s\r\n", XV_HdmiC_VSIF_3DSampMethodToString(XV_HdmiC_VSIF_Conv3DInfoTo3DSampMethod(&VSIFPtr->Info_3D.Stream)));
                xil_printf("Sampling Position : %s\r\n", XV_HdmiC_VSIF_3DSampPosToString(XV_HdmiC_VSIF_Conv3DInfoTo3DSampPos(&VSIFPtr->Info_3D.Stream)));
            }

            /* 3D Metadata */
            if (VSIFPtr->Info_3D.MetaData.IsPresent) {
                /* 3D_Metadata_type */

                /* 3D_Metadata */
            }

            break;

        default :
            break;
    }
}

/*****************************************************************************/
/**
*
* This function returns a string representation of the
* enumerated type XV_HdmiC_3D_Struct_Field.
*
* @param Item specifies the value to convert.
*
* @return Pointer to the converted string.
*
******************************************************************************/
char* XV_HdmiC_VSIF_3DStructToString(XHdmiC_3D_Struct_Field Item)
{
    switch(Item) {
        case XHDMIC_3D_STRUCT_FRAME_PACKING :
            return (char*) "Frame Packing";

        case XHDMIC_3D_STRUCT_FIELD_ALTERNATIVE :
            return (char*) "Field Alternative";

        case XHDMIC_3D_STRUCT_LINE_ALTERNATIVE :
            return (char*) "Line Alternative";

        case XHDMIC_3D_STRUCT_SIDE_BY_SIDE_FULL :
            return (char*) "Side-by-Side(Full)";

        case XHDMIC_3D_STRUCT_L_DEPTH :
            return (char*) "L + Depth";

        case XHDMIC_3D_STRUCT_L_DEPTH_GRAPH_GDEPTH :
            return (char*) "L + Depth + Graphics + Graphics-depth";

        case XHDMIC_3D_STRUCT_TOP_AND_BOTTOM :
            return (char*) "Top-and-Bottom";

        case XHDMIC_3D_STRUCT_SIDE_BY_SIDE_HALF :
            return (char*) "Side-by-Side(Half)";

        default :
            return (char*) "Unknown";
    }
}

/*****************************************************************************/
/**
*
* This function returns a string representation of the
* enumerated type XV_HdmiC_3D_Sampling_Method.
*
* @param Item specifies the value to convert.
*
* @return Pointer to the converted string.
*
******************************************************************************/
char* XV_HdmiC_VSIF_3DSampMethodToString(XHdmiC_3D_Sampling_Method Item)
{
    switch(Item) {
        case XHDMIC_3D_SAMPLING_HORIZONTAL :
            return (char*) "Horizontal Sub-Sampling";

        case XHDMIC_3D_SAMPLING_QUINCUNX :
            return (char*) "Quincunx Matrix";

        default :
            return (char*) "Unknown";
    }
}

/*****************************************************************************/
/**
*
* This function returns a string representation of the
* enumerated type XV_HdmiC_3D_Sampling_Position.
*
* @param Item specifies the value to convert.
*
* @return Pointer to the converted string.
*
******************************************************************************/
char* XV_HdmiC_VSIF_3DSampPosToString(XHdmiC_3D_Sampling_Position Item)
{
    switch(Item) {
        case XHDMIC_3D_SAMPPOS_OLOR :
            return (char*) "Odd/Left, Odd/Right";

        case XHDMIC_3D_SAMPPOS_OLER :
            return (char*) "Odd/Left, Even/Right";

        case XHDMIC_3D_SAMPPOS_ELOR :
            return (char*) "Even/Left, Odd/Right";

        case XHDMIC_3D_SAMPPOS_ELER :
            return (char*) "Even/Left, Even/Right";

        default :
            return (char*) "Unknown";
    }
}
