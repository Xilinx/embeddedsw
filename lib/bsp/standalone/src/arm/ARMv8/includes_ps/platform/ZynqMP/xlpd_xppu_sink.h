/*******************************************************************************
 *
 * Copyright (C) 2014-2015 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/

#ifndef __XLPD_XPPU_SINK_H__
#define __XLPD_XPPU_SINK_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * XlpdXppuSink Base Address
 */
#define XLPD_XPPU_SINK_BASEADDR      0xFF9C0000UL

/**
 * Register: XlpdXppuSinkErrSts
 */
#define XLPD_XPPU_SINK_ERR_STS    ( ( XLPD_XPPU_SINK_BASEADDR ) + 0x0000FF00UL )
#define XLPD_XPPU_SINK_ERR_STS_RSTVAL   0x00000000UL

#define XLPD_XPPU_SINK_ERR_STS_RDWR_SHIFT   31UL
#define XLPD_XPPU_SINK_ERR_STS_RDWR_WIDTH   1UL
#define XLPD_XPPU_SINK_ERR_STS_RDWR_MASK    0x80000000UL
#define XLPD_XPPU_SINK_ERR_STS_RDWR_DEFVAL  0x0UL

#define XLPD_XPPU_SINK_ERR_STS_ADDR_SHIFT   0UL
#define XLPD_XPPU_SINK_ERR_STS_ADDR_WIDTH   12UL
#define XLPD_XPPU_SINK_ERR_STS_ADDR_MASK    0x00000fffUL
#define XLPD_XPPU_SINK_ERR_STS_ADDR_DEFVAL  0x0UL

/**
 * Register: XlpdXppuSinkIsr
 */
#define XLPD_XPPU_SINK_ISR    ( ( XLPD_XPPU_SINK_BASEADDR ) + 0x0000FF10UL )
#define XLPD_XPPU_SINK_ISR_RSTVAL   0x00000000UL

#define XLPD_XPPU_SINK_ISRADDRDECDERR_SHIFT   0UL
#define XLPD_XPPU_SINK_ISRADDRDECDERR_WIDTH   1UL
#define XLPD_XPPU_SINK_ISRADDRDECDERR_MASK    0x00000001UL
#define XLPD_XPPU_SINK_ISRADDRDECDERR_DEFVAL  0x0UL

/**
 * Register: XlpdXppuSinkImr
 */
#define XLPD_XPPU_SINK_IMR    ( ( XLPD_XPPU_SINK_BASEADDR ) + 0x0000FF14UL )
#define XLPD_XPPU_SINK_IMR_RSTVAL   0x00000001UL

#define XLPD_XPPU_SINK_IMRADDRDECDERR_SHIFT   0UL
#define XLPD_XPPU_SINK_IMRADDRDECDERR_WIDTH   1UL
#define XLPD_XPPU_SINK_IMRADDRDECDERR_MASK    0x00000001UL
#define XLPD_XPPU_SINK_IMRADDRDECDERR_DEFVAL  0x1UL

/**
 * Register: XlpdXppuSinkIer
 */
#define XLPD_XPPU_SINK_IER    ( ( XLPD_XPPU_SINK_BASEADDR ) + 0x0000FF18UL )
#define XLPD_XPPU_SINK_IER_RSTVAL   0x00000000UL

#define XLPD_XPPU_SINK_IERADDRDECDERR_SHIFT   0UL
#define XLPD_XPPU_SINK_IERADDRDECDERR_WIDTH   1UL
#define XLPD_XPPU_SINK_IERADDRDECDERR_MASK    0x00000001UL
#define XLPD_XPPU_SINK_IERADDRDECDERR_DEFVAL  0x0UL

/**
 * Register: XlpdXppuSinkIdr
 */
#define XLPD_XPPU_SINK_IDR    ( ( XLPD_XPPU_SINK_BASEADDR ) + 0x0000FF1CUL )
#define XLPD_XPPU_SINK_IDR_RSTVAL   0x00000000UL

#define XLPD_XPPU_SINK_IDRADDRDECDERR_SHIFT   0UL
#define XLPD_XPPU_SINK_IDRADDRDECDERR_WIDTH   1UL
#define XLPD_XPPU_SINK_IDRADDRDECDERR_MASK    0x00000001UL
#define XLPD_XPPU_SINK_IDRADDRDECDERR_DEFVAL  0x0UL


#ifdef __cplusplus
}
#endif

#endif /* __XLPD_XPPU_SINK_H__ */
