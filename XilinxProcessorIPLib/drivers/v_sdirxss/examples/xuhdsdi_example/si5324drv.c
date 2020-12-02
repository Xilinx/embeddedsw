/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file si5324drv.c
 *
 * This file contains low-level driver functions for controlling the
 * SiliconLabs Si5324 clock generator as mounted on the ZCU106 demo board.
 * The user should refer to the hardware device specification for more details
 * of the device operation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date         Changes
 * ----- --- ----------   -----------------------------------------------
 * 1.00  hf  2014/10/10   First release
 * 1.10  MG  2016/07/05   Updated LOCKT register
 * 1.11  YH  2016/09/14   Add option to enable fast switching
 * </pre>
 *
 ****************************************************************************/

#include <stdlib.h>
#include "xil_types.h"
#include "xiic.h"
#include "si5324drv.h"
#include "xparameters.h"


/******************************************************************************
 * User settable constant that depends on the specific board design.
 * The defaults are for the Xilinx KC705 board.
 *****************************************************************************/

/**
 * Default register settings that differ from the (power-on-)reset values.
 * This array consists of pairs with the first value the register number and
 * the second number the register value.
 */
u8 SI5324_DEFAULTS[] = {
	/*
	 * Disable output clocks during calibration (bit 4 SQ_ICAL=1),
	 * other bits are default
	 */
	3, 0x15,
	/*
	 *  Auto select clock (automatic revertive) (bit 7:6 AUTOSEL_REG)10
	 *  History delay default
	 */
	4, 0x92,
	/*
	 *  Disable CKOUT2 (SFOUT2_REG=001)
	 *  set CKOUT1 to LVDS (SFOUT1_REG=111)
	 * (default is LVPECL for both)
	 */
	6, 0x0F,
	/*
	 *  Enable CKOUT1 output (bit 2 DSBL1_REG = 1)
	 *  disable CKOUT2 output (bit 3 DSBL2_REG=0)
	 */
	10, 0x08,
	/*
	 * Disable CKIN2 input buffer (bit 1 PD_CK2=1)
	 * enable CKIN1 buffer (bit 0 PD_CK1=0)
	 * (bit 6 is reserved, write default value)
	 */
	11, 0x42,
	/*
	 * Set lock time to 53ms as recommended (bits 2:0 LOCKT=001)
	 * other bits are default
	 */
	19, 0x2f,  /* 0x2f, 0x29 */
	/* Enable fast locking (bit 0 FASTLOCK=1) */
	137, 0x01   /* FASTLOCK=1 (enable fast locking) */
};

/******************************************************************************
 * Definitions independent on the specific board design. Should not be changed.
 *****************************************************************************/

/*****************************************************************************/
/**
 * Send a list of register settings to the Si5324 clock generator.
 *
 * @param	IICBaseAddress contains the base address of the IIC master
 *		device.
 * @param	IICAddress contains the 7 bit IIC address of the Si5324 device.
 * @param	BufPtr is a pointer to an array with alternating register addresses
 *		and register values to program into the Si5324. The array length
 *		must be at least 2*NumRegs.
 * @param	NumRegs contains the number of registers to write.
 *
 * @return	SI5324_SUCCESS for success, SI5324_ERR_IIC for IIC access failure,
 *		SI5324_ERR_PARM when the number of registers to write is less than
 *		one.
 *
 * @note	Private function. Does not modify the contents of the buffer
 *		pointed to by BufPtr.
 *****************************************************************************/
int Si5324_DoSettings(u32 IICBaseAddress, u8 IICAddress,
		u8 *BufPtr, int NumRegs) {
	int result;
	int i;

	/* Check the number of registers to write. It must be at least one. */
	if (NumRegs < 1) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: Illegal number of registers"
					" to write.");
		}
		return SI5324_ERR_PARM;
	}
	for (i = 0; i < NumRegs; i++) {
		result = XIic_Send(IICBaseAddress, IICAddress,
					BufPtr + (i << 1), 2,
					XIIC_STOP);
		if (result != 2) {
			if (SI5324_DEBUG) {
				xil_printf("Si5324: ERROR: IIC write request"
						" error.");
			}
			return SI5324_ERR_IIC;
		}
	}
	return SI5324_SUCCESS;
}

