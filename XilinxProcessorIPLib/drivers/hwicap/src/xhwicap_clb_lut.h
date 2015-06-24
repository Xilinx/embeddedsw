/******************************************************************************
*
* Copyright (C) 2003 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xhwicap_clb_lut.h
*
* This header file contains bit information about the CLB LUT resource.
* This header file can be used with the XHwIcap_GetClbBits() and
* XHwIcap_SetClbBits() functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/14/03 First release
* 1.01a bjb  04/10/06 V4 Support
* 2.00a ecm  10/20/07 V5 Support
* 4.00a hvm  11/13/09 V6 Support
* </pre>
*
*****************************************************************************/
#ifndef XHWICAP_CLB_LUT_H_  /* prevent circular inclusions */
#define XHWICAP_CLB_LUT_H_  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */

/************************** Constant Definitions ****************************/

/**
 * Index into SLICE and MODE for F LUT.
 */
#define XHI_CLB_LUT_F 0

/**
 * Index into SLICE and MODE for G LUT.
 */
#define XHI_CLB_LUT_G 1

/**************************** Type Definitions ******************************/


typedef struct {
	/**
	 * MODE resource values.
	 */
	const u8 LUT_MODE[1];	/**< Set MODE to LUT mode */
	const u8 ROM_MODE[1];	/**< Set MODE to ROM mode.
					  * (Same as LUT mode) */
	const u8 RAM_MODE[1];	/**< Set MODE to RAM mode. */

	/**
	 * CONFIG resource values.
	 */
	const u8 SHIFT_CONFIG[2];	/**< Set CONFIG to shfiter. */
	const u8 RAM_CONFIG[2];	/**< Set CONFIG to ram. */
	const u8 LUT_CONFIG[2];	/**< Set CONFIG to LUT. */

	/**
	 * RAM_MODE, ROM_MODE, or LUT_MODE.  Indexed by the slice (0-3).  If
	 * only one LUT is in RAM or SHIFT mode, it MUST be the G LUT.
	 */
	const u8 MODE[4][1][2];

	/**
	 * SHIFT_CONFIG, RAM_CONFIG, or LUT_CONFIG.  Indexed by the slice
	 * (0-3).  And then indexed by the logic element (LUT.F or LUT.G).
	 * Note that if the F LUT is in any sort of ram or shifter modes,
	 * the G LUT must also be in ram or shifter mode.  Also, be sure to
	 * set the MODE bit appropriately.
	 */
	const u8 CONFIG[4][2][2][2];

	/**
	 * LUT memory contents. Indexed by slice first (0-3) and by
	 * XHI_CLB_LUT_F or XHI_CLB_LUT_G second.
	 */
	const u8 CONTENTS[4][2][16][2];

} XHwIcap_ClbLut;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*  This structure defines the Look Up Tables, or <em>LUTs</em>.
*  in the Virtex4 CLB.  Note that there are 8 16-bit
*  LUTs, the F and G LUTs in Slice 0, 1, 2 and 3.  These
*  LUTs can take any arbitrary bit pattern.
*
*  <p>
*
*  Note, that DUAL_PORT mode cannot be configured here.  Thats because
*  it is essentially always in effect. But, it can only be used in the top
*  two slices (2 and 3) using the address lines from the bottom
*  two slices (0 and 1) for the write address. Although you can technically
*  put the bottom two slice LUTs in dual port mode in the fpga_editor,
*  the read and write addresses will always be the same.  This is
*  different from the Virtex where the two LUTs in a slice were
*  combined to make a dual port RAM.  In Virtex4, every LUT is
*  dual ported, but only the top two have different read/write
*  addresses.
*
***************************************************************************/
const XHwIcap_ClbLut XHI_CLB_LUT =
{

	{0},    /* LUT_MODE*/
	{0},    /* ROM_MODE*/
	{1},    /* RAM_MODE*/
	{0,1},  /* SHIFT_CONFIG*/
	{1,0}, 	/* RAM_CONFIG*/
	{0,0}, 	/* LUT_CONFIG*/
	/* MODE*/
	{
		/* Slice 0. */
		{
			{38, 20}
		},
		/* Slice 1. */
		{
			{79, 20}
		},
		/* Slice 2. */
		{
			/* No MODE for SLICE_L's, LUT only. */
		},
		/* Slice 3. */
		{
			/* No MODE for SLICE_L's, LUT only. */
		}
	},
	/* CONFIG*/
	{
		/* Slice 0. */
		{
		 	/* LE 0. */
		 	{
		 		{8, 20}, {37, 20}
		 	},
		 	/* LE 1. */
		 	{
				{7, 20}, {36, 20}
		 	}
		},
		/* Slice 1. */
		{
			 /* LE 0. */
			{
				{48, 20}, {78, 20}
	 		},
			 /* LE 1. */
			{
				{40, 20}, {78, 20}
			}
		},
		/* Slice 2. */
		{
		 	/* LE 0. */
		 	{
		 	 /* No CONFIG for SLICE_L's, LUT only. */
		 	},
		 	/* LE 1. */
		 	{
		 	 /* No CONFIG for SLICE_L's, LUT only. */
		 	}
		},
		/* Slice 3. */
		{
		 	/* LE 0. */
		 	{
		 	 /* No CONFIG for SLICE_L's, LUT only. */
		 	},
		 	/* LE 1. */
		 	{
		 	 /* No CONFIG for SLICE_L's, LUT only. */
		 	}
		}
	},
	/* CONTENTS*/
	{
		/* Slice 0. */
		{
		 	/* LE 0. */
			{
				{15, 21}, {14, 21}, {13, 21}, {12, 21},
				{11, 21}, {10, 21}, {9, 21}, {8, 21},
				{7, 21}, {6, 21}, {5, 21}, {4, 21},
				{3, 21}, {2, 21}, {1, 21}, {0, 21}
			},
			/* LE 1. */
			{
				{38, 21}, {37, 21}, {36, 21}, {35, 21},
				{34, 21}, {33, 21}, {32, 21}, {31, 21},
				{30, 21}, {29, 21}, {28, 21}, {27, 21},
				{26, 21}, {25, 21}, {24, 21}, {23, 21}
			}
		},
		/* Slice 1. */
		{
			 /* LE 0. */
			 {
				{55, 21}, {54, 21}, {53, 21}, {52, 21},
				{51, 21}, {50, 21}, {49, 21}, {48, 21},
				{47, 21}, {46, 21}, {45, 21}, {44, 21},
				{43, 21}, {42, 21}, {41, 21}, {40, 21}
			 },
			/* LE 1. */
			{
				{78, 21}, {77, 21}, {76, 21}, {75, 21},
				{74, 21}, {73, 21}, {72, 21}, {71, 21},
				{70, 21}, {69, 21}, {68, 21}, {67, 21},
				{66, 21}, {65, 21}, {64, 21}, {63, 21}
			}
		},
		/* Slice 2. */
		{
			 /* LE 0. */
			 {
				{15, 19}, {14, 19}, {13, 19}, {12, 19},
				{11, 19}, {10, 19}, {9, 19}, {8, 19},
				{7, 19}, {6, 19}, {5, 19}, {4, 19},
				{3, 19}, {2, 19}, {1, 19}, {0, 19}
			 },
			 /* LE 1. */
			 {
				{38, 19}, {37, 19}, {36, 19}, {35, 19},
				{34, 19}, {33, 19}, {32, 19}, {31, 19},
				{30, 19}, {29, 19}, {28, 19}, {27, 19},
				{26, 19}, {25, 19}, {24, 19}, {23, 19}
			 }
		},
		/* Slice 3. */
		{
			/* LE 0. */
			{
				{55, 19}, {54, 19}, {53, 19}, {52, 19},
				{51, 19}, {50, 19}, {49, 19}, {48, 19},
				{47, 19}, {46, 19}, {45, 19}, {44, 19},
				{43, 19}, {42, 19}, {41, 19}, {40, 19}
			},
		 	/* LE 1. */
		 	{
				{78, 19}, {77, 19}, {76, 19}, {75, 19},
				{74, 19}, {73, 19}, {72, 19}, {71, 19},
				{70, 19}, {69, 19}, {68, 19}, {67, 19},
				{66, 19}, {65, 19}, {64, 19}, {63, 19}
		 	}
		}
	},

};

