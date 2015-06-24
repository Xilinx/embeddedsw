/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xhwicap_clb_ff.h
*
* This header file contains bit information about the CLB FF resource.
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
* </pre>
*
*****************************************************************************/
#ifndef XHWICAP_CLB_FF_H_  /* prevent circular inclusions */
#define XHWICAP_CLB_FF_H_  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */
/************************** Constant Definitions ****************************/

/**
 * Index into the CONTENTS and SRMODE for XQ Register.
 */
#define XHI_CLB_XQ 0

/**
 * Index into the CONTENTS and SRMODE for YQ Register.
 */
#define XHI_CLB_YQ 1

/**************************** Type Definitions ******************************/

typedef struct  {

	/**
	 * MODE values.
	 */
	const u8 LATCH[1];  /**< Value to put register into LATCH mode */
	const u8 FF[1];     /**< Value to put register into FF mode */

	/**
	 * CONTENTS values.
	 */
	const u8 INIT0[1];  /**< Value to initialize register CONTENTS to 0 */
	const u8 INIT1[1];  /**< Value to initialize register CONTENTS to 1 */
	const u8 ZERO[1];   /**< Same as INIT0 */
	const u8 ONE[1];    /**< Same as INIT1 */

	/**
	 * SRMODE values.
	 */
	const u8 SRLOW[1];  /**< When SR is asserted register goes to 0-Reset */
	const u8 SRHIGH[1]; /**< When SR is asserted register goes to 1-Set */

	/**
	 * SYNCMODE values.
	 */
	const u8 SYNC[1];  /**< Puts XQ and YQ in synchronous set/reset mode */
	const u8 ASYNC[1]; /**< Puts XQ and YQ in asynchronous set/reset mode */

	/**
	 * LATCH or FF mode.  Indexed by slice (0-3) only.
	 * It affects both XQ and YQ registers.
	 */
	const u8 MODE[4][1][2];

	/**
	 * SYNC or ASYNC mode.  Indexed by slice (0-3) only.
	 * It affects both  XQ and YQ registers.
	 */
	const u8 SYNCMODE[4][1][2];

	/**
	 * INIT0, INIT1, ONE, or ZERO.  Indexed by the slice basis (0-3).
	 * And then indexed by the element (XHI_CLB_XQ or XHI_CLB_YQ).
	 * INIT0 and ZERO are equivalent as well as INIT1 and ONE.  There
	 * are two values there only as to not confuse the values given in
	 * FPGA_EDITOR which are INIT0 and INIT1.  They both can either
	 * initialize or directly set the Register contents (assuming a
	 * GRESTORE packet command is used after doing a configuration on a
	 * device).
	 */
	const u8 CONTENTS[4][2][1][2];

	/**
	 * SRHIGH or SRLOW.  Indexed by the slice (0-3).
	 * And then indexed by the element (XHI_CLB_XQ or XHI_CLB_YQ)
	 */
	const u8 SRMODE[4][2][1][2];

} XHwIcap_ClbFf;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

/***************************************************************************/

/**
 *  This structure defines the bits associated with a Flip Flop in a CLB
 *  tile. Note that there are 8 FFs, the XQ and YQ Registers in
 *  Slice 0, 1, 2 and 3.
 */
const XHwIcap_ClbFf XHI_CLB_FF =
{
	{1}, 	/* LATCH*/
	{0}, 	/* FF*/
	{1}, 	/* INIT0*/
	{0},	/* INIT1*/
	{1}, 	/* ZERO*/
	{0}, 	/* ONE*/
	{1}, 	/* SRLOW*/
	{0},	/* SRHIGH*/
	{1},	/* SYNC*/
	{0},	/* ASYNC*/
	/* MODE*/
	{
		/* Slice 0. */
		{
			{10, 20}
		},
		/* Slice 1. */
		{
			{50, 20}
		},
		/* Slice 2. */
		{
			{22, 20}
		},
		/* Slice 3. */
		{
			{62, 20}
		}
	},
	/* SYNCMODE*/
	{
		/* Slice 0. */
		{
			 {26, 20}
		},
		/* Slice 1. */
		{
			 {66, 20}
		},
		/* Slice 2. */
		{
			 {25, 20}
		},
		/* Slice 3. */
		{
			 {65, 20}
		}
	},
	/* CONTENTS*/
	{
		/* Slice 0. */
		{
			/* LE 0. */
			{
				{6, 20}
			},
			/* LE 1. */
			{
				{34, 20}
			}
		},
		/* Slice 1. */
		{
			/* LE 0. */
			{
				{46, 20}
			},
			/* LE 1. */
			{
				{74, 20}
			}
		},
		/* Slice 2. */
		{
			/* LE 0. */
			{
				{5, 20}
			},
			/* LE 1. */
			{
				{33, 20}
			}
		},
		/* Slice 3. */
		{
			/* LE 0. */
			 {
			    {45, 20}
			 },
			 /* LE 1. */
			 {
			    {73, 20}
			 }
		}
	},
	/* SRMODE*/
	{
		/* Slice 0. */
		{
			/* LE 0. */
			{
				{0, 20}
			},
			/* LE 1. */
			{
				{30, 20}
			}
		},
		/* Slice 1. */
		{
			/* LE 0. */
			{
				{42, 20}
			},
			/* LE 1. */
			{
				{70, 20}
			}
		},
		/* Slice 2. */
		{
			/* LE 0. */
			{
				{1, 20}
			},
			/* LE 1. */
			{
				{29, 20}
			}
		},
		/* Slice 3. */
		{
			/* LE 0. */
			{
				{41, 20}
			},
			/* LE 1. */
			{
				{69, 20}
			}
		}
	},

};
#elif XHI_FAMILY == XHI_DEV_FAMILY_V5 /* Virtex5 */