/*****************************************************************************/
/**
 * Reset the SiliconLabs Si5324 clock generator.
 *
 * @param    IICBaseAddress contains the base address of the IIC master
 *           device.
 * @param    IICAddress contains the 7 bit IIC address of the Si5324 device.
 *
 * @return   SI5324_SUCCESS for success, SI5324_ERR_IIC for IIC access failure.
 *
 * @note     Private function.
 *****************************************************************************/
int Si5324_Reset(u32 IICBaseAddress, u8 IICAddress) {
	int result;
	u8 buf[2];

	if (SI5324_DEBUG) {
		xil_printf("Resetting Si5324.\n");
	}
	buf[0] = 136;  /* Register number */
	buf[1] = 0x80; /* bit 7 = RST_REG = 1: start of reset */
	result = Si5324_DoSettings(IICBaseAddress, IICAddress, buf, 2);
	if (result != SI5324_SUCCESS) {
		/* Not all bytes were sent: IIC error occurred */
		return SI5324_ERR_IIC;
	}
	/*
	 * Should wait here for minimum reset active time (this time is not
	 * documented in the Si5324 data sheet). Since IIC is slow, assume that
	 * enough time has passed before the next IIC command is finished.
	 */
	buf[1] = 0x00; /* bit 7 = RST_REG = 0: end of reset */
	result = Si5324_DoSettings(IICBaseAddress, IICAddress, buf, 2);
	if (result != SI5324_SUCCESS) {
		/* Not all bytes were sent: IIC error occurred */
		return SI5324_ERR_IIC;
	}
	return SI5324_SUCCESS;
}

/*****************************************************************************/
/**
 * Initialize the SiliconLabs Si5324 clock generator. After initialization,
 * the clock generator is not generating a clock yet. Call si5324_set_clock
 * to start the clock generator.
 *
 * @param    IICBaseAddress contains the base address of the IIC master
 *           device.
 * @param    IICAddress contains the 7 bit IIC address of the Si5324 device.
 *
 * @return   SI5324_SUCCESS for success, SI5324_ERR_IIC for IIC access failure.
 *****************************************************************************/
int Si5324_Init(u32 IICBaseAddress, u8 IICAddress) {
	int result;

	if (SI5324_DEBUG) {
		xil_printf("Si5324: Initializing.\n");
	}
	result = Si5324_DoSettings(IICBaseAddress, IICAddress,
					SI5324_DEFAULTS,
					sizeof(SI5324_DEFAULTS) / 2);
	return result;
}

/*****************************************************************************/
/**
 * Find the closest rational approximation for the N2_LS/N3 fraction.
 *
 * @param	f  Holds the N2_LS/N3 fraction in 36.28 fixed point notation.
 * @param	md Holds the maximum denominator (N3) value allowed.
 * @param	num Will store the numinator (N2_LS) found.
 * @param	denom Will store the denominator (N3) found.
 */
void Si5324_RatApprox(u64 f, u64 md, u32 *num, u32 *denom)
{
	/*  a: Continued fraction coefficients. */
	u64 a, h[3] = { 0, 1, 0 }, k[3] = { 1, 0, 0 };
	u64 x, d, n = 1;
	int i = 0;

	/* Degenerate case: only n/1 solution allowed. Return trunc(f)/1. */
	if (md <= 1) {
		*denom = 1;
		*num = (u32)(f >> 28);
		return;
	}

	/*
	 * Multiply fraction until there are no
	 * more digits after decimal point
	 */
	n <<= 28;
	for (i = 0; i < 28; i++) {
		if ((f & 0x1) == 0) {
			n >>= 1;
			f >>= 1;
		} else {
			break;
		}
	}
	d = f;

	/* Continued fraction and check denominator each step */
	for (i = 0; i < 64; i++) {
		a = n ? d / n : 0;
		if (i && !a) {
			break;
		}

		x = d;
		d = n;
		n = x % n;

		x = a;
		if (k[1] * a + k[0] >= md) {
			x = (md - k[0]) / k[1];
			if (x * 2 >= a || k[1] >= md) {
				i = 65;
			} else {
				break;
			}
		}

		h[2] = x * h[1] + h[0];
		h[0] = h[1];
		h[1] = h[2];
		k[2] = x * k[1] + k[0];
		k[0] = k[1];
		k[1] = k[2];
	}
	*denom = (u32)k[1];
	*num   = (u32)h[1];
}

