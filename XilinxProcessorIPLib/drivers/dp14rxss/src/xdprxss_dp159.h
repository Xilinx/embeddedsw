/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xdprxss_dp159.h
* @addtogroup dprxss_v4_2
* @{
*
* This is the header file contains macros, enum, structure and function
* prototypes for DP159.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 07/13/15 Initial release.
* 1.00 sha 08/11/15 Modified XDPRXSS_DP159_CT_PWR -> XDPRXSS_DP159_CT_UNPLUG.
*                   Added bit error count function.
* 2.2  als 02/01/16 Functions with pointer arguments that don't modify
*                   contents now const.
* 3.1  aad 06/16/16 Updated CPI pull down HBR2 value
* 4.0  aad 11/15/16 Moved to dprxss form video_common
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_DP159_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPRXSS_DP159_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xvidc.h"
#include "xparameters.h"

#ifdef XPAR_XIIC_NUM_INSTANCES
#include "xiic.h"
#endif /* End of XPAR_XIIC_NUM_INSTANCES */

/************************** Constant Definitions *****************************/

#define XDPRXSS_DP159_IIC_SLAVE          0x5E    /**< DP159 slave device
                                                 *  address*/

#define XDPRXSS_DP159_RBR                        0x06    /**< 1.62 Gbps link rate */
#define XDPRXSS_DP159_HBR                        0x0A    /**< 2.70 Gbps link rate */
#define XDPRXSS_DP159_HBR2               0x14    /**< 5.40 Gbps link rate */

#define XDPRXSS_DP159_LANE_COUNT_1       1       /**< Lane count of 1 */
#define XDPRXSS_DP159_LANE_COUNT_2       2       /**< Lane count of 2 */
#define XDPRXSS_DP159_LANE_COUNT_4       4       /**< Lane count of 4 */

#define XDPRXSS_DP159_CPI_PD_RBR         0x1F    /**< CPI pull down RBR */
#define XDPRXSS_DP159_CPI_PD_HBR         0x27    /**< CPI pull down HBR */
#define XDPRXSS_DP159_CPI_PD_HBR2        0x5F    /**< CPI pull down HBR2 */

#define XDPRXSS_DP159_PLL_CTRL_PD_RBR    0x30    /**< PLL control pull down
                                                 *  RBR */
#define XDPRXSS_DP159_PLL_CTRL_PD_HBR    0x30    /**< PLL control pull down
                                                 *  HBR */
#define XDPRXSS_DP159_PLL_CTRL_PD_HBR2   0x30    /**< PLL control pull down
                                                 *  HBR2 */

#define XDPRXSS_DP159_EQ_LEV             8       /**< Equivalisation level */
#define XDPRXSS_DP159_LOCK_WAIT          512     /**< Lock wait value */

/**************************** Type Definitions *******************************/

/**
 * This typedef enumerates the types of configuration types applied to DP159
 * to configure.
 */
typedef enum {
       XDPRXSS_DP159_CT_TP1 = 1, /**< DP159 training pattern 1 */
       XDPRXSS_DP159_CT_TP2,     /**< DP159 training pattern 2 */
       XDPRXSS_DP159_CT_TP3,     /**< DP159 training pattern 1 */
       XDPRXSS_DP159_CT_UNPLUG   /**< DP159 unplug */
} XDpRxSs_Dp159ConfigType;

/**
 * DP159 data structure.
 */
typedef struct {
       u8 Dp159Offset;         /**< DP159 offset */
       u8 Dp159Value;          /**< DP159 value */
} XDpRxSs_Dp159Data;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/* Protecting following functions to avoid popping compilation errors if
 * system does not have IIC instance(s) as video common library is common.
 */
#ifdef XPAR_XIIC_NUM_INSTANCES
u32 XDpRxSs_Dp159Initialize(const XIic *InstancePtr);
u32 XDpRxSs_Dp159Write(const XIic *InstancePtr, u8 IicSlaveAddr,
		u8 Dp159RegOffset, u8 WriteVal);
u32 XDpRxSs_Dp159Read(const XIic *InstancePtr, u8 IicSlaveAddr,
		u8 Dp159RegOffset, u8 *ReadVal);
void XDpRxSs_Dp159Config(const XIic *InstancePtr, u8 ConfigType, u8 LinkRate,
		u8 LaneCount);
void XDpRxSs_Dp159Reset(const XIic *InstancePtr, u8 Reset);
void XDpRxSs_Dp159BitErrCount(const XIic *InstancePtr);
#endif /* End of XPAR_XIIC_NUM_INSTANCES */

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif
#endif /* End of protection macro */
/** @} */