#elif XHI_FAMILY == XHI_DEV_FAMILY_V5 /* Virtex5 */

/************************** Constant Definitions ****************************/

/**
 * Index into SLICE and MODE for TYPE, L or M.
 */
#define XHI_CLB_TYPE_L 0
#define XHI_CLB_TYPE_M 1


/**
 * Index into SLICE and MODE for LUT A...D.
 */
#define XHI_CLB_LUT_A 0
#define XHI_CLB_LUT_B 1
#define XHI_CLB_LUT_C 2
#define XHI_CLB_LUT_D 3


/**************************** Type Definitions ******************************/

typedef struct {
	/**
	 * MODE resource values.
	 */
	const u8 LUT_MODE[1];	/**< Set MODE to LUT mode */
	const u8 ROM_MODE[1];	/**< Set MODE to ROM mode.
					  * (Same as LUT mode) */
	const u8 RAM_MODE[1];	/**< Set MODE to RAM mode. */

	/**
	 * CONFIG resource values.
	 */
	const u8 SHIFT_CONFIG16[2];	/**< Set CONFIG to shifter SRL16 */
	const u8 SHIFT_CONFIG32[2];	/**< Set CONFIG to shifter SRL32 */
	const u8 RAM_CONFIG_SP[2];	/**< Set CONFIG to ram, single port,
						64/32. */
	const u8 RAM_CONFIG_DP[2];	/**< Set CONFIG to ram. dual port,
						64/32.*/
	const u8 LUT_CONFIG[2];		/**< Set CONFIG to LUT. */

	/**
	 * [num slice, 0/1{L/M}][num LUTS,4][(lut/ram)/srl]
	 * RAM_MODE, ROM_MODE, or LUT_MODE.  Indexed by the slice (0,1).
	 */
	const u8 MODE[2][4][2][2];

	/**
	 * [type slice, L/M][num LUTS,4][num configs, 4]
	 * SHIFT_CONFIG, RAM_CONFIG, or LUT_CONFIG.  Indexed by the slice
	 * (0-1).  And then indexed by the LUT number (A=0 ... D=3).
	 */
	const u8 CONFIG[2][4][4][2];

	/**
	 * [type slice, L/M][num LUTs 4][contents]
	 * LUT memory contents. Indexed by slice first (0-1) and by
	 * LUT number {A=0...D=3}.
	 */
	const u8 CONTENTS[2][4][64][2];

} XHwIcap_ClbLut;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*  This structure defines the Look Up Tables, or <em>LUTs</em>.
*  in the Virtex5 CLB.  Note that there are 8 16-bit
*  LUTs, the L and M LUTs in Slice 0, and 1.  These
*  LUTs can take any arbitrary bit pattern.
*
*
*
***************************************************************************/