/*****************************************************************************/
/**
 * Search through the possible settings for the N2_LS parameter. Finds the best
 * setting for N2_LS and N3n with the values for N1_HS, NCn_LS, and N2_HS
 * already set in settings.
 *
 * @param	settings Holds the settings up till now.
 * @return	1 when the best possible result has been found.
 * @note	Private function.
 */
int Si5324_FindN2ls(si5324_settings_t *settings) {
	u32 result = 0;
	u64 f3_actual;
	u64 fosc_actual;
	u64 fout_actual;
	u64 delta_fout;
	u64 n2_ls_div_n3;
	u32 mult;

	n2_ls_div_n3 = settings->fosc / (settings->fin >> 28) / settings->n2_hs / 2;
	Si5324_RatApprox(n2_ls_div_n3,
			settings->n3_max,
			&(settings->n2_ls),
			&(settings->n3));
	settings->n2_ls *= 2;

	/*
	 * Rational approximation returns the smalles ratio possible.
	 * Upscaling might be needed when when one or both of the
	 * numbers are too low.
	 */
	if (settings->n2_ls < settings->n2_ls_min) {
		mult =  settings->n2_ls_min / settings->n2_ls;
		mult = (settings->n2_ls_min % settings->n2_ls) ? mult + 1 : mult;
		settings->n2_ls *= mult;
		settings->n3    *= mult;
	}
	if (settings->n3 < settings->n3_min) {
		mult =  settings->n3_min / settings->n3;
		mult = (settings->n3_min % settings->n3) ? mult + 1 : mult;
		settings->n2_ls *= mult;
		settings->n3    *= mult;
	}
	if (SI5324_DEBUG) {
		xil_printf("\t\t\tTrying N2_LS = %d N3 = %d.\n", settings->n2_ls, settings->n3);
	}
	/* Check if N2_LS and N3 are within the required ranges */
	if ((settings->n2_ls < settings->n2_ls_min) ||
			(settings->n2_ls > settings->n2_ls_max)) {
		xil_printf("\t\t\tN2_LS out of range.\n");
	} else if ((settings->n3 < settings->n3_min) ||
			(settings->n3 > settings->n3_max)) {
		xil_printf("\t\t\tN3 out of range.\n");
	}
	else {
		/*
		 * N2_LS and N3 values within range:
		 * check actual output frequency
		 */
		f3_actual = settings->fin / settings->n3;
		fosc_actual = f3_actual * settings->n2_hs * settings->n2_ls;
		fout_actual = fosc_actual / (settings->n1_hs * settings->nc_ls);
		delta_fout = fout_actual - settings->fout;
		/* Check actual frequencies for validity */
		if ((f3_actual < ((u64)SI5324_F3_MIN) << 28) ||
				(f3_actual > ((u64)SI5324_F3_MAX) << 28)) {
			if (SI5324_DEBUG) {
				xil_printf("\t\t\tF3 frequency out of range.\n");
			}
		} else if ((fosc_actual < ((u64)SI5324_FOSC_MIN) << 28) ||
				(fosc_actual > ((u64)SI5324_FOSC_MAX) << 28)) {
			if (SI5324_DEBUG) {
				xil_printf("\t\t\tFosc frequency out of range.\n");
			}
		} else if ((fout_actual < ((u64)SI5324_FOUT_MIN) << 28) ||
				(fout_actual >((u64)SI5324_FOUT_MAX) << 28)) {
			if (SI5324_DEBUG) {
				xil_printf("\t\t\tFout frequency out of range.\n");
			}
		} else {
			if (SI5324_DEBUG) {
				xil_printf("\t\t\tFound solution: "
						"fout = %dHz delta = %dHz.\n",
						(u32)(fout_actual >> 28),
						(u32)(delta_fout >> 28));
				xil_printf("\t\t\t                "
						"fosc = %dkHz f3 = %dHz.\n",
						(u32)((fosc_actual >> 28) / 1000),
						(u32)(f3_actual >> 28));
			}
			if (((u64)llabs(delta_fout)) < settings->best_delta_fout) {
				/* Found a better solution: remember this one! */
				if (SI5324_DEBUG) {
					xil_printf("\t\t\tThis solution is the best yet!\n");
				}
				settings->best_n1_hs = settings->n1_hs;
				settings->best_nc_ls = settings->nc_ls;
				settings->best_n2_hs = settings->n2_hs;
				settings->best_n2_ls = settings->n2_ls;
				settings->best_n3 = settings->n3;
				settings->best_fout = fout_actual;
				settings->best_delta_fout = llabs(delta_fout);
				if (delta_fout == 0) {
					/*
					 * Best possible result found.
					 * Skip the rest of the possibilities.
					 */
					result = 1;
				}
			}
		}
	}
	return result;
}

