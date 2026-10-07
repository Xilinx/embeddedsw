/******************************************************************************
* Copyright (C) 1995 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/**
 * @file xil_printf.c
 * @brief Lightweight printf implementation for embedded standalone BSP.
 */
/*---------------------------------------------------*/
/* Modified from :                                   */
/* Public Domain version of printf                   */
/* Rud Merriam, Compsult, Inc. Houston, Tx.          */
/* For Embedded Systems Programming, 1991            */
/*                                                   */
/*---------------------------------------------------*/
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "bspconfig.h"
#include "xil_stdarg.h"
#include <string.h>

/************************** Function Prototypes ******************************/

static void padding( const s32 l_flag, const struct params_s *par);
static void outs(const charptr lp, struct params_s *par);
static s32 getnum( charptr *linep);

/**************************** Type Definitions *******************************/

typedef struct params_s {
	s32 len;  /**< length */
	s32 num1; /**< number 1 */
	s32 num2; /**< number 2 */
	char8 pad_character;  /**< pad character */
	s32 do_padding; /**< do padding */
	s32 left_flag;  /**< left flag */
	s32 unsigned_flag; /**< unsigned flag */
} params_t;

/************************** Additional Prototypes ****************************/

static void handle_escape_sequence(char8 ch);
static void handle_digit_flag(u8 ch, s32 dot_flag, params_t *par, char8 **ctrl);
static void print_bounded_string(const char *string, u32 width);
static s32 is_star_s_format(const char8 *ctrl);
static void handle_char_output(char8 ch);
static void handle_long_flag(char8 **ctrl, s32 *long_flag);
static void handle_decimal_output(params_t *par, va_list *argp, s32 long_flag);
static void handle_hex_output(params_t *par, va_list *argp, s32 long_flag);
static void process_format_spec(char8 **ctrl_ptr, va_list *argp);

/***************** Macros (Inline Functions) Definitions *********************/

#if (defined(__MICROBLAZE__)) && (!defined(__arch64__))
#define MICROBLAZE32
#endif

#if (defined(__riscv) && (__riscv_xlen == 32))
#define RISCV32
#endif

#if (!defined(MICROBLAZE32)) && (!defined(RISCV32)) && \
	(!defined(ZYNQMP_R5_FSBL_BSP))  && (!defined(DISABLE_64BIT_PRINT))
/** @brief Defined when the target supports 64-bit print formatting. */
#define SUPPORT_64BIT_PRINT
#endif

/*---------------------------------------------------*/
/* The purpose of this routine is to output data the */
/* same as the standard printf function without the  */
/* overhead most run-time libraries involve. Usually */
/* the printf brings in many kilobytes of code and   */
/* that is unacceptable in most embedded systems.    */
/*---------------------------------------------------*/

/*****************************************************************************/
/**
* This routine checks if a character is a decimal digit ('0'-'9').
*
******************************************************************************/
static int xil_isdigit(unsigned char c)
{
    return (c >= '0' && c <= '9');
}