const XHwIcap_ClbLut XHI_CLB_LUT =
{
   /* LUT_MODE*/
   {0},
   /* ROM_MODE*/
   {0},
   /* RAM_MODE*/
   {1},
   /* SHIFT_CONFIG 16 */
   {4, 1},
   /* SHIFT_CONFIG 32 */
   {4,  1},
   /* RAM_CONFIG SP */
   {4,  1},
   /* RAM_CONFIG DP */
   {4,  1},
   /* LUT_CONFIG*/
   {4,  1},
   /* MODE*/
   {
      /* Type 0, Slice  L.*/
      {
         /* LE 0. LUTA, LUT/RAM mode, SRL mode */
         {
            {4,  1}, {5, 1}
         },
         /* LE 1. LUTB LUT/RAM mode, SRL mode */
         {
            {4,  15}, {5, 13}
         },
         /* LE 2. LUTC LUT/RAM mode, SRL mode */
         {
            {4,  31}, {5, 32}
         },
         /* LE 3. LUTD LUT/RAM mode, SRL mode */
         {
            {4 , 38}, {5, 37}
         }
      },
      /* Type 1, Slice M */
      {
         /* LE 0. LUTA, LUT/RAM mode, SRL mode */
         {
            {4,  1}, {5, 1}
         },
         /* LE 1. LUTB LUT/RAM mode, SRL mode */
         {
            {4,  15}, {5, 13}
         },
         /* LE 2. LUTC LUT/RAM mode, SRL mode */
         {
            {4,  31}, {5, 32}
         },
         /* LE 3. LUTD LUT/RAM mode, SRL mode */
         {
            {4,  38}, {5, 37}
         }
	  }
   },
   /* CONFIG*/
   {
      /* Type 0, Slice L. */
      {
         /* LE 0. LUTA*/
         {
            {8, 20}, {37, 20}, {8, 20}, {37, 20}
         },
         /* LE 1. LUTB*/
         {
            {8, 20}, {37, 20}, {8, 20}, {37, 20}
         },
         /* LE 2. LUTC*/
         {
            {8, 20}, {37, 20}, {8, 20}, {37, 20}
         },
         /* LE 3. LUTD*/
         {
            {7, 20}, {36, 20}, {8, 20}, {37, 20}
         }
      },
      /* Type 1, Slice M. */
      {
         /* LE 0. LUTA*/
         {
            {48, 20}, {78, 20}, {48, 20}, {78, 20}
         },
         /* LE 1. LUTB*/
         {
            {48, 20}, {78, 20}, {48, 20}, {78, 20}
         },
         /* LE 2. LUTC*/
         {
            {48, 20}, {78, 20}, {48, 20}, {78, 20}
         },
         /* LE 3. LUTD*/
         {
            {40, 20}, {78, 20}, {48, 20}, {78, 20}
         }
      }
   },
   /* CONTENTS*/
   {
      /* Type 0, Slice L. */
      {
         /* LE 0. LUTA*/
         {
			/* Icle_ll.ISL.LUTA */

			/* LSb to MSb, offset, minor */
			{15,3},{14,2},{13,3},{12,2},{11,3},{10,2},{9,3}, {8,2},
			{7,3}, {6,2}, {5,3}, {4,2}, {3,3}, {2,2}, {1,3}, {0,2},
			{15,2},{14,3},{13,2},{12,3},{11,2},{10,3},{9,2}, {8,3},
			{7,2}, {6,3}, {5,2}, {4,3}, {3,2}, {2,3}, {1,2}, {0,3},
			{15,0},{14,1},{13,0},{12,1},{11,0},{10,1},{9,0}, {8,1},
			{7,0}, {6,1}, {5,0}, {4,1}, {3,0}, {2,1}, {1,0}, {0,1},
			{15,1},{14,0},{13,1},{12,0},{11,1},{10,0},{9,1}, {8,0},
			{7,1}, {6,0}, {5,1}, {4,0}, {3,1}, {2,0}, {1,1}, {0,0}


  	 },
         /* LE 1. LUTB*/
         {
			/* Icle_ll.ISL.LUTB */

			/* LSb to MSb, offset, minor */
		{31, 3},{30, 2},{29, 3},{28, 2},{27, 3},{26, 2},{25, 3},{24, 2},
		{23, 3},{22, 2},{21, 3},{20, 2},{19, 3},{18, 2},{17, 3},{16, 2},
		{31, 2},{30, 3},{29, 2},{28, 3},{27, 2},{26, 3},{25, 2},{24, 3},
		{23, 2},{22, 3},{21, 2},{20, 3},{19, 2},{18, 3},{17, 2},{16, 3},
		{31, 0},{30, 1},{29, 0},{28, 1},{27, 0},{26, 1},{25, 0},{24, 1},
		{23, 0},{22, 1},{21, 0},{20, 1},{19, 0},{18, 1},{17, 0},{16, 1},
		{31, 1},{30, 0},{29, 1},{28, 0},{27, 1},{26, 0},{25, 1},{24, 0},
		{23, 1},{22, 0},{21, 1},{20, 0},{19, 1},{18, 0},{17, 1},{16, 0}

         },
         /* LE 2. LUTC*/
         {
			/* Icle_ll.ISL.LUTC */

			/* LSb to MSb, offset, minor */
		{47, 3},{46, 2},{45, 3},{44, 2},{43, 3},{42, 2},{41, 3},{40, 2},
		{39, 3},{38, 2},{37, 3},{36, 2},{35, 3},{34, 2},{33, 3},{32, 2},
		{47, 2},{46, 3},{45, 2},{44, 3},{43, 2},{42, 3},{41, 2},{40, 3},
		{39, 2},{38, 3},{37, 2},{36, 3},{35, 2},{34, 3},{33, 2},{32, 3},
		{47, 0},{46, 1},{45, 0},{44, 1},{43, 0},{42, 1},{41, 0},{40, 1},
		{39, 0},{38, 1},{37, 0},{36, 1},{35, 0},{34, 1},{33, 0},{32, 1},
		{47, 1},{46, 0},{45, 1},{44, 0},{43, 1},{42, 0},{41, 1},{40, 0},
		{39, 1},{38, 0},{37, 1},{36, 0},{35, 1},{34, 0},{33, 1},{32, 0}
         },
         /* LE 3. LUTD*/
         {
			/* Icle_ll.ISL.LUTD */

			/* LSb to MSb, offset, minor */
		{63, 3},{62, 2},{61, 3},{60, 2},{59, 3},{58, 2},{57, 3},{56, 2},
		{55, 3},{54, 2},{53, 3},{52, 2},{51, 3},{50, 2},{49, 3},{48, 2},
		{63, 2},{62, 3},{61, 2},{60, 3},{59, 2},{58, 3},{57, 2},{56, 3},
		{55, 2},{54, 3},{53, 2},{52, 3},{51, 2},{50, 3},{49, 2},{48, 3},
		{63, 0},{62, 1},{61, 0},{60, 1},{59, 0},{58, 1},{57, 0},{56, 1},
		{55, 0},{54, 1},{53, 0},{52, 1},{51, 0},{50, 1},{49, 0},{48, 1},
		{63, 1},{62, 0},{61, 1},{60, 0},{59, 1},{58, 0},{57, 1},{56, 0},
		{55, 1},{54, 0},{53, 1},{52, 0},{51, 1},{50, 0},{49, 1},{48, 0}

         }
      },
      /* Type 1. Slice M */
      {
         /* LE 0. LUTA*/
         {
			/* Icle_lm.ISL.LUTA */


			/* LSb to MSb, offset, minor */
			{15,3},{14,2},{13,3},{12,2},{11,3},{10,2},{9,3}, {8,2},
			{7,3}, {6,2}, {5,3}, {4,2}, {3,3}, {2,2}, {1,3}, {0,2},
			{15,2},{14,3},{13,2},{12,3},{11,2},{10,3},{9,2}, {8,3},
			{7,2}, {6,3}, {5,2}, {4,3}, {3,2}, {2,3}, {1,2}, {0,3},
			{15,0},{14,1},{13,0},{12,1},{11,0},{10,1},{9,0}, {8,1},
			{7,0}, {6,1}, {5,0}, {4,1}, {3,0}, {2,1}, {1,0}, {0,1},
			{15,1},{14,0},{13,1},{12,0},{11,1},{10,0},{9,1}, {8,0},
			{7,1}, {6,0}, {5,1}, {4,0}, {3,1}, {2,0}, {1,1}, {0,0}
		},
         /* LE 1. LUTB*/
         {
			/* Icle_lm.ISL.LUTB */

			/* LSb to MSb, offset, minor */
		{31, 3},{30, 2},{29, 3},{28, 2},{27, 3},{26, 2},{25, 3},{24, 2},
		{23, 3},{22, 2},{21, 3},{20, 2},{19, 3},{18, 2},{17, 3},{16, 2},
		{31, 2},{30, 3},{29, 2},{28, 3},{27, 2},{26, 3},{25, 2},{24, 3},
		{23, 2},{22, 3},{21, 2},{20, 3},{19, 2},{18, 3},{17, 2},{16, 3},
		{31, 0},{30, 1},{29, 0},{28, 1},{27, 0},{26, 1},{25, 0},{24, 1},
		{23, 0},{22, 1},{21, 0},{20, 1},{19, 0},{18, 1},{17, 0},{16, 1},
		{31, 1},{30, 0},{29, 1},{28, 0},{27, 1},{26, 0},{25, 1},{24, 0},
		{23, 1},{22, 0},{21, 1},{20, 0},{19, 1},{18, 0},{17, 1},{16, 0}
         },
         /* LE 2. LUTC*/
         {
			/* Icle_lm.ISL.LUTC */

			/* LSb to MSb, offset, minor */
		{47, 3},{46, 2},{45, 3},{44, 2},{43, 3},{42, 2},{41, 3},{40, 2},
		{39, 3},{38, 2},{37, 3},{36, 2},{35, 3},{34, 2},{33, 3},{32, 2},
		{47, 2},{46, 3},{45, 2},{44, 3},{43, 2},{42, 3},{41, 2},{40, 3},
		{39, 2},{38, 3},{37, 2},{36, 3},{35, 2},{34, 3},{33, 2},{32, 3},
		{47, 0},{46, 1},{45, 0},{44, 1},{43, 0},{42, 1},{41, 0},{40, 1},
		{39, 0},{38, 1},{37, 0},{36, 1},{35, 0},{34, 1},{33, 0},{32, 1},
		{47, 1},{46, 0},{45, 1},{44, 0},{43, 1},{42, 0},{41, 1},{40, 0},
		{39, 1},{38, 0},{37, 1},{36, 0},{35, 1},{34, 0},{33, 1},{32, 0}

         },
         /* LE 3. LUTD*/
         {
			/* Icle_lm.ISL.LUTD */

			/* LSb to MSb, offset, minor */
		{63, 3},{62, 2},{61, 3},{60, 2},{59, 3},{58, 2},{57, 3},{56, 2},
		{55, 3},{54, 2},{53, 3},{52, 2},{51, 3},{50, 2},{49, 3},{48, 2},
		{63, 2},{62, 3},{61, 2},{60, 3},{59, 2},{58, 3},{57, 2},{56, 3},
		{55, 2},{54, 3},{53, 2},{52, 3},{51, 2},{50, 3},{49, 2},{48, 3},
		{63, 0},{62, 1},{61, 0},{60, 1},{59, 0},{58, 1},{57, 0},{56, 1},
		{55, 0},{54, 1},{53, 0},{52, 1},{51, 0},{50, 1},{49, 0},{48, 1},
		{63, 1},{62, 0},{61, 1},{60, 0},{59, 1},{58, 0},{57, 1},{56, 0},
		{55, 1},{54, 0},{53, 1},{52, 0},{51, 1},{50, 0},{49, 1},{48, 0}

         }
      }
   },

};

