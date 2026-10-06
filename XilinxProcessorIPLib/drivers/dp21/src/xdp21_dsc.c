/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdp21_dsc.c
 * @brief Picture Parameter Set packing and validation for Display Stream
 * Compression.
 */

#include "xdp21_dsc.h"

#ifdef XDP_USE_DSC

#include "xil_assert.h"
#include "xil_io.h"
#include "xstatus.h"
#include <string.h>

void XDp21_DscInitContext(XDp21_DscContext *ctx)
{
    Xil_AssertVoid(ctx != NULL);

    memset(ctx, 0, sizeof(XDp21_DscContext));
    ctx->dsc_passthrough_enable = 0;
    ctx->pps_valid = 0;
}

int XDp21_DscParsePps(const u8 *pps_buffer, XDp21_DscPps *pps_data)
{
    if (!pps_buffer || !pps_data) {
        return XST_FAILURE;
    }

    pps_data->dsc_version_major = (pps_buffer[0] >> 4) & 0x0F;
    pps_data->dsc_version_minor = pps_buffer[0] & 0x0F;
    pps_data->pps_identifier = pps_buffer[1];
    pps_data->bits_per_component = (pps_buffer[3] >> 4) & 0x0F;
    pps_data->linebuf_depth = pps_buffer[3] & 0x0F;
    pps_data->block_pred_enable = (pps_buffer[4] >> 5) & 0x01;
    pps_data->convert_rgb = (pps_buffer[4] >> 4) & 0x01;
    pps_data->simple_422 = (pps_buffer[4] >> 3) & 0x01;
    pps_data->vbr_enable = (pps_buffer[4] >> 2) & 0x01;
    pps_data->bits_per_pixel_high = (pps_buffer[4] & 0x03);

    pps_data->bits_per_pixel_low = pps_buffer[5];
    pps_data->pic_height = (pps_buffer[6] << 8) | pps_buffer[7];
    pps_data->pic_width = (pps_buffer[8] << 8) | pps_buffer[9];
    pps_data->slice_height = (pps_buffer[10] << 8) | pps_buffer[11];
    pps_data->slice_width = (pps_buffer[12] << 8) | pps_buffer[13];
    pps_data->chunk_size = (pps_buffer[14] << 8) | pps_buffer[15];
    pps_data->initial_xmit_delay_high = (pps_buffer[16] & 0x03);
    pps_data->initial_xmit_delay_low = pps_buffer[17];
    pps_data->initial_dec_delay = (pps_buffer[18] << 8) | pps_buffer[19];
    pps_data->initial_scale_value = pps_buffer[21] & 0x3F;
    pps_data->scale_increment_interval = (pps_buffer[22] << 8) | pps_buffer[23];
    pps_data->scale_decrement_interval_high = (pps_buffer[24] & 0x0F);
    pps_data->scale_decrement_interval_low = pps_buffer[25];
    pps_data->first_line_bpg_offset = pps_buffer[27] & 0x1F;
    pps_data->nfl_bpg_offset = (pps_buffer[28] << 8) | pps_buffer[29];
    pps_data->slice_bpg_offset = (pps_buffer[30] << 8) | pps_buffer[31];
    pps_data->initial_offset = (pps_buffer[32] << 8) | pps_buffer[33];
    pps_data->final_offset = (pps_buffer[34] << 8) | pps_buffer[35];
    pps_data->flatness_min_qp = pps_buffer[36] & 0x1F;
    pps_data->flatness_max_qp = pps_buffer[37] & 0x1F;
    pps_data->rc_model_size = (pps_buffer[38] << 8) | pps_buffer[39];
    pps_data->rc_edge_factor = pps_buffer[40] & 0x0F;
    pps_data->rc_quant_incr_limit0 = pps_buffer[41] & 0x1F;
    pps_data->rc_quant_incr_limit1 = pps_buffer[42] & 0x1F;
    pps_data->rc_tgt_offset_high = (pps_buffer[43] >> 4) & 0x0F;
    pps_data->rc_tgt_offset_low = pps_buffer[43] & 0x0F;
    memcpy(pps_data->rc_buf_thresh, &pps_buffer[44], 14);
    memcpy(pps_data->rc_range_parameters, &pps_buffer[58], 30);
    pps_data->native_420 = (pps_buffer[88] >> 1) & 0x01;
    pps_data->native_422 = pps_buffer[88] & 0x01;
    pps_data->second_line_bpg_offset = pps_buffer[89] & 0x1F;
    pps_data->nsl_bpg_offset = (pps_buffer[90] << 8) | pps_buffer[91];
    pps_data->second_line_offset_adj = (pps_buffer[92] << 8) | pps_buffer[93];
    pps_data->pps_crc[0] = pps_buffer[126];
    pps_data->pps_crc[1] = pps_buffer[127];

    return XST_SUCCESS;
}

