/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* @file xvidc_dp159.c
* @addtogroup video_common_v2_1
* @{
*
* This file contains a set of functions to configure the DP159.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ---------------------------------------------------------
* 1.00 sha 07/13/15 Initial release.
* 1.00 sha 08/11/15 Removed extra DP159 register programming as per new DP159
*                   programming guide. Added bit error count function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_printf.h"
#include "xvidc_dp159.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/* This table contains the DP159 configuration data: offset and value */
const XVidC_Dp159Data Dp159Values [] = {
       /* DP159 Initialize */
       {0xFF, 0x00},
       {0x09, 0x36},
       {0x0A, 0x7B},
       {0x0D, 0x80},
       {0x0C, 0x6D},
       {0x10, 0x00},
       {0x16, 0xF1},
       {0xFF, 0x01},
       {0x00, 0x02},
       {0x04, 0x80},
       {0x05, 0x00},
       {0x08, 0x00},
       {0x0D, 0x02},
       {0x0E, 0x03},
       {0x01, 0x01},
       {0x02, 0x3F},
       {0x0B, 0x33},
       {0xA1, 0x02},
       {0xA4, 0x02},
       {0x10, 0xF0},
       {0x11, 0x30},
       {0x14, 0x00},
       {0x12, 0x03},
       {0x13, 0xFF},
       {0x13, 0x00},
       {0x30, 0xE0},
       {0x32, 0x00},
       {0x31, 0x00},
       {0x4D, 0x08},
       {0x4C, 0x01},
       {0x34, 0x01},
       {0x32, 0xF0},
       {0x32, 0x00},
       {0x33, 0xF0},
       {0xFF, 0x00},
       {0x0A, 0x3B},
       {0xFF, 0x01}
};

/************************** Function Definitions *****************************/

#ifdef XPAR_XIIC_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function initializes the DP159.
*
* @param       InstancePtr is a pointer to the XIic instance.
*
* @return
*              - XST_SUCCESS if DP159 initialized successful.
*
* @note                None.
*
******************************************************************************/
u32 XVidC_Dp159Initialize(XIic *InstancePtr)
{
       u32 Index;

       /* Verify argument. */
       Xil_AssertNonvoid(InstancePtr != NULL);

       for (Index = 0x0; Index < 37; Index++) {
               XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                       Dp159Values[Index].Dp159Offset,
                               Dp159Values[Index].Dp159Value);
       }

       return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send on the IIC bus. This function writes the data using
* polled I/O and blocks until the data has been sent. It only supports 7-bit
* addressing.
*
* @param       InstancePtr is a pointer to the XIic instance.
* @param       IicSlaveAddr contains the 7-bit IIC address of the device to
*              write the specified data to.
* @param       Dp159RegOffset is the register offset of the DP159 register to
*              be written.
* @param       WriteVal is the 8-bit value to write into the register.
*
* @return      The number of bytes written.
*
* @note                None.
*
******************************************************************************/
u32 XVidC_Dp159Write(XIic *InstancePtr, u8 IicSlaveAddr, u8 Dp159RegOffset,
                       u8 WriteVal)
{
       u8 WriteBuf[2];
       u8 NumBytes;
       u8 Index = 0;

       /* Verify arguments. */
       Xil_AssertNonvoid(InstancePtr != NULL);
       Xil_AssertNonvoid(IicSlaveAddr != 0x0);

       WriteBuf[0] = Dp159RegOffset;
       WriteBuf[1] = WriteVal;

       /* Try twice to write data */
       do {
               NumBytes = XIic_DynSend(InstancePtr->BaseAddress, IicSlaveAddr,
                               WriteBuf, 0x2, XIIC_STOP);
               Index++;
       } while ((NumBytes != 0x2) && (Index < 0x2));

       return NumBytes;
}

/*****************************************************************************/
/**
*
* This function read data on the IIC bus. This function reads the data using
* polled I/O and block until the data has been read. It only supports 7-bit
* addressing.
*
* @param       InstancePtr is a pointer to the XIic instance.
* @param       IicSlaveAddr contains the 7-bit IIC address of the device to
*              read the specified data from.
* @param       Dp159RegOffset is the register offset of the register to be
*              read from.
* @param       ReadVal is a pointer to 8-bit data to be updated.
*
* @return      The number of bytes read.
*
* @note                None.
*
******************************************************************************/
u32 XVidC_Dp159Read(XIic *InstancePtr, u8 IicSlaveAddr, u8 Dp159RegOffset,
                       u8 *ReadVal)
{
       u8 NumBytes;

       /* Verify arguments. */
       Xil_AssertNonvoid(InstancePtr != NULL);
       Xil_AssertNonvoid(IicSlaveAddr != 0x0);
       Xil_AssertNonvoid(ReadVal != NULL);

       NumBytes = XIic_DynSend(InstancePtr->BaseAddress, IicSlaveAddr,
                               &Dp159RegOffset, 0x1, XIIC_REPEATED_START);

       NumBytes = XIic_DynRecv(InstancePtr->BaseAddress, IicSlaveAddr,
                               ReadVal, 1);

       return NumBytes;
}

/*****************************************************************************/
/**
*
* This function configures DP159 based upon configuration type.
*
* @param       InstancePtr is a pointer to the XIic instance.
* @param       ConfigType specifies the enum of configuration type to
*              configure DP159.
* @param       LinkRate is the link rate to be used over the main link based
*              on one of the following selects:
*              - XVIDC_DP159_RBR = 0x06 (for a 1.62 Gbps data rate)
*              - XVIDC_DP159_HBR = 0x0A (for a 2.70 Gbps data rate)
*              - XVIDC_DP159_HBR2 = 0x14 (for a 5.40 Gbps data rate)
* @param       LaneCount is the number of lanes to be used over the main link
*              based on one of the following selects:
*              - XVIDC_DP159_LANE_COUNT_1 = 0x1
*              - XVIDC_DP159_LANE_COUNT_2 = 0x2
*              - XVIDC_DP159_LANE_COUNT_4 = 0x4
*
* @note                None.
*
******************************************************************************/
void XVidC_Dp159Config(XIic *InstancePtr, u8 ConfigType, u8 LinkRate,
                       u8 LaneCount)
{
       u8 LRate;
       u8 LCount;
       u8 ReadBuf = 0;
       u32 Counter = 0;

       u16 Cpi;
       u16 PllCtrl;

       /* Verify arguments. */
       Xil_AssertVoid(InstancePtr != NULL);
       Xil_AssertVoid((ConfigType == XVIDC_DP159_CT_TP1) ||
                       (ConfigType == XVIDC_DP159_CT_TP2) ||
                       (ConfigType == XVIDC_DP159_CT_TP3) ||
                       (ConfigType == XVIDC_DP159_CT_UNPLUG));
       Xil_AssertVoid((LinkRate == XVIDC_DP159_RBR) ||
                       (LinkRate == XVIDC_DP159_HBR) ||
                       (LinkRate == XVIDC_DP159_HBR2));
       Xil_AssertVoid((LaneCount == XVIDC_DP159_LANE_COUNT_1) ||
                       (LaneCount == XVIDC_DP159_LANE_COUNT_2) ||
                       (LaneCount == XVIDC_DP159_LANE_COUNT_4));

       /* Configure DP159 based on config type */
       switch (ConfigType) {
               case XVIDC_DP159_CT_TP1:
                       /* Enable bandgap, DISABLE PLL, clear A_LOCK_OVR */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x00, 0x02);

                       /* CP_EN = PLL (reference) mode */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x01, 0x01);

                       /* Set PLL control */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x0B, 0x33);

                       /* Set CP_CURRENT */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x02, 0x3F);


                       LCount = (LaneCount == XVIDC_DP159_LANE_COUNT_1) ?
                               0xE1 : (LaneCount == XVIDC_DP159_LANE_COUNT_2) ?
                                       0xC3:0x0F;
                       LRate = (LinkRate == XVIDC_DP159_HBR2)? 0x0:
                               (LinkRate == XVIDC_DP159_HBR) ? 0x1 : 0x2;

                       /* Enable RX lanes */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x30, LCount);

                       /* Enable Bandgap, Enable PLL, clear A_LOCK_OVR */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x00, 0x03);

                       /* Enable fixed EQ (to reset adaptive EQ logic) */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x4C, 0x01);

                       /* Set EQFTC and EQLEV (fixed EQ) */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x4D, (LRate << 4) |
                                       (XVIDC_DP159_EQ_LEV & 0x0F));

                       /* Wait for PLL lock */
                       while ((ReadBuf == 0) && (Counter <
                                               XVIDC_DP159_LOCK_WAIT)) {
                               XVidC_Dp159Read(InstancePtr,
                                       XVIDC_DP159_IIC_SLAVE, 0x00, &ReadBuf);
                               ReadBuf = ReadBuf & 0x40;
                               Counter++;
                       }

                       if (LinkRate == XVIDC_DP159_HBR2) {
                               Cpi = XVIDC_DP159_CPI_PD_HBR2;
                               PllCtrl = XVIDC_DP159_PLL_CTRL_PD_HBR2;
                       }
                       else if (LinkRate == XVIDC_DP159_HBR) {
                               Cpi = XVIDC_DP159_CPI_PD_HBR;
                               PllCtrl = XVIDC_DP159_PLL_CTRL_PD_HBR;
                       }
                       else {
                               Cpi = XVIDC_DP159_CPI_PD_RBR;
                               PllCtrl = XVIDC_DP159_PLL_CTRL_PD_RBR;
                       }

                       /* Enable TX lanes */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x10, LCount);

                       /* Enable PLL and Bandgap, set A_LOCK_OVR, and set
                        * expand LPRES
                        */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x00, 0x23);

                       /* CP_CURRENT */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x02, Cpi);

                       /* Set PLL Control */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x0B, PllCtrl);

                       /* CP_EN is PD mode */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x01, 0x02);

                       /* Select page 0*/
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0xFF, 0x00);

                       /* Set DP_TST_EN per #lanes, latch FIFO errors */
                       LCount = (LaneCount == XVIDC_DP159_LANE_COUNT_1) ?
                                       0x11 : (LaneCount ==
                                               XVIDC_DP159_LANE_COUNT_2) ?
                                                       0x31 : 0xF1;
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x16, LCount);

                       /* Disable PV, allows char-align and 8b10 decode to
                        * operate
                        */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x10, 0x00);

                       /* Select page 1 */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0xFF, 0x01);
                       break;

               case XVIDC_DP159_CT_TP2:
               case XVIDC_DP159_CT_TP3:
                       /* Enable adaptive EQ */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x4C, 0x03);

                       /* Select page 0 */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0xFF, 0x00);

                       /* Clear BERT counters and TST_INTQ latches --
                        * Self-clearing in DP159
                        */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x15, 0x18);

                       /* Select page 1 */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0xFF, 0x01);
                       break;

               case XVIDC_DP159_CT_UNPLUG:
                       /*Enable bandgap, disable PLL, clear A_LOCK_OVR */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x00, 0x02);

                       /* Enable Offset Correction (when RX next enabled) */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x34, 0x01);

                       /* Set CP_CURRENT is high BW */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x02, 0x3F);

                       /* CP_EN  is PLL (reference) mode */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x01, 0x01);

                       /* PLL Loop filter 8K */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x0B, 0x33);

                       /* EQFTC = 1 and EQLEV = 2. eq_lev = 8, eq_lev &
                       * 0x0F
                       */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x4D, 0x08);

                       /* Enable fixed EQ (use fixed when RX disabled) */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x4C, 0x01);

                       /* Load Equalization settings */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x33, 0xF0);

                       /* Disable TX (all lanes) */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x10, 0xF0);

                       /* Enable RX Lane 0 analog only */
                       XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE,
                               0x30, 0xE0);
                       break;
       }
}

