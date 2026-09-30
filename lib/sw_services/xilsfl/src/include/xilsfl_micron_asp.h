/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XILSFL_MICRON_ASP_H
#define XILSFL_MICRON_ASP_H

/*****************************************************************************/
/**
 *
 * @file xilsfl_micron_asp.h
 * @brief Defines the Micron MT35 ASP vendor-private opcodes and constants.
 *
 * Micron MT35 OCTAL/QUAD NOR Advanced Sector Protection (ASP) vendor-private
 * opcodes and constants. These are the command opcodes, dummy cycle counts,
 * password width, and the Sector Protection Register (SPR) one-time
 * programmable (OTP) lock bit. Only xilsfl_micron_asp.c includes this file,
 * and it is not part of the public xilsfl API.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.3   sb  07/20/26  Initial support for Advanced Sector Protection.
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

/**
 * @name READ / PROGRAM Sector Protection Register
 * @{
 */
/** Sector protection program command. */
#define XSFL_MICRON_PROGRAM_SECTOR_PROT_CMD		0x2CU
/** Sector protection read command. */
#define XSFL_MICRON_READ_SECTOR_PROT_CMD		0x2DU
/** @} */

/**
 * @name Nonvolatile lock reads / writes / bulk erase
 * @{
 */
/** NV lock read command. */
#define XSFL_MICRON_READ_NV_LOCK_CMD			0xE2U
/** NV lock write command. */
#define XSFL_MICRON_WRITE_NV_LOCK_CMD			0xE3U
/** NV lock bits erase command. */
#define XSFL_MICRON_ERASE_NV_LOCK_BITS_CMD		0xE4U
/** @} */

/** Global freeze bit write command (WRITE GLOBAL FREEZE BIT). */
#define XSFL_MICRON_WRITE_GLOBAL_FREEZE_CMD		0xA6U

/**
 * @name READ / WRITE / UNLOCK PASSWORD
 * @{
 */
/** Password read command. */
#define XSFL_MICRON_READ_PASSWORD_CMD		0x27U
/** Password write command. */
#define XSFL_MICRON_WRITE_PASSWORD_CMD		0x28U
/** Password unlock command. */
#define XSFL_MICRON_UNLOCK_PASSWORD_CMD		0x29U
/** @} */

/** READ PASSWORD (27h) payload width on OCTAL/QUAD flash parts. */
#define XSFL_MICRON_ASP_PASSWORD_BYTES		8U

/**
 * Common dummy cycle count for all Micron OCTAL/QUAD register reads (SPR, NV
 * lock, password), in dummy clock cycles.
 */
#define XSFL_MICRON_REG_READ_DUMMY_OCTAL	8U

/**
 * SPR bit 1: sector protection lock OTP bit (SPR lock bit). When 0, password
 * protection is permanently disabled.
 */
#define XSFL_MICRON_SPR_OTP_LOCK_BIT    (u16)0x0002U

/** @} */

#endif /* XILSFL_MICRON_ASP_H */
