/*
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#include "xpfw_config.h"

#ifdef ENABLE_IOCTL
#include "pm_ioctl.h"
#include "pm_common.h"
#ifdef ENABLE_DYNAMIC_MIO_CONFIG
#include "iou_slcr.h"
#include "iou_secure_slcr.h"
#include "lpd_slcr_secure.h"
#include "pm_defs.h"
#include "pm_pinctrl.h"
#endif
#ifdef ENABLE_FEATURE_CONFIG
#ifdef ENABLE_RUNTIME_OVERTEMP
#include "xpfw_mod_overtemp.h"
#endif
#ifdef ENABLE_RUNTIME_EXTWDT
#include "xpfw_mod_extwdt.h"
#endif
#endif

#ifdef ENABLE_DYNAMIC_MIO_CONFIG
#define SET_FIXED_SD_CONFIG(id)							\
	XPfw_RMW32(SD_CONFIG_REG2, SD##id##_3P0V_MASK, 0U);			\
	XPfw_RMW32(SD_CONFIG_REG3, SD##id##_RETUNETMR_MASK, 0U);		\
	XPfw_RMW32(SD_DLL_CTRL, SD##id##_DLL_RST_DIS_MASK,			\
		   (u32)1U << SD##id##_DLL_RST_DIS_SHIFT);			\
	XPfw_RMW32(SD_CONFIG_REG1, SD##id##_TUNIGCOUNT_MASK,			\
		   (u32)40U << SD##id##_TUNIGCOUNT_SHIFT);			\
	XPfw_RMW32(IOU_COHERENT_CTRL, SD##id##_AXI_COH_MASK,			\
		   (u32)1U << SD##id##_AXI_COH_SHIFT);				\
	XPfw_RMW32(IOU_INTERCONNECT_ROUTE, SD##id##_INTERCONNECT_ROUTE_MASK,	\
		   (u32)1U << SD##id##_INTERCONNECT_ROUTE_SHIFT);		\
	XPfw_RMW32(IOU_AXI_WPRTCN, SD##id##_AXI_AWPROT_MASK,			\
		   (u32)2U << SD##id##_AXI_AWPROT_SHIFT);			\
	XPfw_RMW32(IOU_AXI_RPRTCN, SD##id##_AXI_ARPROT_MASK,			\
		   (u32)2U << SD##id##_AXI_ARPROT_SHIFT);

#define SET_FIXED_GEM_CONFIG(id)						\
	XPfw_RMW32(IOU_COHERENT_CTRL, GEM##id##_AXI_COH_MASK,			\
		   (u32)1U << GEM##id##_AXI_COH_SHIFT);				\
	XPfw_RMW32(IOU_INTERCONNECT_ROUTE, GEM##id##_INTERCONNECT_ROUTE_MASK,	\
		   (u32)1U << GEM##id##_INTERCONNECT_ROUTE_SHIFT);		\
	XPfw_RMW32(IOU_AXI_WPRTCN, GEM##id##_AXI_AWPROT_MASK,			\
		   (u32)2U << GEM##id##_AXI_AWPROT_SHIFT);			\
	XPfw_RMW32(IOU_AXI_RPRTCN, GEM##id##_AXI_ARPROT_MASK,			\
		   (u32)2U << GEM##id##_AXI_ARPROT_SHIFT);

#define SET_FIXED_USB_CONFIG(id)						\
	XPfw_RMW32(SLCR_USB, TZ_USB3_##id##_MASK, (u32)1U << TZ_USB3_##id##_SHIFT);
#endif /* ENABLE_DYNAMIC_MIO_CONFIG */

#ifdef ENABLE_FEATURE_CONFIG
#ifdef ENABLE_RUNTIME_OVERTEMP
static u32 OverTempState = 0U;
#endif /* ENABLE_RUNTIME_OVERTEMP */

#ifdef ENABLE_RUNTIME_EXTWDT
static u32 ExtWdtState = 0U;
#endif /* ENABLE_RUNTIME_EXTWDT */

