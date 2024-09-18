/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_REQUIREMENT_INFO_H_
#define XPM_REQUIREMENT_INFO_H_

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Size of bit fields for XPm_ReqmInfo structure */
#define REQ_INFO_CAPS_BIT_FIELD_SIZE		4
#define REQ_INFO_LATENCY_BIT_FIELD_SIZE		21
#define REQ_INFO_RESERVED_BIT_FIELD_SIZE	7

typedef struct XPm_ReqmInfo XPm_ReqmInfo;
/**
 * Specific requirement information.
 */
struct XPm_ReqmInfo {
	u32 Capabilities:REQ_INFO_CAPS_BIT_FIELD_SIZE; /**< Device capabilities (1-hot) */
	u32 Latency:REQ_INFO_LATENCY_BIT_FIELD_SIZE; /**< Maximum device latency */
	u32 Reserved:REQ_INFO_RESERVED_BIT_FIELD_SIZE; /**< Reserved for future use */
	u32 QoS; /**< QoS requirement */
};

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_REQUIREMENT_INFO_H_ */
