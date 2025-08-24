// Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
/*copyright (C) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * @file vpi_error_code.h
 * @brief Error codes for VeriSilicon Platform Interface(VPI)
 *
 * This header define error codes for VPI
 */

#ifndef _MBOX_ERROR_CODE_H_
#define _MBOX_ERROR_CODE_H_

/** Error codes for VeriSilicon Platform Interface(VPI) */
typedef enum VpiError {
	VPI_ERR_GENERIC = -1, /**< Generic Error */
	VPI_SUCCESS = 0, /**< Success */
	VPI_ERR_INVALID, /**< Invalid input parameter */
	VPI_ERR_NOMEM, /**< No memory error */
	VPI_ERR_UNINITED, /**< Uninitialized error */
	VPI_ERR_FULL, /**< Resource is full */
	VPI_ERR_EMPTY, /**< Resource is empty */
	VPI_ERR_IO, /**< IO error */
	VPI_ERR_BUSY, /**< Device is busy */
	VPI_ERR_TIMEOUT, /**< Timeout */
	VPI_ERR_NODEVICE, /**< No device error */
	VPI_ERR_LOST_CON, /**< Lost connection */
	VPI_ERR_NO_ACK, /**< No ACK */
	VPI_ERR_NOT_READY, /**< System or data is not ready */
	VPI_ERR_LACK, /**< Not enough data */
	VPI_ERR_IOCTL,
	DUMMY_VPI_ERR = 0xDEADFEED,
} VpiError;

#endif // _MBOX_ERROR_CODE_H_