#elif XHI_FAMILY == XHI_DEV_FAMILY_V6 /* Virtex6 */

/************************** Constant Definitions ****************************/

/**
 * Index into SLICE and MODE for TYPE, L or M.
 */
#define XHI_CLB_TYPE_L 0
#define XHI_CLB_TYPE_M 1


/**
 * Index into SLICE and MODE for LUT A...D.
 */
#define XHI_CLB_LUT_A 0
#define XHI_CLB_LUT_B 1
#define XHI_CLB_LUT_C 2
#define XHI_CLB_LUT_D 3


/**************************** Type Definitions ******************************/

typedef struct {
	/**
	 * MODE resource values.
	 */
	const u8 LUT_MODE[1];	/**< Set MODE to LUT mode */
	const u8 ROM_MODE[1];	/**< Set MODE to ROM mode.
					  * (Same as LUT mode) */
	const u8 RAM_MODE[1];	/**< Set MODE to RAM mode. */

	/**
	 * CONFIG resource values.
	 */
	const u8 SHIFT_CONFIG16[2];	/**< Set CONFIG to shifter SRL16 */
	const u8 SHIFT_CONFIG32[2];	/**< Set CONFIG to shifter SRL32 */
	const u8 RAM_CONFIG_SP[2];	/**< Set CONFIG to ram, single port,
						64/32. */
	const u8 RAM_CONFIG_DP[2];	/**< Set CONFIG to ram. dual port,
						64/32.*/
	const u8 LUT_CONFIG[2];		/**< Set CONFIG to LUT. */

	/**
	 * [num slice, 0/1{L/M}][num LUTS,4][(lut/ram)/srl]
	 * RAM_MODE, ROM_MODE, or LUT_MODE.  Indexed by the slice (0,1).
	 */
	const u8 MODE[2][4][2][2];

	/**
	 * [type slice, L/M][num LUTS,4][num configs, 4]
	 * SHIFT_CONFIG, RAM_CONFIG, or LUT_CONFIG.  Indexed by the slice
	 * (0-1).  And then indexed by the LUT number (A=0 ... D=3).
	 */
	const u8 CONFIG[2][4][4][2];

	/**
	 * [type slice, L/M][num LUTs 4][contents]
	 * LUT memory contents. Indexed by slice first (0-1) and by
	 * LUT number {A=0...D=3}.
	 */
	const u8 CONTENTS[2][4][64][2];

} XHwIcap_ClbLut;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*  This structure defines the Look Up Tables, or <em>LUTs</em>.
*  in the Virtex6 CLB.  Note that there are 8 16-bit
*  LUTs, the L and M LUTs in Slice 0, and 1.  These
*  LUTs can take any arbitrary bit pattern.
*
*
*
***************************************************************************/

