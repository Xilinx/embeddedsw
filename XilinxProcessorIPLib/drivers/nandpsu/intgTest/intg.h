/******************************************************************************
*
* (c) Copyright 2010-14 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file intg.h - defines integration test API for the can driver
*
* @note
* TEST_METHOD has to be defined whether you want
*	- internal loopback (TEST_LOOPBACK) or
*	- receive/send data from/to (TEST_EXTERNAL_CAN_ANALYSER) an
*	  external Can Analyser.
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	 Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/09 First release
* 1.01  kvn	   26/03/14 First release for xilinx.
*
* </pre>
*
*****************************************************************************/
#ifndef INTG_H  /**< prevent circular inclusions */
#define INTG_H  /**< by using protection macros */

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xnandps8.h"
#include "xscugic.h"
#include "xstatus.h"
#include <stdio.h>
#include <stdlib.h>
#include "ct.h"
#include "xil_exception.h"
#include "xil_testlib.h"
#ifdef __aarch64__
#include "xreg_cortexa53.h"
#else
#include "xreg_cortexr5.h"
#endif
/************************** Constant Definitions ****************************/

/**
 * Nand device and other important HW properties for this build
 * @{
 */
#define INTG_XPAR_INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTG_XPAR_DEVICE_INTR_VEC	XPAR_XNANDPSU_0_INTR
#define printf xil_printf
/*@}*/
#define NAND_DEVICE_ID		0U
#define TEST_BUF_SIZE		0x4000U
#define TEST_PAGE_START		0x2U
#define TEST_BLOCK_START	0x1U

/*
 * Buffers used during read and write transactions.
 */
extern u8 ReadBuffer[];		/**< read buffer */
extern u8 WriteBuffer[];	/**< write buffer */

/**
 * Pele Regression tests
 */
#define AUTOMATIC_TEST_MODE

/**************************** Type Definitions ******************************/

#define printf xil_printf

/***************** Macros (Inline Functions) Definitions ********************/

/**< Clear the Structure variable to zero */
#define Intg_ClearStruct(s) (memset(&(s), 0, sizeof(s)))

/************************** Variable Definitions ****************************/

/**< XNandPsu instance used throughout tests */
extern XNandPsu* NandInstPtr;
s32 MismatchCounter;
/************************** Function Prototypes *****************************/

/*
 * Utility test functions implemented in intg.c
 */
void Intg_Entry(void);
int Intg_ReinitializeInstance(void);
int UART_RecvByte(u8 *Data);

/*
 * Flash Erase Read test implemented in intg_erase_read.c
 */
int Intg_EraseReadTest(XNandPsu * NandInstPtr, int TestLoops);

/*
 * Flash Read Write test implemented in intg_flash_rw.c
 */
int Intg_FlashRWTest(XNandPsu * NandInstPtr, int TestLoops);

/*
 * Random Block Read Write test implemented in intg_random_rw.c
 */
int Intg_RandomRWTest(XNandPsu * NandInstPtr, int TestLoops);

/*
 * SpareBytes Read Write test implemented in intg_sparebytes_rw.c
 */
int Intg_SpareBytesRWTest(XNandPsu * NandInstPtr,int TestLoops);

/*
 * Partial Page Read Write test implemented in intg_partialpage_rw.c
 */
int Intg_PartialRWTest(XNandPsu * NandInstPtr,int TestLoops);

/*
 * ECC error check tests implemented in intg_ecc_test.c
 */
int Intg_EccTest(XNandPsu * NandInstPtr,int TestLoops);

/*
 * BBT Scan test implemented in intg_bbt_test.c
 */
int Intg_BbtTest(XNandPsu * NandInstPtr, int TestLoops);

/*
 * Mark Block Bad test implemented in intg_markblockbad_test.c
 */
int Intg_MarkBlockBadTest(XNandPsu * NandInstPtr, int TestLoops);

/*
 * Code Coverage test implemented in intg_codecoverage_test.c
 */
int Intg_CodeCoverageTest(XNandPsu * NandInstPtr, int TestLoops);

#endif	/**< End of protection macro */