/*****************************************************************************/
/**
 * Find a valid setting for N2_HS and N2_LS. Finds the best
 * setting for N2_HS, N2_LS, and N3n with the values for N1_HS, and NCn_LS
 * already set in settings. Iterates over all possibilities
 * of N2_HS and then performs a binary search over the N2_LS values.
 *
 * @param settings Holds the settings up till now.
 * @return 1 when the best possible result has been found.
 * @note     Private function.
 *****************************************************************************/
int Si5324_FindN2(si5324_settings_t *settings) {
	u32 result;

	for (settings->n2_hs = SI5324_N2_HS_MAX; \
			settings->n2_hs >= SI5324_N2_HS_MIN; \
			settings->n2_hs--) {
		if (SI5324_DEBUG) {
			xil_printf("\t\tTrying N2_HS = %d.\n", settings->n2_hs);
		}
		settings->n2_ls_min = (u32)(settings->fosc / ((u64)(SI5324_F3_MAX * settings->n2_hs) << 28));
		if (settings->n2_ls_min < SI5324_N2_LS_MIN) {
			settings->n2_ls_min = SI5324_N2_LS_MIN;
		}
		settings->n2_ls_max = (u32)(settings->fosc / ((u64)(SI5324_F3_MIN * settings->n2_hs) << 28));
		if (settings->n2_ls_max > SI5324_N2_LS_MAX) {
			settings->n2_ls_max = SI5324_N2_LS_MAX;
		}
		result = Si5324_FindN2ls(settings);
		if (result) {
			/*
			 * Best possible result found.
			 * Skip the rest of the possibilities.
			 */
			break;
		}
	}
	return result;
}

/*****************************************************************************/
/**
 * Calculates the valid range for NCn_LS with the value for the output
 * frequency and N1_HS already set in settings.
 *
 * @param	settings Holds the input and output frequencies and the setting
 *                 for N1_HS.
 * @return	-1 when there are no valid settings for NCn_LS, 0 otherwise.
 * @note	Private function.
 */
int Si5324_CalcNclsLimits(si5324_settings_t *settings) {
	/* Calculate limits for NCn_LS */
	settings->nc_ls_min = settings->n1_min / settings->n1_hs;
	if (settings->nc_ls_min < SI5324_NC_LS_MIN) {
		settings->nc_ls_min = SI5324_NC_LS_MIN;
	}
	/* Make sure NC_ls_min is one or even */
	if ((settings->nc_ls_min > 1) && ((settings->nc_ls_min & 0x1) == 1)) {
		settings->nc_ls_min++;
	}
	settings->nc_ls_max = settings->n1_max / settings->n1_hs;
	if (settings->nc_ls_max > SI5324_NC_LS_MAX) {
		settings->nc_ls_max = SI5324_NC_LS_MAX;
	}
	/* Make sure NC_ls_max is even */
	if ((settings->nc_ls_max & 0x1) == 1) {
		settings->nc_ls_max--;
	}
	/* Check if actual N1 is within limits */
	if ((settings->nc_ls_max * settings->n1_hs < settings->n1_min) ||
			(settings->nc_ls_min * settings->n1_hs > settings->n1_max)) {
		/* No valid NC_ls possible: take next N1_hs */
		return -1;
	}
	return 0;
}