/************************** Constant Definitions ****************************/

/**
 * Index into the CONTENTS and SRMODE for {A...D}Q Register.
 */
#define XHI_CLB_AQ 0
#define XHI_CLB_BQ 1
#define XHI_CLB_CQ 2
#define XHI_CLB_DQ 3


/**************************** Type Definitions ******************************/

typedef struct  {

	/**
	 * MODE values.
	 */
	const u8 LATCH[1];  /**< Value to put register into LATCH mode */
	const u8 FF[1];     /**< Value to put register into FF mode */

	/**
	 * CONTENTS values.
	 */
	const u8 INIT0[1];  /**< Value to initialize register CONTENTS to 0 */
	const u8 INIT1[1];  /**< Value to initialize register CONTENTS to 1 */
	const u8 ZERO[1];   /**< Same as INIT0 */
	const u8 ONE[1];    /**< Same as INIT1 */

	/**
	 * SRMODE values.
	 */
	const u8 SRLOW[1];  /**< When SR is asserted register goes to 0-Reset */
	const u8 SRHIGH[1]; /**< When SR is asserted register goes to 1-Set */

	/**
	 * SYNCMODE values.
	 */
	const u8 SYNC[1];  /**< Puts XQ and YQ in synchronous set/reset mode */
	const u8 ASYNC[1]; /**< Puts XQ and YQ in asynchronous set/reset mode */

	/**
	 * LATCH or FF mode.  Indexed by slice (0-1) only.
	 * It affects both XQ and YQ registers.
	 */
	const u8 MODE[2][4][2];

	/**
	 * SYNC or ASYNC mode.  Indexed by slice (0-1) only.
	 * It affects both  XQ and YQ registers.
	 */
	const u8 SYNCMODE[2][4][2];

	/**
	 * [type slice, L/M][num LUTS,4][num configs, 4]
	 * INIT0, INIT1, ONE, or ZERO.  Indexed by the slice basis (0-1).
	 * And then indexed by the element (XHI_CLB_AQ .. XHI_CLB_DQ).
	 * INIT0 and ZERO are equivalent as well as INIT1 and ONE.  There
	 * are two values there only as to not confuse the values given in
	 * FPGA_EDITOR which are INIT0 and INIT1.  They both can either
	 * initialize or directly set the Register contents (assuming a
	 * GRESTORE packet command is used after doing a configuration on a
	 * device).
	 */
	const u8 CONTENTS[2][4][1][2];

	/**
	 * [type slice, L/M][num LUTs 4][mode]
	 * SRHIGH or SRLOW.  Indexed by the slice (0-3).
	 * And then indexed by the element (XHI_CLB_AQ ... XHI_CLB_DQ)
	 */
	const u8 SRMODE[2][4][1][2];

} XHwIcap_ClbFf;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

/***************************************************************************/

/**
 *  This structure defines the bits associated with a Flip Flop in a CLB
 *  tile. Note that there are 8 FFs, the XQ and YQ Registers in
 *  Slice 0, 1, 2 and 3.
 */

const XHwIcap_ClbFf XHI_CLB_FF =
{
	{1}, 	/* LATCH*/
	{0}, 	/* FF*/
	{1}, 	/* INIT0*/
	{0},	/* INIT1*/
	{1}, 	/* ZERO*/
	{0}, 	/* ONE*/
	{1}, 	/* SRLOW*/
	{0},	/* SRHIGH*/
	{1},	/* SYNC*/
	{0},	/* ASYNC*/
	/* MODE*/
	{
		/* Slice 0. */
		{
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0}
		},
		/* Slice 1. */
		{
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0}

		}
	},
	/* SYNCMODE*/
	{
		/* Slice 0. */
		{
			 {6, 0},
			 {6, 0},
			 {6, 0},
			 {6, 0}
		},
		/* Slice 1. */
		{
			 {6, 0},
			 {6, 0},
			 {6, 0},
			 {6, 0}
		}
	},
	/* CONTENTS*/
	{
		/* Slice 0. */
		{
			/* LE 0. */
			{
				{6, 20}
			},
			/* LE 1. */
			{
				{6, 20}
			},
			/* LE 2. */
			{
				{6, 20}
			},
			/* LE 3. */
			{
				{6, 20}
			}
		},
		/* Slice 1. */
		{
			/* LE 0. */
			{
				{6, 20}
			},
			/* LE 1. */
			{
				{6, 20}
			},
			/* LE 2. */
			{
				{6, 20}
			},
			/* LE 3. */
			{
				{4, 20}
			}
		}
	},
	/* SRMODE*/
	{
		/* Slice 0. */
		{
			/* LE 0. */
			{
				{0, 20}
			},
			/* LE 1. */
			{
				{0, 20}
			},
			/* LE 2. */
			{
				{0, 20}
			},
			/* LE 3. */
			{
				{0, 20}
			}
		},
		/* Slice 1. */
		{
			/* LE 0. */
			{
				{2, 20}
			},
			/* LE 1. */
			{
				{2, 20}
			},
			/* LE 2. */
			{
				{2, 20}
			},
			/* LE 3. */
			{
				{0, 20}
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