const XHwIcap_ClbLut XHI_CLB_LUT =
{
   /* LUT_MODE*/
   {0},
   /* ROM_MODE*/
   {0},
   /* RAM_MODE*/
   {1},
   /* SHIFT_CONFIG 16 */
   {4, 1},
   /* SHIFT_CONFIG 32 */
   {4,  1},
   /* RAM_CONFIG SP */
   {4,  1},
   /* RAM_CONFIG DP */
   {4,  1},
   /* LUT_CONFIG*/
   {4,  1},
   /* MODE*/
   {
      /* Type 0, Slice  L.*/
      {
         /* LE 0. LUTA, LUT/RAM mode, SRL mode */
         {
            {4,  1}, {5, 1}
         },
         /* LE 1. LUTB LUT/RAM mode, SRL mode */
         {
            {4,  15}, {5, 13}
         },
         /* LE 2. LUTC LUT/RAM mode, SRL mode */
         {
            {4,  31}, {5, 32}
         },
         /* LE 3. LUTD LUT/RAM mode, SRL mode */
         {
            {4 , 38}, {5, 37}
         }
      },
      /* Type 1, Slice M */
      {
         /* LE 0. LUTA, LUT/RAM mode, SRL mode */
         {
            {4,  1}, {5, 1}
         },
         /* LE 1. LUTB LUT/RAM mode, SRL mode */
         {
            {4,  15}, {5, 13}
         },
         /* LE 2. LUTC LUT/RAM mode, SRL mode */
         {
            {4,  31}, {5, 32}
         },
         /* LE 3. LUTD LUT/RAM mode, SRL mode */
         {
            {4,  38}, {5, 37}
         }
	  }
   },
   /* CONFIG*/
   {
      /* Type 0, Slice L. */
      {
         /* LE 0. LUTA*/
         {
            {8, 20}, {37, 20}, {8, 20}, {37, 20}
         },
         /* LE 1. LUTB*/
         {
            {8, 20}, {37, 20}, {8, 20}, {37, 20}
         },
         /* LE 2. LUTC*/
         {
            {8, 20}, {37, 20}, {8, 20}, {37, 20}
         },
         /* LE 3. LUTD*/
         {
            {7, 20}, {36, 20}, {8, 20}, {37, 20}
         }
      },
      /* Type 1, Slice M. */
      {
         /* LE 0. LUTA*/
         {
            {48, 20}, {78, 20}, {48, 20}, {78, 20}
         },
         /* LE 1. LUTB*/
         {
            {48, 20}, {78, 20}, {48, 20}, {78, 20}
         },
         /* LE 2. LUTC*/
         {
            {48, 20}, {78, 20}, {48, 20}, {78, 20}
         },
         /* LE 3. LUTD*/
         {
            {40, 20}, {78, 20}, {48, 20}, {78, 20}
         }
      }
   },
   /* CONTENTS*/
   {
      /* Type 0, Slice L. */
      {
         /* LE 0. LUTA*/
         {
			/* Icle_ll.ISL.LUTA */
	{15,1}, {13,1}, {15, 3},  {13,3},  {11,1},  {9,1},   {11,3}, {9,3},
	{7, 1}, {5, 1}, {7,  3},  {5,3},   {3,1},   {1,1},   {3,3},  {1,3},
	{15,0}, {13,0}, {15,2},   {13,2},  {11,0},  {9,0},   {11,2},  {9,2},
	{7,0},  {5,0},  {7,2},    {5,2},   {3,0},   {1,0},   {3,2},   {1,2},
	{14, 1},{12, 1},{14, 3},  {12,3},  {10,1},  {8,1},   {10,3},  {8,3},
	{6,1},  {4,1},  {6,3},    {4,3},   {2,1},    {0,1},   {2,3},   {0,3},
 	{14,0}, {12,0}, {14,2},   {12,2},  {10,0},  {8,0},   {10,2},  {8,2},
	{6,0},  {4,0},  {6,2},    {4,2},   {2,0},   {0,0},   {2,2},   {0,2}
         },
         /* LE 1. LUTB*/
         {
			/* Icle_ll.ISL.LUTB */
	{31, 1}, {29, 1}, {31, 3}, {29, 3}, {27, 1}, {25, 1}, {27, 3}, {25, 3},
	{23, 1}, {21, 1}, {23, 3}, {21, 3}, {19, 1}, {17, 1}, {19, 3}, {17, 3},
	{31, 0}, {29, 0}, {31, 2}, {29, 2}, {27, 0}, {25, 0}, {27, 2}, {25, 2},
	{23, 0}, {21, 0}, {23, 2}, {21, 2}, {19, 0}, {17, 0}, {19, 2}, {17, 2},
	{30, 1}, {28, 1}, {30, 3}, {28, 3}, {26, 1}, {24, 1}, {26, 3}, {24, 3},
	{22, 1}, {20, 1}, {22, 3}, {20, 3}, {18, 1}, {16, 1}, {18, 3}, {16, 3},
	{30, 0}, {28, 0}, {30, 2}, {28, 2}, {26, 0}, {24, 0}, {26, 2}, {24, 2},
	{22, 0}, {20, 0}, {22, 2}, {20, 2}, {18, 0}, {16, 0}, {18, 2}, {16, 2}
	 },
         /* LE 2. LUTC*/
         {
			/* Icle_ll.ISL.LUTC */
	{47, 1}, {45, 1}, {47, 3}, {45, 3}, {43, 1}, {41, 1}, {43, 3}, {41, 3},
	{39, 1}, {37, 1}, {39, 3}, {37, 3}, {35, 1}, {33, 1}, {35, 3}, {33, 3},
	{47, 0}, {45, 0}, {47, 2}, {45, 2}, {43, 0}, {41, 0}, {43, 2}, {41, 2},
	{39, 0}, {37, 0}, {39, 2}, {37, 2}, {35, 0}, {33, 0}, {35, 2}, {33, 2},
	{46, 1}, {44, 1}, {46, 3}, {44, 3}, {42, 1}, {40, 1}, {42, 3}, {40, 3},
	{38, 1}, {36, 1}, {38, 3}, {36, 3}, {34, 1}, {32, 1}, {34, 3}, {32, 3},
	{46, 0}, {44, 0}, {46, 2}, {44, 2}, {42, 0}, {40, 0}, {42, 2}, {40, 2},
	{38, 0}, {36, 0}, {38, 2}, {36, 2}, {34, 0}, {32, 0}, {34, 2}, {32, 2}
         },
         /* LE 3. LUTD*/
         {
			/* Icle_ll.ISL.LUTD */
	{63, 1}, {61, 1}, {63, 3}, {61, 3}, {59, 1}, {57, 1}, {59, 3}, {57, 3},
	{55, 1}, {53, 1}, {55, 3}, {53, 3}, {51, 1}, {49, 1}, {51, 3}, {49, 3},
	{63, 0}, {61, 0}, {63, 2}, {61, 2}, {59, 0}, {57, 0}, {59, 2}, {57, 2},
	{55, 0}, {53, 0}, {55, 2}, {53, 2}, {51, 0}, {49, 0}, {51, 2}, {49, 2},
	{62, 1}, {60, 1}, {62, 3}, {60, 3}, {58, 1}, {56, 1}, {58, 3}, {56, 3},
	{54, 1}, {52, 1}, {54, 3}, {52, 3}, {50, 1}, {48, 1}, {50, 3}, {48, 3},
	{62, 0}, {60, 0}, {62, 2}, {60, 2}, {58, 0}, {56, 0}, {58, 2}, {56, 2},
	{54, 0}, {52, 0}, {54, 2}, {52, 2}, {50, 0}, {48, 0}, {50, 2}, {48, 2}
         }
      },
      /* Type 1. Slice M */
      {
         /* LE 0. LUTA*/
         {
			/* Icle_lm.ISL.LUTA */
	{15, 9}, {13, 9}, {15, 6}, {13, 6}, {11, 9}, {9, 9}, {11, 6}, {9, 6},
	{7, 9},  {5, 9},  {7, 6},  {5, 6},  {3, 9},  {1, 9}, {3, 6},  {1, 6},
	{15, 8}, {13, 8}, {15, 7}, {13, 7}, {11, 8}, {9, 8}, {11, 7}, {9, 7},
	{7, 8},  {5, 8},  {7, 7},  {5, 7},  {3, 8},  {1, 8}, {3, 7},  {1, 7},
	{14, 9}, {12,6},  {14,6},  {12, 6}, {10, 9}, {8, 9}, {10, 6}, {8, 6},
	{6, 9},  {4, 9},  {6, 6},  {4, 6},  {2, 9},  {0, 9}, {2, 6},  {0, 6},
	{14, 8}, {12, 8}, {14, 7}, {12, 7}, {10, 8}, {8, 8}, {10, 7}, {8, 7},
	{6, 8},  {4, 8},  {6, 7},  {4, 7},  {2, 8},  {0, 8}, {2, 7},  {0, 7},         },
         /* LE 1. LUTB*/
         {
			/* Icle_lm.ISL.LUTB */
	{31, 8}, {29, 9}, {31, 6}, {29, 6}, {27, 9}, {25, 9}, {27, 6}, {25, 6},
	{23, 9}, {21, 9}, {23, 6}, {21, 6}, {19, 9}, {17, 9}, {19, 6}, {17, 6},
	{31, 8}, {29, 8}, {31, 7}, {29, 7}, {27, 8}, {25, 8}, {27, 7}, {25, 7},
	{23, 8}, {21, 8}, {23, 7}, {21, 7}, {19, 8}, {17, 8}, {19, 7}, {17, 7},
	{30, 9}, {28, 9}, {30, 6}, {28, 6}, {26, 9}, {24, 9}, {26, 6}, {24, 6},
	{22, 9}, {20, 9}, {22, 6}, {20, 6}, {18, 9}, {16, 9}, {18, 6}, {16, 6},
	{30, 8}, {28, 8}, {30, 7}, {28, 7}, {26, 8}, {24, 8}, {26, 7}, {24, 7},
	{22, 8}, {20, 8}, {22, 7}, {20, 7}, {18, 8}, {16, 8}, {18, 7}, {16, 7}
         },
         /* LE 2. LUTC*/
         {
			/* Icle_lm.ISL.LUTC */
	{47, 9}, {45, 9}, {47, 6}, {44, 6}, {43, 9}, {41, 9}, {43, 6}, {41, 6},
	{39, 9}, {37, 9}, {39, 6}, {37, 6}, {35, 9}, {33, 9}, {35, 6}, {33, 6},
	{47, 8}, {45, 8}, {47, 7}, {45, 7}, {43, 8}, {41, 8}, {43, 7}, {41, 7},
	{39, 8}, {37, 8}, {39, 7}, {37, 7}, {35, 8}, {33, 8}, {35, 7}, {33, 7},
	{46, 9}, {44, 9}, {46, 6}, {44, 6}, {42, 9}, {40, 9}, {42, 6}, {40, 6},
	{38, 9}, {36, 9}, {38, 6}, {36, 6}, {34, 9}, {32, 9}, {34, 6}, {32, 6},
	{46, 8}, {44, 8}, {46, 7}, {44, 7}, {42, 8}, {40, 8}, {42, 7}, {40, 7},
	{38, 8}, {36, 8}, {38, 7}, {36, 7}, {34, 8}, {32, 8}, {34, 7}, {32, 7}
         },
         /* LE 3. LUTD*/
         {
			/* Icle_lm.ISL.LUTD */
	{63, 9}, {61, 9}, {63, 6}, {61, 6}, {59, 9}, {57, 9}, {59, 6}, {57, 6},
	{55, 9}, {53, 9}, {55, 6}, {53, 6}, {51, 9}, {49, 9}, {51, 6}, {49, 6},
	{63, 8}, {61, 8}, {63, 7}, {61, 7}, {59, 8}, {57, 8}, {59, 7}, {57, 7},
	{55, 8}, {53, 8}, {55, 7}, {53, 7}, {51, 8}, {49, 8}, {51, 7}, {49, 7},
	{62, 9}, {60, 9}, {62, 6}, {60, 6}, {58, 9}, {56, 9}, {58, 6}, {56, 6},
	{54, 9}, {52, 9}, {54, 6}, {52, 6}, {50, 9}, {48, 9}, {50, 6}, {48, 6},
	{62, 8}, {60, 8}, {62, 7}, {60, 7}, {58, 8}, {56, 8}, {58, 7}, {56, 7},
	{54, 8}, {52, 8}, {54, 7}, {52, 7}, {50, 8}, {48, 8}, {50, 7}, {48, 7}
         }
      }
   },

};

#else

#error Unsupported FPGA Family

#endif

#ifdef __cplusplus
}
#endif

#endif