int XDp21_DscEncodePps(const XDp21_DscPps *pps_data, u8 *pps_buffer)
{
    if (!pps_data || !pps_buffer) {
        return XST_FAILURE;
    }

    memset(pps_buffer, 0, 128);
    pps_buffer[0] = (pps_data->dsc_version_major << 4) | (pps_data->dsc_version_minor & 0x0F);
    pps_buffer[1] = pps_data->pps_identifier;
    pps_buffer[3] = (pps_data->bits_per_component << 4) | (pps_data->linebuf_depth & 0x0F);
    pps_buffer[4] = (pps_data->block_pred_enable << 5) | (pps_data->convert_rgb << 4) |
                    (pps_data->simple_422 << 3) | (pps_data->vbr_enable << 2) |
                    (pps_data->bits_per_pixel_high & 0x03);
    pps_buffer[5] = pps_data->bits_per_pixel_low;
    pps_buffer[6] = (pps_data->pic_height >> 8) & 0xFF;
    pps_buffer[7] = pps_data->pic_height & 0xFF;
    pps_buffer[8] = (pps_data->pic_width >> 8) & 0xFF;
    pps_buffer[9] = pps_data->pic_width & 0xFF;
    pps_buffer[10] = (pps_data->slice_height >> 8) & 0xFF;
    pps_buffer[11] = pps_data->slice_height & 0xFF;
    pps_buffer[12] = (pps_data->slice_width >> 8) & 0xFF;
    pps_buffer[13] = pps_data->slice_width & 0xFF;
    pps_buffer[14] = (pps_data->chunk_size >> 8) & 0xFF;
    pps_buffer[15] = pps_data->chunk_size & 0xFF;
    pps_buffer[16] = pps_data->initial_xmit_delay_high & 0x03;
    pps_buffer[17] = pps_data->initial_xmit_delay_low;
    pps_buffer[18] = (pps_data->initial_dec_delay >> 8) & 0xFF;
    pps_buffer[19] = pps_data->initial_dec_delay & 0xFF;
    pps_buffer[21] = pps_data->initial_scale_value & 0x3F;
    pps_buffer[22] = (pps_data->scale_increment_interval >> 8) & 0xFF;
    pps_buffer[23] = pps_data->scale_increment_interval & 0xFF;
    pps_buffer[24] = pps_data->scale_decrement_interval_high & 0x0F;
    pps_buffer[25] = pps_data->scale_decrement_interval_low;
    pps_buffer[27] = pps_data->first_line_bpg_offset & 0x1F;
    pps_buffer[28] = (pps_data->nfl_bpg_offset >> 8) & 0xFF;
    pps_buffer[29] = pps_data->nfl_bpg_offset & 0xFF;
    pps_buffer[30] = (pps_data->slice_bpg_offset >> 8) & 0xFF;
    pps_buffer[31] = pps_data->slice_bpg_offset & 0xFF;
    pps_buffer[32] = (pps_data->initial_offset >> 8) & 0xFF;
    pps_buffer[33] = pps_data->initial_offset & 0xFF;
    pps_buffer[34] = (pps_data->final_offset >> 8) & 0xFF;
    pps_buffer[35] = pps_data->final_offset & 0xFF;
    pps_buffer[36] = pps_data->flatness_min_qp & 0x1F;
    pps_buffer[37] = pps_data->flatness_max_qp & 0x1F;
    pps_buffer[38] = (pps_data->rc_model_size >> 8) & 0xFF;
    pps_buffer[39] = pps_data->rc_model_size & 0xFF;
    pps_buffer[40] = pps_data->rc_edge_factor & 0x0F;
    pps_buffer[41] = pps_data->rc_quant_incr_limit0 & 0x1F;
    pps_buffer[42] = pps_data->rc_quant_incr_limit1 & 0x1F;
    pps_buffer[43] = (pps_data->rc_tgt_offset_high << 4) | (pps_data->rc_tgt_offset_low & 0x0F);
    memcpy(&pps_buffer[44], pps_data->rc_buf_thresh, 14);
    memcpy(&pps_buffer[58], pps_data->rc_range_parameters, 30);
    pps_buffer[88] = (pps_data->native_420 << 1) | (pps_data->native_422 & 0x01);
    pps_buffer[89] = pps_data->second_line_bpg_offset & 0x1F;
    pps_buffer[90] = (pps_data->nsl_bpg_offset >> 8) & 0xFF;
    pps_buffer[91] = pps_data->nsl_bpg_offset & 0xFF;
    pps_buffer[92] = (pps_data->second_line_offset_adj >> 8) & 0xFF;
    pps_buffer[93] = pps_data->second_line_offset_adj & 0xFF;
    pps_buffer[126] = pps_data->pps_crc[0];
    pps_buffer[127] = pps_data->pps_crc[1];

    return XST_SUCCESS;
}

int XDp21_DscValidatePps(const XDp21_DscPps *pps_data)
{
    if (!pps_data) {
        return XST_FAILURE;
    }

    if (pps_data->bits_per_component != 8 &&
        pps_data->bits_per_component != 10 &&
        pps_data->bits_per_component != 12) {
        return XST_FAILURE;
    }

    if (pps_data->pic_width == 0 || pps_data->pic_height == 0) {
        return XST_FAILURE;
    }

    if (pps_data->slice_width == 0 || pps_data->slice_height == 0) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

#endif /* XDP_USE_DSC */
