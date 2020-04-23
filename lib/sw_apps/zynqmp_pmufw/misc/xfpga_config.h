/******************************************************************************
* Copyright (c) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#ifndef _XFPGA_CONFIG_H
#define _XFPGA_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xilfpga_pcap_common.h>
#include <xilfpga_pcap.h>

#define XFPGA_OCM_ADDRESS 0xfffc0000U
#define XFPGA_BASE_ADDRESS 0x80000U
#define XFPGA_SECURE_MODE
#define XFPGA_DEBUG     (0U)

#ifdef __cplusplus
}
#endif

#endif /* _XFPGA_CONFIG_H */