/*****************************************************************************/
/**
 * Find a valid setting for NCn_LS that can deliver the correct output
 * frequency. Assumes that the valid range is relatively small so a full search
 * can be done (should be true for video clock frequencies).
 *
 * @param	settings Holds the input and output frequencies, the setting for
 *                 N1_HS, and the limits for NCn_LS.
 * @return	1 when the best possible result has been found.
 * @note	Private function.
 */
int Si5324_FindNcls(si5324_settings_t *settings) {
	u64 fosc_1;
	u32 result;

	fosc_1 = settings->fout * settings->n1_hs;
	for (settings->nc_ls = settings->nc_ls_min; \
			settings->nc_ls <= settings->nc_ls_max;) {
		settings->fosc = fosc_1 * settings->nc_ls;
		if (SI5324_DEBUG) {
			xil_printf("\tTrying NCn_LS = %d: fosc = %dkHz.\n",
					settings->nc_ls,
					(u32)((settings->fosc >> 28) / 1000));
		}
		result = Si5324_FindN2(settings);
		if (result) {
			/*
			 * Best possible result found.
			 * Skip the rest of the possibilities.
			 */
			break;
		}
		if (settings->nc_ls == 1) {
			settings->nc_ls++;
		} else {
			settings->nc_ls += 2;
		}
	}
	return result;
}

/*****************************************************************************/
/**
 * Calculate the frequency settings for the desired output frequency.
 *
 * @param	ClkInFreq contains the frequency of the input clock.
 * @param	ClkOutFreq contains the desired output clock frequency.
 * @param	N1_hs  will be set to the value for the N1_HS register.
 * @param	NCn_ls will be set to the value for the NCn_LS register.
 * @param	N2_hs  will be set to the value for the N2_HS register.
 * @param	N2_ls  will be set to the value for the N2_LS register.
 * @param	N3n    will be set to the value for the N3n register.
 * @param	BwSel  will be set to the value for the BW_SEL register.
 *
 * @return	SI5324_SUCCESS for success, SI5324_ERR_FREQ when the requested
 *		frequency cannot be generated.
 * @note	Private function.
 *****************************************************************************/