/**
 * PmSetFeatureConfig() - The feature can be configured by using IOCTL.
 * @configId	The config id of the feature to be configured
 * @value	The value to be configured
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error
 *		code or a reason code
 */
s32 PmSetFeatureConfig(XPm_FeatureConfigId configId, u32 value)
{
	s32 status = XST_FAILURE;

	switch (configId) {
#ifdef ENABLE_RUNTIME_OVERTEMP
	case XPM_FEATURE_OVERTEMP_STATUS:
		if ((1U == value) && (0U == OverTempState)) {
			/* Initialize over temperature */
			status = OverTempCfgInit();
			OverTempState = 1U;
		} else if ((0U == value) && (1U == OverTempState)) {
			/* De-initialize over temperature */
			status = OverTempCfgDeInit();
			OverTempState = 0U;
		} else {
			status = XST_INVALID_PARAM;
		}
		break;
	case XPM_FEATURE_OVERTEMP_VALUE:
		if (((s32)value >= OT_LIMIT_MIN) && ((s32)value <= OT_LIMIT_MAX)) {
			SetOverTempLimit(value);
			status = XST_SUCCESS;
		} else {
			status = XST_INVALID_PARAM;
		}
		break;
#endif /* ENABLE_RUNTIME_OVERTEMP */
#ifdef ENABLE_RUNTIME_EXTWDT
	case XPM_FEATURE_EXTWDT_STATUS:
		if ((1U == value) && (0U == ExtWdtState)) {
			/* Initialize external watchdog */
			status = ExtWdtCfgInit();
			ExtWdtState = 1U;
		} else if ((0U == value) && (1U == ExtWdtState)) {
			/* De-initialize external watchdog */
			status = ExtWdtCfgDeInit();
			ExtWdtState = 0U;
		} else {
			status = XST_INVALID_PARAM;
		}
		break;
	case XPM_FEATURE_EXTWDT_VALUE:
		if ((value >= EWDT_LIMIT_MIN) && (value <= EWDT_LIMIT_MAX)) {
			SetExtWdtInterval(value);
			status = XST_SUCCESS;
		} else {
			status = XST_INVALID_PARAM;
		}
		break;
#endif /* ENABLE_RUNTIME_EXTWDT */
	default:
		status = XST_INVALID_PARAM;
		break;
	}

	return status;
}

/**
 * PmGetFeatureConfig() - Get the configured value of thee feature.
 * @configId	The config id of the feature to be queried
 * @value	return by reference value
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error
 * 		code or a reason code
 */
s32 PmGetFeatureConfig(XPm_FeatureConfigId configId, u32 *value)
{
	s32 status = XST_FAILURE;

	switch (configId) {
#ifdef ENABLE_RUNTIME_OVERTEMP
	case XPM_FEATURE_OVERTEMP_STATUS:
		*value = OverTempState;
		status = XST_SUCCESS;
		break;
	case XPM_FEATURE_OVERTEMP_VALUE:
		*value = GetOverTempLimit();
		status = XST_SUCCESS;
		break;
#endif /* ENABLE_RUNTIME_OVERTEMP */
#ifdef ENABLE_RUNTIME_EXTWDT
	case XPM_FEATURE_EXTWDT_STATUS:
		*value = ExtWdtState;
		status = XST_SUCCESS;
		break;
	case XPM_FEATURE_EXTWDT_VALUE:
		*value = GetExtWdtInterval();
		status = XST_SUCCESS;
		break;
#endif /* ENABLE_RUNTIME_EXTWDT */
	default:
		status = XST_INVALID_PARAM;
		break;
	}

	return status;
}
#endif /* ENABLE_FEATURE_CONFIG */

