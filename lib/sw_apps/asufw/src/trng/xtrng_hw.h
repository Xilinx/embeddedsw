/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrng_hw.h
 *
 * This header file contains macros for TRNG HW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/20/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XTRNG_HW_H
#define XTRNG_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/
/* Definitions for TRNG driver */
#define XASU_XTRNG_NUM_INSTANCES					1U

/* Canonical Definitions for peripheral PSV_ASU_TRNG */
#define XASU_XTRNG_0_DEVICE_ID						0U
#define XASU_XTRNG_0_S_AXI_BASEADDR					0xEBF20000U
#define XASU_XTRNG_0_FIFO_S_AXI_BASEADDR			0xEBF10000U

/**< TRNG Interrupt status register offset */
#define XASU_TRNG_INTR_STS_OFFSET			        (0x00000000U)
/**< Interrupt status register trng_ac mask */
#define XASU_TRNG_INTR_STS_TRNG_AC_MASK		        (0x00000100U)

/**< TRNG NRN AVAIL register offset */
#define XASU_TRNG_NRN_AVAIL_OFFSET		            (0x00000018U)

/**< TRNG reset register offset */
#define XASU_TRNG_RESET_OFFSET			            (0x0000001CU)
/**< TRNG reset register value mask */
#define XASU_TRNG_RESET_VAL_MASK		            (0x00000001U)
/**< TRNG reset register default value */
#define XASU_TRNG_RESET_DEFVAL			            (0x1U)

/**< TRNG OSC Enable register offset */
#define XASU_TRNG_OSC_EN_OFFSET				        (0x00000020U)
/**< TRNG OSC Enable register val mask */
#define XASU_TRNG_OSC_EN_VAL_MASK			        (0x00000001U)
/**< TRNG OSC Enable register val default value */
#define XASU_TRNG_OSC_EN_VAL_DEFVAL			        (0x0U)

/**< TRNG AUTOPROC register offset */
#define XASU_TRNG_AUTOPROC_OFFSET		            (0x00000028U)
/**< TRNG AUTOPROC regsiter enable mask */
#define XASU_TRNG_AUTOPROC_ENABLE_MASK	            (0x1U)
/**< TRNG AUTOPROC register disable mask */
#define XASU_TRNG_AUTOPROC_DISABLE_MASK	            (0x0U)

/**< TRNG NRNPS register offset */
#define XASU_TRNG_NRNPS_OFFSET			            (0x0000002CU)

/**< TRNG status register offset */
#define XASU_TRNG_STATUS_OFFSET				        (0x00001004U)
/**< TRNG status register QCNT mask */
#define XASU_TRNG_STATUS_QCNT_MASK			        (0x00000e00U)
/**< TRNG status register QCNT shift value */
#define XASU_TRNG_STATUS_QCNT_SHIFT			        (9U)
/**< TRNG status register DFT mask */
#define XASU_TRNG_STATUS_DFT_MASK			        (0x00000002U)
/**< TRNG status register Done mask */
#define XASU_TRNG_STATUS_DONE_MASK			        (0x00000001U)
/**< TRNG status register CERTF mask */
#define XASU_TRNG_STATUS_CERTF_MASK			        (0x00000008U)

/**< TRNG control register offset */
#define XASU_TRNG_CTRL_OFFSET				        (0x00001008U)
/**< TRNG control register PRNG Mode mask */
#define XASU_TRNG_CTRL_PRNGMODE_MASK		        (0x00000080U)
/**< TRNG control register single mode mask */
#define XASU_TRNG_CTRL_SINGLEGENMODE_MASK	        (0x00000200U)
/**< TRNG control register PRNG start mask */
#define XASU_TRNG_CTRL_PRNGSTART_MASK		        (0x00000020U)
/**< TRNG control register TRSSEN mask */
#define XASU_TRNG_CTRL_TRSSEN_MASK			        (0x00000004U)
/**< TRNG control register PERSODISABLE mask */
#define XASU_TRNG_CTRL_PERSODISABLE_MASK	        (0x00000400U)
/**< TRNG control register PERSODISABLE default value */
#define XASU_TRNG_CTRL_PERSODISABLE_DEFVAL	        (0x0U)
/**< TRNG control register TSTMode mask */
#define XASU_TRNG_CTRL_TSTMODE_MASK			        (0x00000040U)
/**< TRNG control register PRNGXS mask */
#define XASU_TRNG_CTRL_PRNGXS_MASK			        (0x00000008U)
/**< TRNG control register EU Mode mask */
#define XASU_TRNG_CTRL_EUMODE_MASK			        (0x00000100U)
/**< TRNG control register PRNG SRST mask */
#define XASU_TRNG_CTRL_PRNGSRST_MASK	            (0x00000001U)

/**< TRNG CONF0 register offset */
#define XASU_TRNG_CONF0_OFFSET						(0x0000100CU)
/**< TRNG CONF0 DIT mask */
#define XASU_TRNG_CONF0_DIT_MASK					(0x0000001fU)
/**< TRNG CONF0 DIT shift value */
#define XASU_TRNG_CONF0_DIT_SHIFT					(0U)
/**< TRNG CONF0 REPCOUNTTESTCUTOFF mask */
#define XASU_TRNG_CONF0_REPCOUNTTESTCUTOFF_MASK		(0x0001ff00U)
/**< TRNG CONF0 REPCOUNTTESTCUTOFF shift value */
#define XASU_TRNG_CONF0_REPCOUNTTESTCUTOFF_SHIFT	(8U)
/**< TRNG CONF0 DIT default value */
#define XASU_TRNG_CONF0_DIT_DEFVAL					(0xcU)

/**< TRNG CONF1 register offset */
#define XASU_TRNG_CONF1_OFFSET				        (0x00001010U)
/**< TRNG CONF1 register DLEN mask */
#define XASU_TRNG_CONF1_DLEN_MASK			        (0x000000ffU)
/**< TRNG CONF1 register DLEN shift value */
#define XASU_TRNG_CONF1_DLEN_SHIFT			        (0U)
/**< TRNG CONF1 regsiter ADAPTPROTESTCUTOFF mask */
#define XASU_TRNG_CONF1_ADAPTPROPTESTCUTOFF_MASK	(0x0003ff00U)
/**< TRNG CONF1 ADAPTPROPTESTCUTOFF shift value */
#define XASU_TRNG_CONF1_ADAPTPROPTESTCUTOFF_SHIFT	(8U)

/**< TRNG Test register offset */
#define XASU_TRNG_TEST_OFFSET				        (0x00001014U)

/**< TRNG Per String 11 register offset */
#define XASU_TRNG_PER_STRNG_11_OFFSET		        (0x000010ACU)

/**< TRNG core output register offset */
#define XASU_TRNG_CORE_OUTPUT_OFFSET		        (0x000010C0U)

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XTRNG_HW_H */