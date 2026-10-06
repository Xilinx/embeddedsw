/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp21txss_hw.h
 *
 * This file contains the register offsets and bit masks for the Display Stream
 * Compression and Forward Error Correction blocks of the Xilinx DisplayPort 2.1
 * TX Subsystem.
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
#ifndef XDP21TXSS_HW_H_
#define XDP21TXSS_HW_H_

/***************************** Include Files **********************************/
#include "xil_types.h"

/**************************** Constant Definitions ****************************/
/** @name Subsystem register offsets for the compression and FEC blocks.
  * @{
  */
#define XDP21_TX_DSC_PPS_DATA_BASE_OFFSET   0x0710  /**< First word of the
                                                      picture parameter set
                                                      sent in the secondary
                                                      data packet. */
#define XDP21_TX_DSC_PPS_DATA_LAST_OFFSET   0x078C  /**< Last word of the
                                                      picture parameter set,
                                                      bounding the write
                                                      loop. */
#define XDP21_TX_DSC_ENABLE_OFFSET          0x0790  /**< Enables the
                                                      compressor. Bit 0 reads
                                                      back as zero on designs
                                                      that implement this
                                                      register write only. */
#define XDP21_TX_DSC_ENABLE_OFFSET_ALT      0x0094  /**< Second compressor
                                                      enable register, the one
                                                      carrying the slice
                                                      count. Bit 0 enables,
                                                      bits 2 down to 1 hold
                                                      the division factor and
                                                      bits 12 down to 8 hold
                                                      the slice count. */
#define XDP21_TX_DSC_NUM_SLICES_SHIFT       8U      /**< Position of the slice
                                                      count field. */
#define XDP21_TX_DSC_NUM_SLICES_MASK        (0x1FU << XDP21_TX_DSC_NUM_SLICES_SHIFT)
                                                    /**< Slice count field,
                                                      holding the picture
                                                      width divided by the
                                                      slice width rounded
                                                      up. */
#define XDP21_TX_DSC_EN_ACTION_OFFSET       0x0794  /**< Writing one arms
                                                      transmission of the
                                                      picture parameter set
                                                      packet. */
#define XDP21_TX_PPS_SDP_CTRL_OFFSET        0x0804  /**< Picture parameter set
                                                      packet control. */
#define XDP21_TX_PPS_SDP_SEND_OFFSET        0x0808  /**< Writing one sends the
                                                      picture parameter set
                                                      packet. */
#define XDP21_TX_FEC_CTRL_OFFSET            0x0810  /**< Forward error
                                                      correction control. */
#define XDP21_TX_FEC_STATUS_OFFSET          0x0814  /**< Forward error
                                                      correction status. */
/** @} */

/** @name Bit masks for the control registers above.
  * @{
  */
#define XDP21_TX_PPS_SDP_ENABLE             (1 << 0)  /**< In the packet
                                                        control register,
                                                        enables the picture
                                                        parameter set
                                                        packet. */
#define XDP21_TX_PPS_SDP_TRIGGER            (1 << 1)  /**< In the packet
                                                        control register,
                                                        triggers one
                                                        transmission. */
#define XDP21_TX_DSC_PASSTHROUGH            (1 << 2)  /**< In the packet
                                                        control register,
                                                        forwards an already
                                                        compressed stream
                                                        without recompressing
                                                        it. */
#define XDP21_TX_FEC_ENABLE                 (1 << 0)  /**< In the FEC control
                                                        register, enables the
                                                        encoder. */
/** @} */

/**************************** Type Definitions *******************************/

/**************************** Function Prototypes ****************************/

#endif /* XDP21TXSS_HW_H_ */