/*****************************************************************************/
/**
*
* This function resets DP159.
*
* @param       InstancePtr is a pointer to the XIic instance.
* @param       Reset specifies TRUE/FALSE flag that will be used to reset
*              DP159.
*
* @return      None.
*
* @note                Reset pin of DP159 is driven using the GPIO output of axi_iic.
*
******************************************************************************/
void XVidC_Dp159Reset(XIic *InstancePtr, u8 Reset)
{
       /* Verify arguments. */
       Xil_AssertVoid(InstancePtr != NULL);
       Xil_AssertVoid((Reset == TRUE) || (Reset == FALSE));

       /* Check reset flag */
       if (Reset) {
               XIic_WriteReg(InstancePtr->BaseAddress, 0x124, 0x0);
       }
       else {
               XIic_WriteReg(InstancePtr->BaseAddress, 0x124, 0x1);
       }
}

/*****************************************************************************/
/**
*
* This function prints the bit error encountered in DP159.
*
* @param	InstancePtr is a pointer to the XIic instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVidC_Dp159BitErrCount(XIic *InstancePtr)
{
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Select page 0 */
	XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0xFF, 0x00);

	/* Read TST_INT/Q */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x17, &Data);
	xil_printf("TST_INT/Q           : %d\n\r", Data);

	/* BERT counter0[7:0] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x18, &Data);
	xil_printf("BERT counter0[7:0]  : %d\n\r", Data);

	/* BERT counter0[11:8] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x19, &Data);
	xil_printf("BERT counter0[11:8] : %d\n\r", Data);

	/* BERT counter1[7:0] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x1A, &Data);
	xil_printf("BERT counter0[7:0]  : %d\n\r", Data);

	/* BERT counter1[11:8] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x1B, &Data);
	xil_printf("BERT counter0[11:8] : %d\n\r", Data);

	/* BERT counter2[7:0] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x1C, &Data);
	xil_printf("BERT counter2[7:0]  : %d\n\r", Data);

	/* BERT counter2[11:8] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x1D, &Data);
	xil_printf("BERT counter2[11:8] : %d\n\r", Data);

	/* BERT counter3[7:0] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x1E, &Data);
	xil_printf("BERT counter3[7:0]  : %d\n\r", Data);

	/* BERT counter3[11:8] */
	XVidC_Dp159Read(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x1F, &Data);
	xil_printf("BERT counter3[11:8] : %d\n\r", Data);

	/* Clear BERT counters and TST_INTQ latches - Self-clearing in DP159 */
	XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0x15, 0x18);

	/* Select page 1 */
	XVidC_Dp159Write(InstancePtr, XVIDC_DP159_IIC_SLAVE, 0xFF, 0x01);
}
#endif /* End of XPAR_XIIC_NUM_INSTANCES */
/** @} */