int Si5324_CalcFreqSettings(u32 ClkInFreq, u32 ClkOutFreq,
		u8  *N1_hs, u32 *NCn_ls,
		u8  *N2_hs, u32 *N2_ls,
		u32 *N3n,   u8  *BwSel) {
	/* TBD */
	si5324_settings_t settings;
	int result;

	/* 32.28 fixed point */
	settings.fin = (u64)ClkInFreq  << 28;
	/* 32.28 fixed point */
	settings.fout= (u64)ClkOutFreq << 28;
	/* High frequency error to start with */
	settings.best_delta_fout = settings.fout;

	/* Calculate some limits for N1_HS * NCn_LS and for N3 base on the input */
	/* and output frequencies. */
	settings.n1_min = (int)(SI5324_FOSC_MIN / ClkOutFreq);
	if (settings.n1_min < SI5324_N1_HS_MIN * SI5324_NC_LS_MIN) {
		settings.n1_min = SI5324_N1_HS_MIN * SI5324_NC_LS_MIN;
	}
	settings.n1_max = (int)(SI5324_FOSC_MAX / ClkOutFreq);
	if (settings.n1_max > SI5324_N1_HS_MAX * SI5324_NC_LS_MAX) {
		settings.n1_max = SI5324_N1_HS_MAX * SI5324_NC_LS_MAX;
	}
	settings.n3_min = ClkInFreq / SI5324_F3_MAX;
	if (settings.n3_min < SI5324_N3_MIN) {
		settings.n3_min = SI5324_N3_MIN;
	}
	settings.n3_max = ClkInFreq / SI5324_F3_MIN;
	if (settings.n3_max > SI5324_N3_MAX) {
		settings.n3_max = SI5324_N3_MAX;
	}
	/* Find a valid oscillator frequency with the highest setting of N1_HS */
	/* possible (reduces power) */
	for (settings.n1_hs = SI5324_N1_HS_MAX; \
			settings.n1_hs >= SI5324_N1_HS_MIN; \
			settings.n1_hs--) {
		if (SI5324_DEBUG) {
			xil_printf("Trying N1_HS = %d.\n", settings.n1_hs);
		}
		result = Si5324_CalcNclsLimits(&settings);
		if (result) {
			if (SI5324_DEBUG) {
				xil_printf("\tNo valid settings for NCn_LS.\n");
			}
			continue;
		}
		result = Si5324_FindNcls(&settings);
		if (result) {
			/*
			 * Best possible result found.
			 * Skip the rest of the possibilities.
			 */
			break;
		}
	}
	if (settings.best_delta_fout /= settings.fout) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: No valid settings found.");
		}
		return SI5324_ERR_FREQ;
	}
	if (SI5324_DEBUG) {
		xil_printf("Si5324: Found solution: fout = %dHz.\n",
				(u32)(settings.best_fout >> 28));
	}

	/*
	 * Post processing: convert temporary values
	 * to actual register settings
	 */
	*N1_hs  = (u8)settings.best_n1_hs - 4;
	*NCn_ls =     settings.best_nc_ls - 1;
	*N2_hs  = (u8)settings.best_n2_hs - 4;
	*N2_ls  =     settings.best_n2_ls - 1;
	*N3n    =     settings.best_n3    - 1;
	/* How must the bandwidth selection be determined?
	 * Not all settings will be valid.
	 * refclk        2, 0xA2,  //              BWSEL_REG=1010 (?)
	 * free running  2, 0x42,  //              BWSEL_REG=0100 (?)
	 */
	*BwSel  = 6; /* 4 */
	return SI5324_SUCCESS;
}

/*****************************************************************************/
/**
 * Set the output frequency of the Si5324 clock generator.
 *
 * @param	IICBaseAddress contains the base address of the IIC master
 *		device.
 * @param	IICAddress contains the 7 bit IIC address of the Si5324 device.
 * @param	ClkSrc selects the clock input to use.
 * @param	ClkInFreq contains the frequency of the input clock (2kHz-710MHz).
 * @param	ClkOutFreq contains the desired output clock frequency
 *		(2kHz-945MHz).
 *
 * @return	SI5324_SUCCESS for success, SI5324_ERR_IIC for IIC access failure,
 *		SI5324_ERR_FREQ when the requested frequency cannot be generated,
 *		SI5324_ERR_PARM when the ClkSrc or ClkDest parameters are invalid
 *		or the ClkInFreq or ClkOutFreq are out of range.
 *****************************************************************************/
