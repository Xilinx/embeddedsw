/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef _GPIO_H_
#define _GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * APU Base Address
 */
#define GPIO_BASEADDR      (0XFF0A0000U)

/**
 * Register: GPIO_MASK_DATA_5_MSW_REG
 */
#define GPIO_MASK_DATA_5_MSW_REG    ( GPIO_BASEADDR  + 0X0000002CU )

/**
 * Register: GPIO_DIRM_5
 */
#define GPIO_DIRM_5    ( GPIO_BASEADDR + 0X00000344U )

#define MAX_REG_BITS               32

/*
 * GPIO5 EMIO[95:92] are the PS-PL reset lines
 */
#define GPIO5_EMIO92_MSW_DATA_BIT    12
#define GPIO5_EMIO93_MSW_DATA_BIT    13
#define GPIO5_EMIO94_MSW_DATA_BIT    14
#define GPIO5_EMIO95_MSW_DATA_BIT    15

#define GPIO_PIN_MASK_BITS         0xFFFF0000

#ifdef __cplusplus
}
#endif


#endif /* _GPIO_H_ */
