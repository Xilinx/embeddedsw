/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_micron_asp.h
 * @addtogroup xilsfl overview
 * @{
 *
 * Micron MT35 OCTAL/QUAD NOR ASP vendor-private opcodes and constants: command
 * opcodes, dummy cycle counts, password width, and the Sector Protection
 * Register (SPR) OTP lock bit. Included only from xilsfl_micron_asp.c and not
 * part of the public xilsfl API.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.3   sb  07/20/26  Initial support for Advanced Sector Protection.
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XILSFL_MICRON_ASP_H
#define XILSFL_MICRON_ASP_H

/** READ / PROGRAM Sector Protection Register. */
#define XSFL_MICRON_PROGRAM_SECTOR_PROT_CMD		0x2CU /**< Sector protection program command */
#define XSFL_MICRON_READ_SECTOR_PROT_CMD		0x2DU /**< Sector protection read command */

/** Nonvolatile lock reads / writes / bulk erase. */
#define XSFL_MICRON_READ_NV_LOCK_CMD			0xE2U /**< NV lock read command */
#define XSFL_MICRON_WRITE_NV_LOCK_CMD			0xE3U /**< NV lock write command */
#define XSFL_MICRON_ERASE_NV_LOCK_BITS_CMD		0xE4U /**< NV lock bits erase command */

/** WRITE GLOBAL FREEZE BIT. */
#define XSFL_MICRON_WRITE_GLOBAL_FREEZE_CMD		0xA6U /**<Global freeze write command */

/** READ / WRITE / UNLOCK PASSWORD. */
#define XSFL_MICRON_READ_PASSWORD_CMD		0x27U /**< Password read command */
#define XSFL_MICRON_WRITE_PASSWORD_CMD		0x28U /**< Password write command */
#define XSFL_MICRON_UNLOCK_PASSWORD_CMD		0x29U /**< Password unlock command */

/** READ PASSWORD (27h) payload width on OCTAL/QUAD flash parts. */
#define XSFL_MICRON_ASP_PASSWORD_BYTES		8U

/** Common dummy cycle count for all Micron OCTAL/QUAD register reads (SPR, NV lock, password). */
#define XSFL_MICRON_REG_READ_DUMMY_OCTAL	8U /**< Dummy clock bytes for ASP registers*/

/** SPR bit 1: sector protection lock OTP bit. When 0, password protection is permanently disabled. */
#define XSFL_MICRON_SPR_OTP_LOCK_BIT    (u16)0x0002U /**< SPR lock bit */

/** @} */

#endif /* XILSFL_MICRON_ASP_H */
