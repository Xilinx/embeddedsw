/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdp21_dsc.h
 * @brief Picture Parameter Set types and Display Stream Compression feature
 * selection for the DisplayPort 2.1 core.
 */

#ifndef XDP21_DSC_H_
#define XDP21_DSC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xil_types.h"

/*
 * The value selects the build, not the presence of the macro. A design that
 * builds the Display Stream Compression blocks in reports one, and the macro
 * is also generated as zero whenever the device tree carries xlnx,enable-dsc
 * with that value, so testing only whether it is defined would compile the
 * compression code into designs that cannot use it. An undefined macro
 * evaluates to zero here, which covers the designs whose device tree omits
 * the property entirely. Both core indices are tested because a passthrough
 * design instantiates a transmit and a receive core and either of them may
 * carry the DSC blocks.
 *
 * This is the one place the feature is selected. It lives in the leaf DSC
 * header so that xdp.h can include it for both the macro and the PPS type
 * without the two headers depending on each other.
 */
#if XPAR_XDP_0_ENABLE_DSC || XPAR_XDP_1_ENABLE_DSC
#define XDP_USE_DSC
#endif

#ifdef XDP_USE_DSC

typedef struct {
    u8 dsc_version_major;
    u8 dsc_version_minor;
    u8 pps_identifier;
    u8 bits_per_component;
    u8 linebuf_depth;
    u8 block_pred_enable;
    u8 convert_rgb;
    u8 simple_422;
    u8 vbr_enable;
    u8 bits_per_pixel_low;
    u8 bits_per_pixel_high;
    u16 pic_height;
    u16 pic_width;
    u16 slice_height;
    u16 slice_width;
    u16 chunk_size;
    u8 initial_xmit_delay_high;
    u8 initial_xmit_delay_low;
    u16 initial_dec_delay;
    u8 initial_scale_value;
    u16 scale_increment_interval;
    u8 scale_decrement_interval_high;
    u8 scale_decrement_interval_low;
    u8 first_line_bpg_offset;
    u16 nfl_bpg_offset;
    u16 slice_bpg_offset;
    u16 initial_offset;
    u16 final_offset;
    u8 flatness_min_qp;
    u8 flatness_max_qp;
    u16 rc_model_size;
    u8 rc_edge_factor;
    u8 rc_quant_incr_limit0;
    u8 rc_quant_incr_limit1;
    u8 rc_tgt_offset_high;
    u8 rc_tgt_offset_low;
    u8 rc_buf_thresh[14];
    u8 rc_range_parameters[30];
    u8 native_420;
    u8 native_422;
    u8 second_line_bpg_offset;
    u16 nsl_bpg_offset;
    u16 second_line_offset_adj;
    u8 pps_crc[2];
} XDp21_DscPps;

#define XDP21_FEC_CAPABLE                   (1 << 0)
#define XDP21_FEC_UNCORRECTED_BLOCK_ERR     (1 << 1)
#define XDP21_FEC_CORRECTED_BLOCK_ERR       (1 << 2)
#define XDP21_FEC_BIT_ERR_COUNT             (1 << 3)
#define XDP21_DSC_PPS_SIZE                  128

typedef struct {
    u8 fec_capable;
    u8 fec_ready;
    u8 fec_decode_enable;
    u8 fec_decode_disable;
    u32 fec_error_count_uncorrected;
    u32 fec_error_count_corrected;
    u32 fec_bit_error_count;
} XDp21_FecConfig;

typedef struct {
    XDp21_DscPps pps_data;
    u8 dsc_passthrough_enable;
    u8 pps_valid;
    XDp21_FecConfig fec_config;
} XDp21_DscContext;

int XDp21_DscParsePps(const u8 *pps_buffer, XDp21_DscPps *pps_data);
int XDp21_DscEncodePps(const XDp21_DscPps *pps_data, u8 *pps_buffer);
int XDp21_DscValidatePps(const XDp21_DscPps *pps_data);
void XDp21_DscInitContext(XDp21_DscContext *ctx);

#endif /* XDP_USE_DSC */

#ifdef __cplusplus
}
#endif

#endif
