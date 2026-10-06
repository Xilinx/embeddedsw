/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp21rxss_hw.h
 *
 * This file contains the register offsets and bit masks for the Display Stream
 * Compression and Forward Error Correction blocks of the Xilinx DisplayPort 2.1
 * RX Subsystem.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who       Date     Changes
 * ----- --------- -------- -----------------------------------------------
 * 1.0   Dhanunjay 10/05/26 Initial release.
 * </pre>
 *
*******************************************************************************/
#ifndef XDP21RXSS_HW_H_
#define XDP21RXSS_HW_H_

/***************************** Include Files **********************************/
#include "xil_types.h"

/**************************** Constant Definitions ****************************/
/** @name Subsystem register offsets for the compression and FEC blocks.
  * @{
  */
#define XDP21_RX_PPS_SDP_BASE_OFFSET        0x0800  /**< First word of the
                                                      picture parameter set
                                                      captured from the
                                                      received secondary data
                                                      packet. */
#define XDP21_RX_PPS_SDP_STATUS_OFFSET      0x0804  /**< Picture parameter set
                                                      capture status. */
#define XDP21_RX_PPS_SDP_CTRL_OFFSET        0x0808  /**< Picture parameter set
                                                      capture control. */
#define XDP21_RX_DSC_ENABLE_OFFSET          0x080C  /**< Enables the
                                                      decompressor. */
#define XDP21_RX_FEC_STATUS_OFFSET          0x0810  /**< Forward error
                                                      correction status. */
#define XDP21_RX_FEC_CTRL_OFFSET            0x0814  /**< Forward error
                                                      correction control. */
#define XDP21_RX_FEC_ERR_COUNT_OFFSET       0x0818  /**< Count of errors the
                                                      decoder could not
                                                      correct. */
#define XDP21_RX_FEC_CORR_COUNT_OFFSET      0x081C  /**< Count of errors the
                                                      decoder corrected. */
/** @} */

/** @name Bit masks for the status and control registers above.
  * @{
  */
#define XDP21_RX_PPS_SDP_VALID              (1 << 0)  /**< In the status
                                                        register, the captured
                                                        picture parameter set
                                                        is valid. */
#define XDP21_RX_DSC_STREAM_ACTIVE          (1 << 1)  /**< In the status
                                                        register, the received
                                                        stream is
                                                        compressed. */
#define XDP21_RX_FEC_ACTIVE                 (1 << 0)  /**< In the FEC status
                                                        register, the decoder
                                                        is running. */
/** @} */

/**************************** Type Definitions *******************************/

/**************************** Function Prototypes ****************************/

#endif /* XDP21RXSS_HW_H_ */