int Si5324_SetClock(u32 IICBaseAddress, u8 IICAddress, u8 ClkSrc,
		u32 ClkInFreq, u32 ClkOutFreq) {
	u32 NCn_ls, N2_ls, N3n;
	u8  N1_hs, N2_hs, BwSel;
	int result;
	u8  buf[15*2]; /* Need to set 15 registers */
	int i;

	/* Sanity check */
	if ((ClkSrc < SI5324_CLKSRC_CLK1) || (ClkSrc > SI5324_CLKSRC_XTAL)) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: Incorrect input clock selected!\n");
		}
		return SI5324_ERR_PARM;
	}
	if (ClkSrc == SI5324_CLKSRC_CLK2) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: Clock input 2 not supported!");
		}
		return SI5324_ERR_PARM;
	}
	if ((ClkInFreq < SI5324_FIN_MIN) || (ClkInFreq > SI5324_FIN_MAX)) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: Input frequency out of range!\n");
		}
		return SI5324_ERR_PARM;
	}
	if ((ClkOutFreq < SI5324_FOUT_MIN) || (ClkOutFreq > SI5324_FOUT_MAX)) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: Output frequency out of"
					" range!\n");
		}
		return SI5324_ERR_PARM;
	}

	/* Calculate the frequency settings for the Si5324 */
	result = Si5324_CalcFreqSettings(ClkInFreq, ClkOutFreq,
			&N1_hs, &NCn_ls, &N2_hs, &N2_ls, &N3n,
			&BwSel);
	if (result != SI5324_SUCCESS) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: ERROR: Could not determine settings"
					" for requested frequency!\n");
		}
		return result;
	}

	/* Set the clock settings */
	if (SI5324_DEBUG) {
		xil_printf("Si5324: Programming frequency settings.\n");
	}
	i = 0;

	/* Free running mode or use a reference clock */
	buf[i] = 0;
	if (ClkSrc == SI5324_CLKSRC_XTAL) {
		/* Enable free running mode */
		buf[i+1] = 0x54;
	} else {
		/* Disable free running mode */
		buf[i+1] = 0x14;
	}
	i += 2;

	/* Loop bandwidth */
	buf[i]   = 2;
	buf[i+1] = (BwSel << 4) | 0x02;
	i += 2;

	/* Enable reference clock 2 in free running mode */
	buf[i] = 11;
	if (ClkSrc == SI5324_CLKSRC_CLK1) {
		/* Disable input clock 2 */
		buf[i+1] = 0x42;
	} else {
		/* Enable input clock 2 */
		buf[i+1] = 0x40;
	}
	i += 2;

	/* N1_HS */
	buf[i]   = 25;
	buf[i+1] = N1_hs << 5;
	i += 2;

	/* NC1_LS */
	buf[i]   = 31;
	buf[i+1] = (u8)((NCn_ls & 0x000F0000) >> 16);
	buf[i+2] = 32;
	buf[i+3] = (u8)((NCn_ls & 0x0000FF00) >>  8);
	buf[i+4] = 33;
	buf[i+5] = (u8)( NCn_ls & 0x000000FF       );
	i += 6;

	/* N2_HS and N2_LS */
	buf[i]    = 40;
	buf[i+1]  = (N2_hs << 5);
	/* N2_LS upper bits (same register as N2_HS) */
	buf[i+1] |= (u8)((N2_ls & 0x000F0000) >> 16);
	buf[i+2]  = 41;
	buf[i+3]  = (u8)((N2_ls & 0x0000FF00) >>  8);
	buf[i+4]  = 42;
	buf[i+5]  = (u8)( N2_ls & 0x000000FF       );
	i += 6;

	/* N3n */
	if (ClkSrc == SI5324_CLKSRC_CLK1) {
		/* N31 */
		buf[i]   = 43;
		buf[i+2] = 44;
		buf[i+4] = 45;
	} else {
		/* N32 */
		buf[i]   = 46;
		buf[i+2] = 47;
		buf[i+4] = 48;
	}
	buf[i+1] = (u8)((N3n & 0x00070000) >> 16);
	buf[i+3] = (u8)((N3n & 0x0000FF00) >>  8);
	buf[i+5] = (u8)( N3n & 0x000000FF       );
	i += 6;

	/* Start calibration */
	buf[i]   = 137;
	buf[i+1] = 0x01;
	i += 2;

	/* Start calibration */
	buf[i]   = 136;
	buf[i+1] = 0x40;
	i += 2;

	/* Sanity check */
	if (i != sizeof(buf)) {
		if (SI5324_DEBUG) {
			xil_printf("Si5324: FATAL ERROR: Incorrect buffer size");
			xil_printf(" while programming frequency settings");
		}
		exit(-1);
	}

	/* Send all register settings to the Si5324 */
	result = Si5324_DoSettings(IICBaseAddress, IICAddress,
			buf, i / 2);
	return result;
}