#ifdef ENABLE_DYNAMIC_MIO_CONFIG
static inline s32 PmConfigureSd0Regs(void)
{
	u32 fId = 0U;
	s32 status = XST_FAILURE;

	SET_FIXED_SD_CONFIG(0);

	XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_FBCLK_SEL_MASK,
		   (u32)1U << SDIO0_FBCLK_SEL_SHIFT);
	status = PmPinCtrlGetFunctionInt(38U, &fId);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (PINCTRL_FUNC_SDIO0 == fId) {
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_RX_SRC_SEL_MASK,
			   (u32)1U << SDIO0_RX_SRC_SEL_SHIFT);
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_FBCLK_SEL_MASK, 0U);
	}
	status = PmPinCtrlGetFunctionInt(64U, &fId);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (PINCTRL_FUNC_SDIO0 == fId) {
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_RX_SRC_SEL_MASK,
			   (u32)2U << SDIO0_RX_SRC_SEL_SHIFT);
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_FBCLK_SEL_MASK, 0U);
	}
	status = PmPinCtrlGetFunctionInt(22U, &fId);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (PINCTRL_FUNC_SDIO0 == fId) {
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_RX_SRC_SEL_MASK, 0U);
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO0_FBCLK_SEL_MASK, 0U);
	}

done:
	return status;
}

static inline s32 PmConfigureSd1Regs(void)
{
	u32 fId = 0U;
	s32 status = XST_FAILURE;

	SET_FIXED_SD_CONFIG(1);

	XPfw_RMW32(SDIO_CLK_CTRL, SDIO1_FBCLK_SEL_MASK,
		   (u32)1U << SDIO1_FBCLK_SEL_SHIFT);
	status = PmPinCtrlGetFunctionInt(76U, &fId);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (PINCTRL_FUNC_SDIO1 == fId) {
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO1_RX_SRC_SEL_MASK,
			   (u32)1U << SDIO1_RX_SRC_SEL_SHIFT);
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO1_FBCLK_SEL_MASK, 0U);
	}
	status = PmPinCtrlGetFunctionInt(51U, &fId);
	if (XST_SUCCESS != status) {
		goto done;
	}

	if (PINCTRL_FUNC_SDIO1 == fId) {
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO1_RX_SRC_SEL_MASK, 0U);
		XPfw_RMW32(SDIO_CLK_CTRL, SDIO1_FBCLK_SEL_MASK, 0U);
	}

done:
	return status;
}

/**
 * PmSetSdConfig() - Configure SD registers.
 * @nodeId	SD node ID
 * @configType	configuration type
 * @value	value to be written
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error
 *		code or a reason code
 */