/*****************************************************************************/
/**
* This routine converts an uppercase letter to lowercase.
*
******************************************************************************/
static unsigned char xil_tolower(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

/*****************************************************************************/
/**
* This routine puts pad characters into the output buffer.
*
******************************************************************************/
static void padding( const s32 l_flag, const struct params_s *par)
{
	s32 i;

	if ((par->do_padding != 0) && (l_flag != 0) && (par->len < par->num1)) {
		i = (par->len);
		for (; i < (par->num1); i++) {
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
			outbyte( par->pad_character);
#endif
		}
	}
}

/******************************************************************************/
/**
* This routine moves a string to the output buffer
* as directed by the padding and positioning flags.
*
*******************************************************************************/
static void outs(const charptr lp, struct params_s *par)
{
	charptr LocalPtr;
	LocalPtr = lp;
	/* pad on left if needed                         */
	if (LocalPtr != NULL) {
		par->len = (s32)strlen( LocalPtr);
		padding( !(par->left_flag), par);
		/* Move string to the buffer                     */
		while (((*LocalPtr) != (char8)0) && ((par->num2) != 0)) {
			(par->num2)--;
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
			outbyte(*LocalPtr);
#endif
			LocalPtr += 1;
		}
	}

	/* Pad on right if needed                        */
	/* CR 439175 - elided next stmt. Seemed bogus.   */
	padding( par->left_flag, par);
}

/*****************************************************************************/
/**
*
* This routine moves a number to the output buffer
* as directed by the padding and positioning flags.
*
******************************************************************************/
static void outnum( const s32 n, const s32 base, struct params_s *par)
{
	s32 negative;
	s32 i;
	char8 outbuf[32];
	const char8 digits[] = "0123456789ABCDEF";
	u32 num;
	for (i = 0; i < 32; i++) {
		outbuf[i] = '0';
	}

	/* Check if number is negative                   */
	if ((par->unsigned_flag == 0) && (base == 10) && (n < 0L)) {
		negative = 1;
		num = (-(n));
	} else {
		num = n;
		negative = 0;
	}

	/* Build number (backwards) in outbuf            */
	i = 0;
	do {
		outbuf[i] = digits[(num % (u32)base)];
		i++;
		num /= base;
	} while (num > 0U);

	if (negative != 0) {
		outbuf[i] = '-';
		i++;
	}

	outbuf[i] = '\0';
	i--;

	/* Move the converted number to the buffer and   */
	/* add in the padding where needed.              */
	par->len = (s32)strlen(outbuf);
	padding( !(par->left_flag), par);
	while (&outbuf[i] >= outbuf) {
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
		outbyte( outbuf[i] );
#endif
		i--;
	}
	padding( par->left_flag, par);
}
#if defined (SUPPORT_64BIT_PRINT)
/**
 * @brief Output a 64-bit number to the buffer with the specified base and padding.
 * @param[in] n    Number to print.
 * @param[in] base Numeric base (for example, 10 for decimal, 16 for hex).
 * @param[in] par  Formatting parameters controlling padding and alignment.
 */
static void outnum1( const s64 n, const s32 base, params_t *par)
{
	s32 negative;
	s32 i;
	char8 outbuf[64];
	const char8 digits[] = "0123456789ABCDEF";
	u64 num;
	for (i = 0; i < 64; i++) {
		outbuf[i] = '0';
	}

	/* Check if number is negative                   */
	if ((par->unsigned_flag == 0) && (base == 10) && (n < 0L)) {
		negative = 1;
		num = (-(n));
	} else {
		num = (n);
		negative = 0;
	}

	/* Build number (backwards) in outbuf            */
	i = 0;
	do {
		outbuf[i] = digits[(num % base)];
		i++;
		num /= (u64)base;
	} while (num > 0U);

	if (negative != 0) {
		outbuf[i] = '-';
		i++;
	}

	outbuf[i] = '\0';
	i--;

	/* Move the converted number to the buffer and   */
	/* add in the padding where needed.              */
	par->len = (s32)strlen(outbuf);
	padding( !(par->left_flag), par);
	while (&outbuf[i] >= outbuf) {
		outbyte( outbuf[i] );
		i--;
	}
	padding( par->left_flag, par);
}
#endif

/*****************************************************************************/
/**
*
* This routine gets a number from the format string.
*
******************************************************************************/
static s32 getnum(charptr *linep)
{
	s32 n = 0;
	s32 ResultIsDigit = 0;
	charptr cptr = *linep;

	while (cptr != NULL) {
		ResultIsDigit = xil_isdigit(((u8) * cptr));
		if (ResultIsDigit == 0) {
			break;
		}
		n = ((n * 10) + (((s32) * cptr) - (s32)'0'));
		cptr += 1;
	}

	*linep = ((charptr)(cptr));
	return (n);
}

/*****************************************************************************/
/**
* This routine handles backslash escape sequences.
*
******************************************************************************/
static void handle_escape_sequence(char8 ch)
{
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
	switch (ch) {
		case 'a':
			outbyte((char8)0x07);
			break;
		case 'h':
			outbyte((char8)0x08);
			break;
		case 'r':
			outbyte((char8)0x0D);
			break;
		case 'n':
			outbyte((char8)0x0D);
			outbyte((char8)0x0A);
			break;
		default:
			outbyte(ch);
			break;
	}
#else
	(void)ch;
#endif
}

/*****************************************************************************/
/**
* This routine processes a digit seen in a format specifier (width/precision).
*
******************************************************************************/
static void handle_digit_flag(u8 ch, s32 dot_flag, params_t *par, char8 **ctrl)
{
	if (dot_flag != 0) {
		par->num2 = getnum(ctrl);
	} else {
		if (ch == (u8)'0') {
			par->pad_character = '0';
		}
		par->num1 = getnum(ctrl);
		par->do_padding = 1;
	}
	(*ctrl) -= 1;
}

/*****************************************************************************/
/**
* This routine outputs up to 'width' characters from 'string', stopping early
* at a null terminator.
*
******************************************************************************/
static void print_bounded_string(const char *string, u32 width)
{
	u32 index;

	for (index = 0U; (index < width) && (string[index] != '\0'); index++) {
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
		outbyte(string[index]);
#endif
	}
}

/*****************************************************************************/
/**
* This routine returns 1 if the format string at ctrl starts with '.*s',
* indicating a width-limited string specifier.
*
******************************************************************************/
static s32 is_star_s_format(const char8 *ctrl)
{
	return ((ctrl[1] == '*') && (ctrl[2] == 's')) ? 1 : 0;
}

/*****************************************************************************/
/**
* This routine outputs a single character to the output device when the
* output peripheral is available.
*
******************************************************************************/
static void handle_char_output(char8 ch)
{
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
	outbyte(ch);
#else
	(void)ch;
#endif
}

/*****************************************************************************/
/**
* This routine handles the 'l' format modifier. On ARMR5 with 64-bit support,
* a second 'l' is required; on other 64-bit platforms a single 'l' suffices.
*
******************************************************************************/
static void handle_long_flag(char8 **ctrl, s32 *long_flag)
{
#if defined(SUPPORT_64BIT_PRINT)
#if defined(ARMR5)
	if (*((*ctrl) + 1) == 'l') {
		*long_flag = 1;
		(*ctrl) += 1;
	}
#else
	(void)ctrl;
	*long_flag = 1;
#endif
#else
	(void)ctrl;
	(void)long_flag;
#endif
}

/*****************************************************************************/
/**
* This routine outputs a decimal integer argument, selecting 32-bit or 64-bit
* output based on long_flag and platform support.
*
******************************************************************************/
static void handle_decimal_output(params_t *par, va_list *argp, s32 long_flag)
{
#if defined(SUPPORT_64BIT_PRINT)
	if (long_flag != 0) {
		outnum1((s64)va_arg(*argp, s64), 10L, par);
	} else {
		outnum(va_arg(*argp, s32), 10L, par);
	}
#else
	(void)long_flag;
	outnum(va_arg(*argp, s32), 10L, par);
#endif
}

/*****************************************************************************/
/**
* This routine outputs a hexadecimal integer argument, selecting 32-bit or
* 64-bit output based on long_flag and platform support.
*
******************************************************************************/
static void handle_hex_output(params_t *par, va_list *argp, s32 long_flag)
{
#if defined(SUPPORT_64BIT_PRINT)
	if (long_flag != 0) {
		outnum1((s64)va_arg(*argp, s64), 16L, par);
	} else {
		outnum((s32)va_arg(*argp, s32), 16L, par);
	}
#else
	(void)long_flag;
	outnum((s32)va_arg(*argp, s32), 16L, par);
#endif
}

/*****************************************************************************/
/**
* This routine processes a single '%'-introduced format specifier.
* On entry *ctrl_ptr points to the '%' character; on return it points to
* the character immediately after the consumed specifier.
*
******************************************************************************/
static void process_format_spec(char8 **ctrl_ptr, va_list *argp)
{
	u8 ch;
	s32 Check = 0;
	s32 dot_flag = 0;
	s32 long_flag = 0;
	u32 width;
	const char *string;
	params_t par;

	par.unsigned_flag = 0;
	par.left_flag = 0;
	par.do_padding = 0;
	par.pad_character = ' ';
	par.num2 = 32767;
	par.num1 = 0;
	par.len = 0;

try_next:
	(*ctrl_ptr) += 1;
	ch = (u8)(**ctrl_ptr);

	if (xil_isdigit(ch) != 0) {
		handle_digit_flag(ch, dot_flag, &par, ctrl_ptr);
		goto try_next;
	}

	switch (xil_tolower(ch)) {
		case '%':
			handle_char_output('%');
			Check = 1;
			break;

		case '-':
			par.left_flag = 1;
			Check = 0;
			break;

		case '.':
			if (is_star_s_format(*ctrl_ptr) != 0) {
				width = va_arg(*argp, u32);
				string = va_arg(*argp, const char *);
				print_bounded_string(string, width);
				(*ctrl_ptr) += 2;
				Check = 1;
			} else {
				dot_flag = 1;
				Check = 0;
			}
			break;

		case 'l':
			handle_long_flag(ctrl_ptr, &long_flag);
			Check = 0;
			break;

		case 'u':
			par.unsigned_flag = 1;
		/* fall through */
		case 'i':
		case 'd':
			handle_decimal_output(&par, argp, long_flag);
			Check = 1;
			break;

		case 'p':
#if defined(__aarch64__) || defined(__arch64__)
			par.unsigned_flag = 1;
			outnum1((s64)va_arg(*argp, s64), 16L, &par);
			Check = 1;
			break;
#endif
		case 'X':
		case 'x':
			par.unsigned_flag = 1;
			handle_hex_output(&par, argp, long_flag);
			Check = 1;
			break;

		case 's':
			outs(va_arg(*argp, char *), &par);
			Check = 1;
			break;

		case 'c':
			handle_char_output((char8)va_arg(*argp, s32));
			Check = 1;
			break;

		case '\\':
			(*ctrl_ptr) += 1;
			handle_escape_sequence(**ctrl_ptr);
			Check = 0;
			break;

		default:
			Check = 1;
			break;
	}

	if (Check == 1) {
		(*ctrl_ptr) += 1;
		return;
	}
	goto try_next;
}

/*****************************************************************************/
/**
* @brief Print a formatted string to the console (subset of printf).
*
* Outputs data under control of a formatting string. Not all standard C format
* controls are supported; primarily those needed for embedded systems work.
* Floating point routines are omitted.
*
* @param[in] ctrl1 Format string controlling the output.
* @param[in] ... Variable arguments matching the format string.
*
******************************************************************************/
#if  defined (__aarch64__) && HYP_GUEST && EL1_NONSECURE && XEN_USE_PV_CONSOLE
void xil_printf( const char8 *ctrl1, ...)
{
	XPVXenConsole_Printf(ctrl1);
}
#else
void xil_printf( const char8 *ctrl1, ...)
{
	va_list argp;

	va_start(argp, ctrl1);

	xil_vprintf(ctrl1, argp);

	va_end(argp);
}
#endif

/**
 *@cond nocomments
 */
/*****************************************************************************/
/**
* This routine is equivalent to vprintf routine
******************************************************************************/
/**
 *@endcond
 */
void xil_vprintf(const char8 *ctrl1, va_list argp)
{
	char8 *ctrl = (char8 *)ctrl1;
	va_list local_argp;

	va_copy(local_argp, argp);

	while ((ctrl != NULL) && (*ctrl != (char8)0)) {

		/* move format string chars to buffer until a  */
		/* format control is found.                    */
		if (*ctrl != '%') {
#if defined(STDOUT_BASEADDRESS) || defined(VERSAL_PLM) || defined(SDT) || defined(SPARTANUP_PLM) || defined(ASUFW)
			outbyte(*ctrl);
#endif
			ctrl += 1;
			continue;
		}

		/* delegate all format specifier processing    */
		process_format_spec(&ctrl, &local_argp);
	}

	va_end(local_argp);
}
/*---------------------------------------------------*/
