/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xdprxss_dp159.c
* @addtogroup dprxss_v4_2
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
* 2.2  als 02/01/16 Functions with pointer arguments that don't modify
*                   contents now const.
* 3.0  aad 05/13/16 Added bus busy check before I2C reads and writes.
* 3.1  als 08/03/16 Reordered wait for PLL lock.
* 4.0  aad 11/15/16 Moved to dprxss driver from video_common
* 4.0  aad 07/13/17 Updated DP159 read lock status
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_printf.h"
#include "xdprxss_dp159.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/* This table contains the DP159 configuration data: offset and value */
const XDpRxSs_Dp159Data Dp159Values [] = {
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
u32 XDpRxSs_Dp159Initialize(const XIic *InstancePtr)
{
       u32 Index;

       /* Verify argument. */
       Xil_AssertNonvoid(InstancePtr != NULL);

       for (Index = 0x0; Index < 37; Index++) {
               XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
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
u32 XDpRxSs_Dp159Write(const XIic *InstancePtr, u8 IicSlaveAddr,
		u8 Dp159RegOffset, u8 WriteVal)
{
       u8 WriteBuf[2];
       u8 NumBytes;
       u8 Index = 0;
       u32 TimeoutCount = 0;
       u32 StatusRegister;

       /* Verify arguments. */
       Xil_AssertNonvoid(InstancePtr != NULL);
       Xil_AssertNonvoid(IicSlaveAddr != 0x0);

       do {
	      StatusRegister = XIic_ReadReg(InstancePtr->BaseAddress,
					XIIC_SR_REG_OFFSET);
	      TimeoutCount++;
       } while ((StatusRegister & XIIC_SR_BUS_BUSY_MASK) &&
		     (TimeoutCount < 1000));

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
u32 XDpRxSs_Dp159Read(const XIic *InstancePtr, u8 IicSlaveAddr, u8 Dp159RegOffset,
                       u8 *ReadVal)
{
       u8 NumBytes;
       u32 StatusRegister;
       u32 TimeoutCount = 0;

       /* Verify arguments. */
       Xil_AssertNonvoid(InstancePtr != NULL);
       Xil_AssertNonvoid(IicSlaveAddr != 0x0);
       Xil_AssertNonvoid(ReadVal != NULL);

       do {
	      StatusRegister = XIic_ReadReg(InstancePtr->BaseAddress,
					XIIC_SR_REG_OFFSET);
	      TimeoutCount++;
       } while ((StatusRegister & XIIC_SR_BUS_BUSY_MASK) &&
		     (TimeoutCount < 1000));

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
*              - XDPRXSS_DP159_RBR = 0x06 (for a 1.62 Gbps data rate)
*              - XDPRXSS_DP159_HBR = 0x0A (for a 2.70 Gbps data rate)
*              - XDPRXSS_DP159_HBR2 = 0x14 (for a 5.40 Gbps data rate)
* @param       LaneCount is the number of lanes to be used over the main link
*              based on one of the following selects:
*              - XDPRXSS_DP159_LANE_COUNT_1 = 0x1
*              - XDPRXSS_DP159_LANE_COUNT_2 = 0x2
*              - XDPRXSS_DP159_LANE_COUNT_4 = 0x4
*
* @note                None.
*
******************************************************************************/
void XDpRxSs_Dp159Config(const XIic *InstancePtr, u8 ConfigType, u8 LinkRate,
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
       Xil_AssertVoid((ConfigType == XDPRXSS_DP159_CT_TP1) ||
                       (ConfigType == XDPRXSS_DP159_CT_TP2) ||
                       (ConfigType == XDPRXSS_DP159_CT_TP3) ||
                       (ConfigType == XDPRXSS_DP159_CT_UNPLUG));
       Xil_AssertVoid((LinkRate == XDPRXSS_DP159_RBR) ||
                       (LinkRate == XDPRXSS_DP159_HBR) ||
                       (LinkRate == XDPRXSS_DP159_HBR2));
       Xil_AssertVoid((LaneCount == XDPRXSS_DP159_LANE_COUNT_1) ||
                       (LaneCount == XDPRXSS_DP159_LANE_COUNT_2) ||
                       (LaneCount == XDPRXSS_DP159_LANE_COUNT_4));

       /* Configure DP159 based on config type */
       switch (ConfigType) {
               case XDPRXSS_DP159_CT_TP1:
                       /* Enable bandgap, DISABLE PLL, clear A_LOCK_OVR */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x00, 0x02);

                       /* CP_EN = PLL (reference) mode */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x01, 0x01);

                       /* Set PLL control */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x0B, 0x33);

                       /* Set CP_CURRENT */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x02, 0x3F);


                       LCount = (LaneCount == XDPRXSS_DP159_LANE_COUNT_1) ?
                               0xE1 : (LaneCount == XDPRXSS_DP159_LANE_COUNT_2) ?
                                       0xC3:0x0F;
                       LRate = (LinkRate == XDPRXSS_DP159_HBR2)? 0x0:
                               (LinkRate == XDPRXSS_DP159_HBR) ? 0x1 : 0x2;

                       /* Enable RX lanes */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x30, LCount);

                       /* Enable Bandgap, Enable PLL, clear A_LOCK_OVR */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x00, 0x03);

                       /* Enable fixed EQ (to reset adaptive EQ logic) */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x4C, 0x01);

                       /* Set EQFTC and EQLEV (fixed EQ) */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x4D, (LRate << 4) |
                                       (XDPRXSS_DP159_EQ_LEV & 0x0F));

                       if (LinkRate == XDPRXSS_DP159_HBR2) {
                               Cpi = XDPRXSS_DP159_CPI_PD_HBR2;
                               PllCtrl = XDPRXSS_DP159_PLL_CTRL_PD_HBR2;
                       }
                       else if (LinkRate == XDPRXSS_DP159_HBR) {
                               Cpi = XDPRXSS_DP159_CPI_PD_HBR;
                               PllCtrl = XDPRXSS_DP159_PLL_CTRL_PD_HBR;
                       }
                       else {
                               Cpi = XDPRXSS_DP159_CPI_PD_RBR;
                               PllCtrl = XDPRXSS_DP159_PLL_CTRL_PD_RBR;
                       }

                       /* Enable TX lanes */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x10, LCount);

                       /* Enable PLL and Bandgap, set A_LOCK_OVR, and set
                        * expand LPRES
                        */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x00, 0x23);

                       /* Wait for PLL lock */
                       while ((ReadBuf == 0) && (Counter <
                                               XDPRXSS_DP159_LOCK_WAIT)) {
                               XDpRxSs_Dp159Read(InstancePtr,
					XDPRXSS_DP159_IIC_SLAVE, 0x00, &ReadBuf);
                               ReadBuf &= 0x40; /* Lock status. */
                               Counter++;
                       }

                       /* CP_CURRENT */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x02, Cpi);

                       /* Set PLL Control */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x0B, PllCtrl);

                       /* CP_EN is PD mode */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x01, 0x02);

                       /* Select page 0*/
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0xFF, 0x00);

                       /* Set DP_TST_EN per #lanes, latch FIFO errors */
                       LCount = (LaneCount == XDPRXSS_DP159_LANE_COUNT_1) ?
                                       0x11 : (LaneCount ==
                                               XDPRXSS_DP159_LANE_COUNT_2) ?
                                                       0x31 : 0xF1;
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x16, LCount);

                       /* Disable PV, allows char-align and 8b10 decode to
                        * operate
                        */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x10, 0x00);

                       /* Select page 1 */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0xFF, 0x01);
                       break;

               case XDPRXSS_DP159_CT_TP2:
               case XDPRXSS_DP159_CT_TP3:
                       /* Enable adaptive EQ */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x4C, 0x03);

                       /* Select page 0 */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0xFF, 0x00);

                       /* Clear BERT counters and TST_INTQ latches --
                        * Self-clearing in DP159
                        */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x15, 0x18);

                       /* Select page 1 */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0xFF, 0x01);
                       break;

               case XDPRXSS_DP159_CT_UNPLUG:
                       /*Enable bandgap, disable PLL, clear A_LOCK_OVR */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x00, 0x02);

                       /* Enable Offset Correction (when RX next enabled) */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x34, 0x01);

                       /* Set CP_CURRENT is high BW */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x02, 0x3F);

                       /* CP_EN  is PLL (reference) mode */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x01, 0x01);

                       /* PLL Loop filter 8K */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x0B, 0x33);

                       /* EQFTC = 1 and EQLEV = 2. eq_lev = 8, eq_lev &
                       * 0x0F
                       */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x4D, 0x08);

                       /* Enable fixed EQ (use fixed when RX disabled) */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x4C, 0x01);

                       /* Load Equalization settings */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x33, 0xF0);

                       /* Disable TX (all lanes) */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
                               0x10, 0xF0);

                       /* Enable RX Lane 0 analog only */
                       XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE,
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
void XDpRxSs_Dp159Reset(const XIic *InstancePtr, u8 Reset)
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
void XDpRxSs_Dp159BitErrCount(const XIic *InstancePtr)
{
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Select page 1 */
	XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0xFF, 0x01);

	/* Read LOCK_STATUS */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x00, &Data);
	xil_printf("LOCK_STATUS         : %d\n\r", Data & 0x40);

	/* Select page 0 */
	XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0xFF, 0x00);

	/* Read TST_INT/Q */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x17, &Data);
	xil_printf("TST_INT/Q           : %d\n\r", Data);

	/* BERT counter0[7:0] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x18, &Data);
	xil_printf("BERT counter0[7:0]  : %d\n\r", Data);

	/* BERT counter0[11:8] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x19, &Data);
	xil_printf("BERT counter0[11:8] : %d\n\r", Data);

	/* BERT counter1[7:0] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x1A, &Data);
	xil_printf("BERT counter0[7:0]  : %d\n\r", Data);

	/* BERT counter1[11:8] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x1B, &Data);
	xil_printf("BERT counter0[11:8] : %d\n\r", Data);

	/* BERT counter2[7:0] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x1C, &Data);
	xil_printf("BERT counter2[7:0]  : %d\n\r", Data);

	/* BERT counter2[11:8] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x1D, &Data);
	xil_printf("BERT counter2[11:8] : %d\n\r", Data);

	/* BERT counter3[7:0] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x1E, &Data);
	xil_printf("BERT counter3[7:0]  : %d\n\r", Data);

	/* BERT counter3[11:8] */
	XDpRxSs_Dp159Read(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x1F, &Data);
	xil_printf("BERT counter3[11:8] : %d\n\r", Data);

	/* Clear BERT counters and TST_INTQ latches - Self-clearing in DP159 */
	XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0x15, 0x18);

	/* Select page 1 */
	XDpRxSs_Dp159Write(InstancePtr, XDPRXSS_DP159_IIC_SLAVE, 0xFF, 0x01);
}
#endif /* End of XPAR_XIIC_NUM_INSTANCES */
/** @} */