s32 PmSetSdConfig(u32 nodeId, XPm_SdConfigType configType, u32 value)
{
	s32 status = XST_FAILURE;

	if ((NODE_SD_0 != nodeId) && (NODE_SD_1 != nodeId)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	switch(configType) {
	case SD_CONFIG_EMMC_SEL:
		if (NODE_SD_0 == nodeId) {
			XPfw_RMW32(CTRL_REG_SD, SD0_EMMC_SEL_MASK,
				   value << SD0_EMMC_SEL_SHIFT);
			XPfw_RMW32(SD_CONFIG_REG2, SD0_SLOTTYPE_MASK,
				   value << SD0_SLOTTYPE_SHIFT);
		} else {
			XPfw_RMW32(CTRL_REG_SD, SD1_EMMC_SEL_MASK,
				   value << SD1_EMMC_SEL_SHIFT);
			XPfw_RMW32(SD_CONFIG_REG2, SD1_SLOTTYPE_MASK,
				   value << SD1_SLOTTYPE_SHIFT);
		}
		status = XST_SUCCESS;
		break;
	case SD_CONFIG_BASECLK:
		if (NODE_SD_0 == nodeId) {
			XPfw_RMW32(SD_CONFIG_REG1, SD0_BASECLK_MASK,
				   value << SD0_BASECLK_SHIFT);
		} else {
			XPfw_RMW32(SD_CONFIG_REG1, SD1_BASECLK_MASK,
				   value << SD1_BASECLK_SHIFT);
		}
		status = XST_SUCCESS;
		break;
	case SD_CONFIG_8BIT:
		if (NODE_SD_0 == nodeId) {
			XPfw_RMW32(SD_CONFIG_REG2, SD0_8BIT_MASK,
				   value << SD0_8BIT_SHIFT);
		} else {
			XPfw_RMW32(SD_CONFIG_REG2, SD1_8BIT_MASK,
				   value << SD1_8BIT_SHIFT);
		}
		status = XST_SUCCESS;
		break;
	case SD_CONFIG_FIXED:
		if (NODE_SD_0 == nodeId) {
			status = PmConfigureSd0Regs();
		} else {
			status = PmConfigureSd1Regs();
		}
		break;
	default:
		status = XST_INVALID_PARAM;
		break;
	}

done:
	return status;
}

/**
 * PmSetGemConfig() - Configure GEM registers.
 * @nodeId	GEM node ID
 * @configType	configuration type
 * @value	value to be written
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error
 *		code or a reason code
 */
s32 PmSetGemConfig(u32 nodeId, XPm_GemConfigType configType, u32 value)
{
	s32 status = XST_FAILURE;

	if ((NODE_ETH_0 != nodeId) && (NODE_ETH_1 != nodeId) &&
	    (NODE_ETH_2 != nodeId) && (NODE_ETH_3 != nodeId)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	switch(configType) {
	case GEM_CONFIG_SGMII_MODE:
		if (NODE_ETH_0 == nodeId) {
			XPfw_RMW32(GEM_CLK_CTRL, GEM0_SGMII_MODE_MASK,
				   value << GEM0_SGMII_MODE_SHIFT);
		} else if (NODE_ETH_1 == nodeId) {
			XPfw_RMW32(GEM_CLK_CTRL, GEM1_SGMII_MODE_MASK,
				   value << GEM1_SGMII_MODE_SHIFT);
		} else if (NODE_ETH_2 == nodeId) {
			XPfw_RMW32(GEM_CLK_CTRL, GEM2_SGMII_MODE_MASK,
				   value << GEM2_SGMII_MODE_SHIFT);
		} else {
			XPfw_RMW32(GEM_CLK_CTRL, GEM3_SGMII_MODE_MASK,
				   value << GEM3_SGMII_MODE_SHIFT);
		}
		status = XST_SUCCESS;
		break;
	case GEM_CONFIG_FIXED:
		if (NODE_ETH_0 == nodeId) {
			SET_FIXED_GEM_CONFIG(0);
		} else if (NODE_ETH_1 == nodeId) {
			SET_FIXED_GEM_CONFIG(1);
		} else if (NODE_ETH_2 == nodeId) {
			SET_FIXED_GEM_CONFIG(2);
		} else {
			SET_FIXED_GEM_CONFIG(3);
		}
		status = XST_SUCCESS;
		break;
	default:
		status = XST_INVALID_PARAM;
		break;
	}

done:
	return status;
}

/**
 * PmSetUsbConfig() - Configure USB registers.
 * @nodeId	USB node ID
 * @configType	configuration type
 * @value	value to be written
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error
 *		code or a reason code
 */
s32 PmSetUsbConfig(u32 nodeId, XPm_UsbConfigType configType, u32 value)
{
	s32 status = XST_FAILURE;

	(void)value;

	if ((NODE_USB_0 != nodeId) && (NODE_USB_1 != nodeId)) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	if (USB_CONFIG_FIXED == configType) {
		if (NODE_USB_0 == nodeId) {
			SET_FIXED_USB_CONFIG(0);
		} else {
			SET_FIXED_USB_CONFIG(1);
		}
		status = XST_SUCCESS;
	} else {
		status = XST_INVALID_PARAM;
	}

done:
	return status;
}
#endif /* ENABLE_DYNAMIC_MIO_CONFIG */
#endif /* ENABLE_IOCTL */