/*****************************************************************************/
/**
 * Send a list of register settings to the Si5324 clock generator.
 *
 * @param	IICBaseAddress contains the base address of the IIC master
 *		device.
 * @param	IICAddress1 contains the 7 bit IIC address of the Si5324 device.
 * @param	BufPtr is a pointer to an array with alternating register addresses
 *		and register values to program into the Si5324. The array length
 *		must be at least 2*NumRegs.
 * @param	NumRegs contains the number of registers to write.
 *
 * @return	SI5324_SUCCESS for success, SI5324_ERR_IIC for IIC access failure,SI5324_ERR_PARM when the number of registers to write is less than
 *		SI5324_ERR_PARM when the number of registers to write is less than
 *
 * @note	Private function. Does not modify the contents of the buffer
 *		pointed to by BufPtr.
 *****************************************************************************/
int Si570_DoSettings(u32 IICBaseAddress, u8 IICAddress1,
		u8 *BufPtr, int NumRegs) {
	int result;
	int i;

	/* Check the number of registers to write. It must be at least one. */
	if (NumRegs < 1) {
		if (SI5324_DEBUG) {
			xil_printf("Si570: ERROR: Illegal number of registers to write.");
		}
		return SI5324_ERR_PARM;
	}
	for (i = 0; i < NumRegs; i++) {
		result = XIic_Send(IICBaseAddress, IICAddress1,
					BufPtr + (i << 1), 2,
					XIIC_STOP);
		if (result != 2) {
			if (SI5324_DEBUG) {
				xil_printf("Si570: ERROR: IIC write request error.");
			}
			return SI5324_ERR_IIC;
		}
	}
	return SI5324_SUCCESS;
}

/*****************************************************************************/
/**
 * Set the output frequency of the Si570 clock generator.
 *
 * @param	IICBaseAddress contains the base address of the IIC master
 *		device.
 * @param	IICAddress1 contains the 7 bit IIC address of the Si5324 device.
 * @param	RxRefClk is the input reference clock that can be either of
 * 		FREQ_SI570_148_5_MHz, FREQ_SI570_148_35_MHz
 *
 * @return	SI5324_SUCCESS for success, SI5324_ERR_IIC for IIC access failure,
 *		SI5324_ERR_FREQ when the requested frequency cannot be generated,
 *		SI5324_ERR_PARM when the ClkSrc or ClkDest parameters are invalid
 *		or the ClkInFreq or ClkOutFreq are out of range.
 *****************************************************************************/

/*
 * Set the Si570 clock to 148.5MHz
 */
int Si570_SetClock(u32 IICBaseAddress, u8 IICAddress1, u32 RxRefClk) {

	int result;
	u8  buf[9*2]; /* Need to set 8 registers */
	int i;

	/*
	 * Set the clock settings
	 */
	if (SI5324_DEBUG) {
		xil_printf("Si570: Programming frequency settings.\n");
	}
	i = 0;

	/* Free running mode or use a reference clock */
	buf[i] = 137;
	buf[i+1] = 0x10;

	i += 2;

	buf[i] = 7;
	buf[i+1] = 0x01;

	i += 2;

	buf[i] = 8;
	buf[i+1] = 0xC2;

	i += 2;

	buf[i] = 9;
	buf[i+1] = 0x99;

	i += 2;

	buf[i] = 10;
	buf[i+1] = 0x48;

	i += 2;

	buf[i] = 11;
	switch(RxRefClk) {
	case FREQ_SI570_148_5_MHz:
		buf[i+1] = 0x26;
		break;
	case FREQ_SI570_148_35_MHz:
		buf[i+1] = 0x10;
		break;
	default:
		buf[i+1] = 0x26;
		break;
	}

	i += 2;

	buf[i] = 12;
	buf[i+1] = 0xC4;

	i += 2;

	buf[i] = 137;
	buf[i+1] = 0x00;

	i +=2;

	buf[i] = 135;
	buf[i+1] = 0x40;

	i += 2;

	/* Send all register settings to the Si5324 */
	result = Si570_DoSettings(IICBaseAddress, IICAddress1, buf, i / 2);
	return result;
}
