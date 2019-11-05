/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi_util.h"
#include "xpm_api.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_pldomain.h"
#include "xpm_pll.h"
#include "xpm_powerdomain.h"
#include "xpm_pmcdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_npdomain.h"
#include "xpm_cpmdomain.h"
#include "xpm_psm.h"
#include "xpm_pmc.h"
#include "xpm_periph.h"
#include "xpm_mem.h"
#include "xpm_apucore.h"
#include "xpm_rpucore.h"
#include "xpm_power.h"
#include "xpm_pin.h"
#include "xplmi.h"
#include "xplmi_modules.h"
#include "xpm_aie.h"
#include "xpm_prot.h"
#include "xpm_regs.h"
#include "xpm_ipi.h"
#include "xsysmonpsv.h"
#include "xpm_notifier.h"
#include "xplmi_error_node.h"

#ifdef STDOUT_BASEADDRESS
#if (STDOUT_BASEADDRESS == 0xFF000000U)
#define NODE_UART PM_DEV_UART_0 /* Assign node ID with UART0 device ID */
#elif (STDOUT_BASEADDRESS == 0xFF010000U)
#define NODE_UART PM_DEV_UART_1 /* Assign node ID with UART1 device ID */
#endif
#endif

#define XPm_RegisterWakeUpHandler(GicId, SrcId, NodeId)	\
	XPlmi_GicRegisterHandler(((GicId) << (8U)) | ((SrcId) << (16U)), \
		XPm_DispatchWakeHandler, (void *)(NodeId))

u32 ResetReason;
u32 SysmonAddresses[XPM_NODEIDX_MONITOR_MAX];

void (* PmRequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload);

static XPlmi_ModuleCmd XPlmi_PmCmds[PM_API_MAX+1];
static XPlmi_Module XPlmi_Pm =
{
	XPLMI_MODULE_XILPM_ID,
	XPlmi_PmCmds,
	PM_API_MAX+1,
};

/****************************************************************************/
/**
 * @brief  This function sets the rate of the clock.
 *
 * @param IpiMask	IpiMask of subsystem
 * @param ClockId	Clock node ID
 * @param ClkRate	Clock rate
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static int XPm_SetClockRate(const u32 IpiMask, const u32 ClockId, const u32 ClkRate)
{
	int Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* Set rate is allow only for the request come from CDO,
	 * So by use of IpiMask check that request come from CDO or not,
	 * If request comes from CDO then IpiMask will 0x00U.
	 */
	if (0U != IpiMask) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Set rate is allowed only for ref clocks */
	if (!ISREFCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_SetRate(Clk, ClkRate);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function gets the rate of the clock.
 *
 * @param ClockId	Clock node ID
 * @param ClkRate	Pointer to store clock rate.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static int XPm_GetClockRate(const u32 ClockId, u32 *ClkRate)
{
	int Status = XST_SUCCESS;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (NULL == Clk) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Get rate is allowed only for ref clocks */
	if (!ISREFCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_GetRate(Clk, ClkRate);

done:
	return Status;
}

static int XPm_ProcessCmd(XPlmi_Cmd * Cmd)
{
	u32 ApiResponse[XPLMI_CMD_RESP_SIZE-1] = {0};
	int Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = NULL;
	u32 SubsystemId = INVALID_SUBSYSID;
	u32 *Pload = Cmd->Payload;
	u32 Len = Cmd->Len;
	u32 SetAddress;
	u64 Address;

	PmDbg("Processing Cmd %x\r\n", Cmd->CmdId);

	if((Cmd->CmdId & 0xFFU) != PM_SET_CURRENT_SUBSYSTEM) {
		SubsystemId = XPmSubsystem_GetCurrent();
		if(SubsystemId != INVALID_SUBSYSID) {
			PmDbg("Using current subsystemId: 0x%x\n\r", SubsystemId);
		} else if ((0U == Cmd->IpiMask) && (0U != Cmd->SubsystemId)) {
			SubsystemId = Cmd->SubsystemId;
			PmDbg("Using subsystemId passed by PLM: 0x%x\n\r", SubsystemId);

			/* Use PMC subsystem ID for power domain CDOs. */
			if ((u32)XPM_NODECLASS_POWER == NODECLASS(SubsystemId)) {
				SubsystemId = PM_SUBSYS_PMC;
			}
		} else if (0U != Cmd->IpiMask) {
			SubsystemId = XPmSubsystem_GetSubSysIdByIpiMask(Cmd->IpiMask);
			PmDbg("Using subsystemId mapped to IPI: 0x%x\n\r", SubsystemId);
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Unknown else case\r\n", __LINE__);
		}

		Subsystem = XPmSubsystem_GetById(SubsystemId);
		if ((NULL == Subsystem) || (Subsystem->State == (u8)OFFLINE)) {
			/* Subsystem must not be offline here */
			PmErr("Invalid SubsystemId 0x%x\n\r", SubsystemId);
			Status = XPM_INVALID_SUBSYSID;
			goto done;
		}
		/* Set subsystem to online if suspended or powered off */
		if ((Subsystem->State == (u8)SUSPENDED) ||
		    (Subsystem->State == (u8)POWERED_OFF)) {
			Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	switch (Cmd->CmdId & 0xFFU) {
	case PM_GET_CHIPID:
		Status = XPm_GetChipID(&ApiResponse[0], &ApiResponse[1]);
		break;
	case PM_GET_API_VERSION:
		Status = XPm_GetApiVersion(ApiResponse);
		break;
	case PM_REQUEST_WAKEUP:
		/* setAddress is encoded in the 1st bit of the low-word address */
		SetAddress = Pload[1] & 0x1U;
		/* addresses are word-aligned, ignore bit 0 */
		Address = ((u64) Pload[2]) << 32ULL;
		Address += Pload[1] & (~(u64)0x1U);
		Status = XPm_RequestWakeUp(SubsystemId, Pload[0],
					   SetAddress, Address,
					   Pload[3]);
		break;
	case PM_FORCE_POWERDOWN:
		Status = XPm_ForcePowerdown(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_SYSTEM_SHUTDOWN:
		Status = XPm_SystemShutdown(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_SELF_SUSPEND:
		Status = XPm_SelfSuspend(SubsystemId, Pload[0],
					 Pload[1], (u8)Pload[2],
					 Pload[3], Pload[4]);
		break;
	case PM_REQUEST_SUSPEND:
		Status = XPm_RequestSuspend(SubsystemId, Pload[0], Pload[1], Pload[2], Pload[3]);
		break;
	case PM_ABORT_SUSPEND:
		Status = XPm_AbortSuspend(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_SET_WAKEUP_SOURCE:
		Status = XPm_SetWakeUpSource(SubsystemId, Pload[0], Pload[1], Pload[2]);
		break;
	case PM_CLOCK_SETRATE:
		Status = XPm_SetClockRate(Cmd->IpiMask, Pload[0], Pload[1]);
		break;
	case PM_CLOCK_GETRATE:
		Status = XPm_GetClockRate(Pload[0], ApiResponse);
		break;
	case PM_CLOCK_SETPARENT:
		Status = XPm_SetClockParent(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_CLOCK_GETPARENT:
		Status = XPm_GetClockParent(Pload[0], ApiResponse);
		break;
	case PM_CLOCK_ENABLE:
		Status = XPm_SetClockState(SubsystemId, Pload[0], 1);
		break;
	case PM_CLOCK_DISABLE:
		Status = XPm_SetClockState(SubsystemId, Pload[0], 0);
		break;
	case PM_CLOCK_GETSTATE:
		Status = XPm_GetClockState(Pload[0], ApiResponse);
		break;
	case PM_CLOCK_SETDIVIDER:
		Status = XPm_SetClockDivider(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_CLOCK_GETDIVIDER:
		Status = XPm_GetClockDivider(Pload[0], ApiResponse);
		break;
	case PM_PLL_SET_PARAMETER:
		Status = XPm_SetPllParameter(SubsystemId, Pload[0], Pload[1], Pload[2]);
		break;
	case PM_PLL_GET_PARAMETER:
		Status = XPm_GetPllParameter(Pload[0], Pload[1], ApiResponse);
		break;
	case PM_PLL_SET_MODE:
		Status = XPm_SetPllMode(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_PLL_GET_MODE:
		Status = XPm_GetPllMode(Pload[0], ApiResponse);
		break;
	case PM_REQUEST_NODE:
		Status = XPm_RequestDevice(SubsystemId, Pload[0],
					   Pload[1], Pload[2],
					   Pload[3]);
		break;
	case PM_RELEASE_NODE:
		Status = XPm_ReleaseDevice(SubsystemId, Pload[0]);
		break;
	case PM_SET_REQUIREMENT:
		Status = XPm_SetRequirement(SubsystemId, Pload[0], Pload[1], Pload[2], Pload[3]);
		break;
	case PM_SET_MAX_LATENCY:
		Status = XPm_SetMaxLatency(SubsystemId, Pload[0],
					   Pload[1]);
		break;
	case PM_GET_NODE_STATUS:
		Status = XPm_GetDeviceStatus(SubsystemId, Pload[0], (XPm_DeviceStatus *)ApiResponse);
		break;
	case PM_QUERY_DATA:
		Status = XPm_Query(Pload[0], Pload[1], Pload[2],
				   Pload[3], ApiResponse);
		break;
	case PM_RESET_ASSERT:
		Status = XPm_SetResetState(SubsystemId, Cmd->IpiMask,
					   Pload[0], Pload[1]);
		break;
	case PM_RESET_GET_STATUS:
		Status = XPm_GetResetState(Pload[0], ApiResponse);
		break;
	case PM_ADD_SUBSYSTEM:
		Status = XPm_AddSubsystem(Pload[0]);
		break;
	case PM_DESTROY_SUBSYSTEM:
		Status = XPm_DestroySubsystem(Pload[0]);
		break;
	case PM_PINCTRL_REQUEST:
		Status = XPm_PinCtrlRequest(SubsystemId, Pload[0]);
		break;
	case PM_PINCTRL_RELEASE:
		Status = XPm_PinCtrlRelease(SubsystemId, Pload[0]);
		break;
	case PM_PINCTRL_GET_FUNCTION:
		Status = XPm_GetPinFunction(Pload[0], ApiResponse);
		break;
	case PM_PINCTRL_SET_FUNCTION:
		Status = XPm_SetPinFunction(SubsystemId, Pload[0], Pload[1]);
		break;
	case PM_PINCTRL_CONFIG_PARAM_GET:
		Status = XPm_GetPinParameter(Pload[0], Pload[1], ApiResponse);
		break;
	case PM_PINCTRL_CONFIG_PARAM_SET:
		Status = XPm_SetPinParameter(SubsystemId, Pload[0], Pload[1], Pload[2]);
		break;
	case PM_IOCTL:
		Status = XPm_DevIoctl(SubsystemId, Pload[0], Pload[1],
				      Pload[2], Pload[3], ApiResponse);
		break;
	case PM_INIT_FINALIZE:
		Status = XPm_InitFinalize(SubsystemId);
		break;
	case PM_DESCRIBE_NODES:
		Status = XPm_DescribeNodes(Len);
		break;
	case PM_ADD_NODE:
		Status = XPm_AddNode(&Pload[0], Len);
		break;
	case PM_ADD_NODE_PARENT:
		Status = XPm_AddNodeParent(&Pload[0], Len);
		break;
	case PM_ADD_NODE_NAME:
		Status = XPm_AddNodeName(&Pload[0], Len);
		break;
	case PM_ADD_REQUIREMENT:
		Status = XPm_AddRequirement(Pload[0], Pload[1], Pload[2], &Pload[3], Len-3U);
		break;
	case PM_SET_CURRENT_SUBSYSTEM:
		Status = XPm_SetCurrentSubsystem(Pload[0]);
		break;
	case PM_INIT_NODE:
		Status = XPm_InitNode(Pload[0], Pload[1], &Pload[2], Len-2U);
		break;
	case PM_FEATURE_CHECK:
		Status = XPm_FeatureCheck(Pload[0], ApiResponse);
		break;
	case PM_ISO_CONTROL:
		Status = XPm_IsoControl(Pload[0], Pload[1]);
		break;
	case PM_GET_OP_CHARACTERISTIC:
		Status = XPm_GetOpCharacteristic(Pload[0], Pload[1],
						 ApiResponse);
		break;
	case PM_REGISTER_NOTIFIER:
		Status = XPm_RegisterNotifier(SubsystemId, Pload[0],
					      Pload[1], Pload[2],
					      Pload[3], Cmd->IpiMask);
		break;
	default:
		PmErr("CMD: INVALID PARAM\n\r");
		Status = XST_INVALID_PARAM;
		break;
	}

	/* First word of the response is status */
	Cmd->Response[0] = (u32)Status;
	(void)XPlmi_MemCpy(&Cmd->Response[1], ApiResponse, sizeof(ApiResponse));

	if (Status == XST_SUCCESS) {
		Cmd->ResumeHandler = NULL;
	} else {
		PmErr("Error %d while processing command 0x%x\r\n", Status, Cmd->CmdId);
		PmDbg("Command payload: 0x%x, 0x%x, 0x%x, 0x%x\r\n",
			Pload[0], Pload[1], Pload[2], Pload[3]);
	}
done:
	if(Status != XST_SUCCESS) {
		PmErr("Err Code: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief Register wakeup handlers with XilPlmi
 * @param none
 * @return none
 ****************************************************************************/
static void XPm_RegisterWakeUpHandlers(void)
{
	/**
	 * Register the events for PM
	 */
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC13, XPM_NODEIDX_DEV_GPIO);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC14, XPM_NODEIDX_DEV_I2C_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC15, XPM_NODEIDX_DEV_I2C_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC16, XPM_NODEIDX_DEV_SPI_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC17, XPM_NODEIDX_DEV_SPI_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC18, XPM_NODEIDX_DEV_UART_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC19, XPM_NODEIDX_DEV_UART_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC20, XPM_NODEIDX_DEV_CAN_FD_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC21, XPM_NODEIDX_DEV_CAN_FD_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC22, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC23, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC24, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC25, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP0, (u32)XPLMI_GICP0_SRC26, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC5, XPM_NODEIDX_DEV_TTC_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC6, XPM_NODEIDX_DEV_TTC_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC7, XPM_NODEIDX_DEV_TTC_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC8, XPM_NODEIDX_DEV_TTC_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC9, XPM_NODEIDX_DEV_TTC_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC10, XPM_NODEIDX_DEV_TTC_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC11, XPM_NODEIDX_DEV_TTC_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC12, XPM_NODEIDX_DEV_TTC_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC13, XPM_NODEIDX_DEV_TTC_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC14, XPM_NODEIDX_DEV_TTC_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC15, XPM_NODEIDX_DEV_TTC_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC16, XPM_NODEIDX_DEV_TTC_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC24, XPM_NODEIDX_DEV_GEM_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC25, XPM_NODEIDX_DEV_GEM_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC26, XPM_NODEIDX_DEV_GEM_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC27, XPM_NODEIDX_DEV_GEM_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC28, XPM_NODEIDX_DEV_ADMA_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC29, XPM_NODEIDX_DEV_ADMA_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC30, XPM_NODEIDX_DEV_ADMA_2);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP1, (u32)XPLMI_GICP1_SRC31, XPM_NODEIDX_DEV_ADMA_3);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC0, XPM_NODEIDX_DEV_ADMA_4);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC1, XPM_NODEIDX_DEV_ADMA_5);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC2, XPM_NODEIDX_DEV_ADMA_6);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC3, XPM_NODEIDX_DEV_ADMA_7);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP2, (u32)XPLMI_GICP2_SRC10, XPM_NODEIDX_DEV_USB_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP3, (u32)XPLMI_GICP3_SRC30, XPM_NODEIDX_DEV_SDIO_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP3, (u32)XPLMI_GICP3_SRC31, XPM_NODEIDX_DEV_SDIO_0);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP4, (u32)XPLMI_GICP4_SRC0, XPM_NODEIDX_DEV_SDIO_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP4, (u32)XPLMI_GICP4_SRC1, XPM_NODEIDX_DEV_SDIO_1);
	XPm_RegisterWakeUpHandler((u32)XPLMI_PMC_GIC_IRQ_GICP4, (u32)XPLMI_GICP4_SRC14, XPM_NODEIDX_DEV_RTC);
}

/****************************************************************************/
/**
 * @brief  Initialize XilPM library
 *
 * @param  IpiInst	IPI instance
 * @param  RequestCb 	Pointer to the request calbback handler
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_Init(void (* const RequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload))
{
	XStatus Status = XST_FAILURE;
	unsigned int i;
	u32 Version;
	u32 RegValue;
	u32 PmcIPORMask = (CRP_RESET_REASON_ERR_POR_MASK |
			   CRP_RESET_REASON_SLR_POR_MASK |
			   CRP_RESET_REASON_SW_POR_MASK);
	u32 SysResetMask = (CRP_RESET_REASON_SLR_SYS_MASK |
			    CRP_RESET_REASON_SW_SYS_MASK |
			    CRP_RESET_REASON_ERR_SYS_MASK |
			    CRP_RESET_REASON_DAP_SYS_MASK);
	u32 IsolationIdx[] = {
		(u32)XPM_NODEIDX_ISO_VCCAUX_VCCRAM,
		(u32)XPM_NODEIDX_ISO_VCCRAM_SOC,
		(u32)XPM_NODEIDX_ISO_VCCAUX_SOC,
		(u32)XPM_NODEIDX_ISO_PL_SOC,
		(u32)XPM_NODEIDX_ISO_PMC_SOC,
		(u32)XPM_NODEIDX_ISO_PMC_SOC_NPI,
		(u32)XPM_NODEIDX_ISO_PMC_PL,
		(u32)XPM_NODEIDX_ISO_PMC_LPD,
		(u32)XPM_NODEIDX_ISO_LPD_SOC,
		(u32)XPM_NODEIDX_ISO_LPD_PL,
		(u32)XPM_NODEIDX_ISO_LPD_CPM,
		(u32)XPM_NODEIDX_ISO_FPD_SOC,
		(u32)XPM_NODEIDX_ISO_FPD_PL,
	};

	PmInfo("Initializing LibPM\n\r");

	PmRequestCb = RequestCb;

	/* Register command handlers with eFSBL */
	for (i = 1; i < XPlmi_Pm.CmdCnt; i++) {
		XPlmi_PmCmds[i].Handler = XPm_ProcessCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Pm);

	XPm_PsmModuleInit();

	PmIn32(PMC_TAP_VERSION, Version);
	PlatformVersion = ((Version & PMC_TAP_VERSION_PLATFORM_VERSION_MASK) >>
                        PMC_TAP_VERSION_PLATFORM_VERSION_SHIFT);
	Platform = ((Version & PMC_TAP_VERSION_PLATFORM_MASK) >>
                        PMC_TAP_VERSION_PLATFORM_SHIFT);
	PmIn32(PMC_TAP_SLR_TYPE_OFFSET + PMC_TAP_BASEADDR, RegValue);
	SlrType = (RegValue & PMC_TAP_SLR_TYPE_MASK);

	/* Read and store the reset reason value */
	PmIn32(CRP_RESET_REASON, ResetReason);

	if (0U != (ResetReason & SysResetMask)) {
		/* Clear the system reset bits of reset_reason register */
		PmOut32(CRP_RESET_REASON, (ResetReason & SysResetMask));

		/* Enable domain isolations after system reset */
		for (i = 0; i < ARRAY_SIZE(IsolationIdx); i++) {
			Status = XPmDomainIso_Control(IsolationIdx[i], TRUE_VALUE);
			if (Status != XST_SUCCESS) {
				goto done;
			}
		}

		/* For some boards, vccaux workaround is implemented using gpio to control vccram supply.
		During system reset, when gpio goes low, delay is required for system controller to process
		vccram rail off, before pdi load is started */
		if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
			usleep(300000);
		}
	}

	/*
	 * Clear DomainInitStatusReg in case of internal PMC_POR. Since PGGS0
	 * value is not cleared in case of internal POR.
	 */
	if (0U != (ResetReason & PmcIPORMask)) {
		XPm_Out32(XPM_DOMAIN_INIT_STATUS_REG, 0);
	}

	XPm_RegisterWakeUpHandlers();

	Status = XPmSubsystem_Add(PM_SUBSYS_PMC);

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This is the handler for wake up interrupts
 *
 * @param  DeviceIdx	Index of peripheral device
 *
 * @return Status	XST_SUCCESS if processor wake successfully
 *			XST_FAILURE or error code in case of failure
 *
 *****************************************************************************/
int XPm_DispatchWakeHandler(void *DeviceIdx)
{
	int Status;

	Status = XPm_GicProxyWakeUp((u32)DeviceIdx);
	return Status;
}

static void AddPld0Device(void)
{
	XPm_Device *Device = XPmDevice_GetById(PM_DEV_PLD_0);
	int Status;

	if (NULL == Device) {
		u32 Args[3] = {PM_DEV_PLD_0, PM_POWER_PLD};
		u32 Parents[] = {PM_CLK_PMC_PL0_REF, PM_CLK_PMC_PL1_REF,
				 PM_CLK_PMC_PL2_REF, PM_CLK_PMC_PL3_REF,
				 PM_RST_PL0, PM_RST_PL1, PM_RST_PL2,
				 PM_RST_PL3};

		Status = XPm_AddNode(Args, (u32)ARRAY_SIZE(Args));
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in adding PLD0 device\r\n", Status);
		}

		Status = XPmDevice_AddParent(PM_DEV_PLD_0, Parents,
					     (u32)ARRAY_SIZE(Parents));
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in add patent for PLD0 device\r\n",
				Status);
		}
	}
}

static void AddIPIPmcDevice(void)
{
	XPm_Device *Device = XPmDevice_GetById(PM_DEV_IPI_PMC);
	int Status;

	if (NULL == Device) {
		u32 Args[3] = {PM_DEV_IPI_PMC, PM_POWER_LPD};
		u32 Parents[] = {PM_RST_IPI};

		Status = XPm_AddNode(Args, (u32)ARRAY_SIZE(Args));
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in adding PLD0 device\r\n", Status);
		}

		Status = XPmDevice_AddParent(PM_DEV_IPI_PMC, Parents,
					     (u32)ARRAY_SIZE(Parents));
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in add patent for PLD0 device\r\n",
				Status);
		}
	}
}

static void PostTopologyHook(void)
{

	/**
	 * Currently PLD0 device is not added through CDO. This will create
	 * issues when dynamic subsystems are supported. So add workaround for
	 * adding PLD0 device from XilPM until CDO changes to add PLD0 device
	 * from topology are there.
	 * TODO: Remove this workaround when CDO changes to add PLD0 device
	 *	 will available in tools.
	 */
	AddPld0Device();

	/**
	 * TODO: Remove this workaround when CDO changes to add PMC IPI device
	 *	 will available in tools.
	 */
	AddIPIPmcDevice();
}

XStatus XPm_HookAfterPlmCdo(void)
{
	XStatus Status = XST_FAILURE;

	/*
	 * There is a silicon problem where on 2-4% of Versal ES1 S80 devices
	 * you can get 12A of VCCINT_PL current before CFI housecleaning is run.
	 * The problem is eliminated when PL Vgg frame housecleaning is run
	 * so we need to do that ASAP after PLM is loaded.
	 * Otherwise also, PL housecleaning needs to be trigerred asap to reduce
	 * boot time.
	 */
	XPmPlDomain_InitandHouseclean();

	// On Boot, Update PMC SAT0 & SAT1 sysmon trim
	(void)XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PMC_0], PM_POWER_PMC, 0);
	(void)XPmPowerDomain_ApplyAmsTrim(SysmonAddresses[XPM_NODEIDX_MONITOR_SYSMON_PMC_1], PM_POWER_PMC, 1);
	PostTopologyHook();

	/**
	 * VCK190/VMK180 boards have VCC_AUX workaround where MIO-37 (PMC GPIO)
	 * is used to enable the VCC_RAM which used to power the PL.
	 * As a result, if PMC GPIO is disabled, VCC_RAM goes off.
	 * To prevent this from happening, request PMC GPIO device on behalf PMC subsystem.
	 * This will take care of the use cases where PMC is up.
	 * GPIO will get reset only when PMC goes down.
	 *
	 * The VCC_AUX workaround will be removed from MIO-37 in ES2 going ahead.
	 */
	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		Status = XPmDevice_Request(PM_SUBSYS_PMC, PM_DEV_GPIO_PMC,
					   XPM_MAX_CAPABILITY, XPM_MAX_QOS);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the version and ID code of a chip
 *
 * @param  IDCode  Returns the chip ID code.
 * @param  Version Returns the chip version.
 *
 * @return XST_SUCCESS
 *
 * @note   None
 *
 ****************************************************************************/
int XPm_GetChipID(u32* IDCode, u32 *Version)
{
	/* Read the chip ID code */
	PmIn32(PMC_TAP_IDCODE, *IDCode);

	/* Read the chip version */
	PmIn32(PMC_TAP_VERSION, *Version);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  This function is used to request the version number of the API
 * running on the power management controller.
 *
 * @param  Version Returns the API 32-bit version number.
 * Returns 0 if no firmware present.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetApiVersion(u32 *Version)
{
	*Version = PM_VERSION;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  This function configures the
 * platform resources for the new subsystem.
 *
 * @param  SubSystemCdo	Pointer to the subsystem CDO
 * @param  NotifyCb		Pointer to the notify callback handler
 * @param  SubsystemId	Address to store the new subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The provided address must be in an address space which is
 * accessible by the callee.  There will be no change if the subsystem CDO
 * is incompatible or if the required resources are not available, so no
 * clean-up will be necessary
 *
 ****************************************************************************/
XStatus XPm_AddSubsystem(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	Status = XPmSubsystem_Add(SubsystemId);

	if (Status != XST_SUCCESS) {
		PmErr("Failed to configure platform resources\n\r");
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows to set current subsystem id.
 *
 * @param  SubsystemId	Address to store the new subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_SetCurrentSubsystem(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	Status = XPmSubsystem_SetCurrent(SubsystemId);

	if (Status != XST_SUCCESS) {
		PmErr("Unable to set current subsystem. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows to initialize the node.
 *
 * @param  NodeId	Supported power domain nodes only
 * @param  Function	Function id
 * @param  Args		Arguments speicifc to function
 * @param  NumArgs  Number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_InitNode(u32 NodeId, u32 Function, u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PwrDomainNode;

	if (((u32)XPM_NODECLASS_POWER != NODECLASS(NodeId)) ||
	    ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(NodeId)) ||
	    ((u32)XPM_NODEIDX_POWER_MAX <= NODEINDEX(NodeId))) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	PwrDomainNode = (XPm_PowerDomain *)XPmPower_GetById(NodeId);
	if (NULL == PwrDomainNode) {
		PmErr("Unable to find Power Domain for given Node Id\n\r");
		Status = XPM_PM_INVALID_NODE;
                goto done;
	}

	switch (NODEINDEX(NodeId)) {
	case (u32)XPM_NODEIDX_POWER_PMC:
	case (u32)XPM_NODEIDX_POWER_LPD:
	case (u32)XPM_NODEIDX_POWER_FPD:
	case (u32)XPM_NODEIDX_POWER_NOC:
	case (u32)XPM_NODEIDX_POWER_PLD:
	case (u32)XPM_NODEIDX_POWER_ME:
	case (u32)XPM_NODEIDX_POWER_CPM:
	case (u32)XPM_NODEIDX_POWER_CPM5:
		Status = XPmPowerDomain_InitDomain(PwrDomainNode, Function,
						   Args, NumArgs);
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		PmErr("Unrecognized Power Domain: 0x%x\n\r", NODEINDEX(NodeId));
		break;
	}

	/*
	 * Call LPD init to initialize required components
	 */
	if ((NODEINDEX(NodeId) == (u32)XPM_NODEIDX_POWER_LPD) &&
		(Function == (u32)FUNC_INIT_FINISH) &&
		(XST_SUCCESS == Status)) {
#ifdef DEBUG_UART_PS
		/**
		 * PLM needs to request UART if debug is enabled, else LibPM
		 * will turn it off when it is not used by other processor.
		 * During such scenario when PLM tries to print debug message,
		 * system may not work properly.
		 */
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, NODE_UART,
					   (u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
#endif
		/**
		 * PLM needs to request PMC IPI, else LibPM will reset IPI
		 * when it is not used by other processor. Because of that PLM
		 * hangs when it tires to communicate through IPI.
		 */
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_IPI_PMC,
				(u32)PM_CAP_ACCESS, XPM_MAX_QOS, 0);
		if (XST_SUCCESS != Status) {
			PmErr("Error %d in request IPI PMC\r\n", Status);
		}
		XPlmi_LpdInit();
	}

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to initialize node. Returned: 0x%x\n\r", Status);
	}
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function allows to control isolation nodes.
 *
 * @param  Isoaltion NodeId	Supported isoaltion nodes only
 * @param  Enable/Disable
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   none
 *
 ****************************************************************************/
XStatus XPm_IsoControl(u32 NodeId, u32 Enable)
{
	XStatus Status = XST_FAILURE;

	if (((u32)XPM_NODECLASS_ISOLATION != NODECLASS(NodeId)) ||
	    ((u32)XPM_NODEIDX_ISO_MAX <= NODEINDEX(NodeId))) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XPmDomainIso_Control(NODEINDEX(NodeId), Enable);

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to initialize node. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function releases all the resources of a subsystem.  The
 * subsystem ID will become invalid.
 *
 * @param  SubsystemId	Subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_DestroySubsystem(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	Status = XPmSubsystem_Destroy(SubsystemId);

	if (Status != XST_SUCCESS) {
		PmErr("Unable release node. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to aborting suspend of a
 * child subsystem.
 *
 * @param SubsystemId	Subsystem ID
 * @param Reason	Abort reason
 * @param DeviceId	Processor device ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note        None
 *
 ****************************************************************************/
XStatus XPm_AbortSuspend(const u32 SubsystemId, const u32 Reason,
			 const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;

	PmInfo("(%lu, %lu)\r\n", Reason, DeviceId);

	if((NODECLASS(DeviceId) == (u32)XPM_NODECLASS_DEVICE) &&
	   (NODESUBCLASS(DeviceId) == (u32)XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
		if (NULL == Core) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		Core->Device.Node.State = (u8)XPM_DEVSTATE_RUNNING;
	} else {
		PmErr("Invalid Device Id\n\r");
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	DISABLE_WFI(Core->SleepMask);

	Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);

done:
	if (Status != XST_SUCCESS) {
		PmErr("Unable to abort suspend child subsystem. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to suspend a child
 * subsystem.
 *
 * @param SubsystemId	Subsystem ID
 * @param DeviceId	Processor device ID
 * @param Latency	Maximum wake-up latency requirement in us(microsecs)
 * @param State		Instead of specifying a maximum latency, a CPU can also
 *			explicitly request a certain power state.
 * @param AddressLow	Lower Address from which to resume when wake up.
 * @param AddressHigh	Higher Address from which to resume when wake up.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPm_SelfSuspend(const u32 SubsystemId, const u32 DeviceId,
			const u32 Latency, const u8 State,
			u32 AddrLow, u32 AddrHigh)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	u64 Address = (u64)AddrLow + ((u64)AddrHigh << 32ULL);
	XPm_Requirement *Reqm;

	/* TODO: Remove this warning fix hack when functionality is implemented */
	(void)Latency;

	if ((NODECLASS(DeviceId) == (u32)XPM_NODECLASS_DEVICE) &&
	   (NODESUBCLASS(DeviceId) == (u32)XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(DeviceId);
		if (NULL == Core) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
		Core->ResumeAddr = Address | 1U;
		Core->Device.Node.State = (u8)XPM_DEVSTATE_SUSPENDING;
	} else {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	ENABLE_WFI(Core->SleepMask);

	if (PM_SUSPEND_STATE_SUSPEND_TO_RAM == State) {
		Status = XPmSubsystem_SetState(SubsystemId, (u32)SUSPENDING);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* If subsystem is using DDR, enable self-refresh as post suspend requirement*/
	if (PM_SUSPEND_STATE_SUSPEND_TO_RAM == State) {
		Reqm = XPmDevice_FindRequirement(PM_DEV_DDR_0, SubsystemId);
		if (XST_SUCCESS == XPmRequirement_IsExclusive(Reqm)) {
			Status = XPmDevice_SetRequirement(SubsystemId, PM_DEV_DDR_0,
							  (u32)PM_CAP_CONTEXT, 0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("Unable to Self Suspend child subsystem. Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to send suspend request
 * to another subsystem.  If the target subsystem accepts the request, it
 * needs to initiate its own self suspend.
 *
 * @param SubsystemId		Subsystem ID
 * @param TargetSubsystemId	Target subsystem ID (cannot be the same subsystem)
 * @param Ack			Ack request
 * @param Latency		Desired wakeup latency
 * @param State			Desired power state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been sent.
 *
 ****************************************************************************/
XStatus XPm_RequestSuspend(const u32 SubsystemId, const u32 TargetSubsystemId,
			   const u32 Ack, const u32 Latency, const u32 State)
{
	XStatus Status = XST_FAILURE;
	u32 IpiMask = 0;
	u32 Payload[5] = {0};
	/* Warning Fix */
	(void) (Ack);

	IpiMask = XPmSubsystem_GetIPIMask(TargetSubsystemId);
	if (0U == IpiMask) {
		PmErr("Unable to fetch IpiMask for given TargetSubsystem\r\n");
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	if (SubsystemId == TargetSubsystemId) {
		Status = XPM_INVALID_SUBSYSID;
		PmErr("Cannot Suspend yourself\n\r");
		goto done;
	}

	/* TODO: Check if current subsystem has access to request target subsystem */
	/* Failure in this case should return XPM_PM_NO_ACCESS */

	/* TODO: Target subsystem must be active to get the suspend request */

	/* TODO: Check if other subsystem has sent suspend request to target subsystem */
	/* Failure in this case should return XPM_PM_DOUBLE_REQ */

	Payload[0] = (u32)PM_INIT_SUSPEND_CB;
	Payload[1] = (u32)SUSPEND_REASON_PU_REQ;
	Payload[2] = Latency;
	Payload[3] = State;
	/* Payload[4] is for timeout which is not considered */
	Payload[4] = 0U;

	/* Send the suspend request via callback */
	if (NULL != PmRequestCb) {
		(*PmRequestCb)(IpiMask, PM_INIT_SUSPEND_CB, Payload);
	}

	Status = XST_SUCCESS;

done:
	if (Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

int XPm_GicProxyWakeUp(const u32 PeriphIdx)
{
	int Status = XST_FAILURE;

	XPm_Periph *Periph = (XPm_Periph *)XPmDevice_GetByIndex(PeriphIdx);
	if ((NULL == Periph) || (0U == Periph->WakeProcId)) {
		goto done;
	}

	Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, Periph->WakeProcId, 0, 0, 0);

done:
	return Status;
}


int XPm_SubsystemPwrUp(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;

	/* Add a workaround to power on FPD before
	 * subsystem wakeup if FPD is off.
	 * TODO: This workaround will be reverted,
	 * once recursive CDO loading support is available.
	 */

	XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);
	if ((NULL != FpdPwrNode) &&
	    ((u8)XPM_POWER_STATE_OFF == FpdPwrNode->Node.State)) {
		Status = XPm_PowerUpFPD(&FpdPwrNode->Node);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}
	Status = XLoader_RestartImage(SubsystemId);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to start-up and wake-up
 * a child subsystem.  If the target subsystem has been loaded and is ready
 * to run, it will start running.  If the target subsystem is suspended, it
 * will resume.
 *
 * @param SubsystemId		Subsystem ID
 * @param  DeviceId	Processor device ID
 * @param  SetAddress 	Specifies whether to set the start address.
 * - 0 : do not set start address
 * - 1 : set start address
 * @param  Address	Address from which to resume when woken up.
 * @param  Ack		Ack request
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_RequestWakeUp(u32 SubsystemId, const u32 DeviceId,
			  const u32 SetAddress, const u64 Address,
			  const u32 Ack)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	u32 CoreSubsystemId, CoreDeviceId;
	XPm_Requirement *Reqm;
	XPm_Power *Power;

	/* Warning Fix */
	(void) (Ack);

	/*Validate access first */
	Status = XPm_IsWakeAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	switch (NODECLASS(DeviceId))
	{
		case (u32)XPM_NODECLASS_SUBSYSTEM:
			CoreSubsystemId = DeviceId;
			Status = XPm_SubsystemPwrUp(CoreSubsystemId);
			break;
		case (u32)XPM_NODECLASS_DEVICE:
			CoreDeviceId = DeviceId;
			Core = (XPm_Core *)XPmDevice_GetById(CoreDeviceId);
			if ((NULL == Core) ||
			    (NULL == Core->CoreOps->RequestWakeup)) {
				Status = XPM_ERR_WAKEUP;
				break;
			}
			if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(CoreDeviceId)) ||
			    ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(CoreDeviceId)) ||
			    ((u32)XPM_NODETYPE_DEV_CORE_PSM == NODETYPE(CoreDeviceId))) {
				/* Power up LPD if not powered up */
				Power = XPmPower_GetById(PM_POWER_LPD);
				if ((NULL != Power) && ((u8)XPM_POWER_STATE_OFF == Power->Node.State)) {
					Status = XLoader_RestartImage(Power->Node.Id);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}
			CoreSubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);
			if (INVALID_SUBSYSID == CoreSubsystemId) {
				Status = XPM_ERR_SUBSYS_NOTFOUND;
				break;
			}
			Status = Core->CoreOps->RequestWakeup(Core, SetAddress, Address);
			if (XST_SUCCESS == Status) {
				Status = XPmSubsystem_SetState(CoreSubsystemId, (u32)ONLINE);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* If subsystem is using DDR, disable self-refresh */
	Reqm = XPmDevice_FindRequirement(PM_DEV_DDR_0, CoreSubsystemId);
	if (XST_SUCCESS == XPmRequirement_IsExclusive(Reqm)) {
		Status = XPmDevice_SetRequirement(CoreSubsystemId, PM_DEV_DDR_0,
						  (u32)PM_CAP_ACCESS, 0);
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to Powerdown other
 * 		processor or domain node forcefully. To powerdown whole
 * 		subsystem, this function needs to be called for all processors
 * 		of target subsystem, which in turn will power down the whole
 * 		subsystem
 *
 * @param SubsystemId	Subsystem ID
 * @param  Node 		Processor or domain node to be powered down
 * @param  Ack			Ack request
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   The affected PUs are not notified about the upcoming powerdown,
 *          and PMU does not wait for their WFI interrupt.
 *
 ****************************************************************************/
XStatus XPm_ForcePowerdown(u32 SubsystemId, const u32 NodeId, const u32 Ack)
{
	XStatus Status = XST_FAILURE;
	XPm_Core *Core;
	XPm_Device *Device;
	XPm_Power *Power;
	XPm_Requirement *Reqm;
	u32 i;
	XPm_Power *Acpu0PwrNode = XPmPower_GetById(PM_POWER_ACPU_0);
	XPm_Power *Acpu1PwrNode = XPmPower_GetById(PM_POWER_ACPU_1);
	XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);
	XPm_Subsystem *TargetSubsystem = NULL;
	u32 DeviceId = 0U;

	/* Warning Fix */
	(void) (Ack);

	/*Validate access first */
	Status = XPm_IsForcePowerDownAllowed(SubsystemId, NodeId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if ((NODECLASS(NodeId) == (u32)XPM_NODECLASS_DEVICE) &&
	    (NODESUBCLASS(NodeId) == (u32)XPM_NODESUBCL_DEV_CORE)) {
		Core = (XPm_Core *)XPmDevice_GetById(NodeId);
		if ((NULL != Core) && (NULL != Core->CoreOps)
		    && (NULL != Core->CoreOps->PowerDown)) {
			Status = Core->CoreOps->PowerDown(Core);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			/* Disable the direct wake in case of force power down */
			DISABLE_WAKE(Core->SleepMask);
		} else {
			Status = XST_FAILURE;
			goto done;
		}

		/* Do APU GIC pulse reset if All the cores are in Power OFF
		 * state and FPD in Power ON state.
		 * Now APU has two core as ACPU0 and ACPU1.
		 */
		if (((PM_DEV_ACPU_0 == NodeId) || (PM_DEV_ACPU_1 == NodeId)) &&
		    (NULL != Acpu0PwrNode) && (NULL != Acpu1PwrNode) &&
		    (NULL != FpdPwrNode)) {
			if (((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State) &&
			    ((u8)XPM_POWER_STATE_OFF == Acpu0PwrNode->Node.State) &&
			    ((u8)XPM_POWER_STATE_OFF == Acpu1PwrNode->Node.State)) {
				Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_PULSE);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}

	} else if ((u32)XPM_NODECLASS_POWER == NODECLASS(NodeId)) {
		if ((u32)XPM_NODESUBCL_POWER_ISLAND == NODESUBCLASS(NodeId)) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		if ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(NodeId)) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		/*
		 * PMC power domain can not be powered off.
		 */
		if ((u32)XPM_NODEIDX_POWER_PMC == NODEINDEX(NodeId)) {
			Status = XPM_PM_INVALID_NODE;
			goto done;
		}

		/*
		 * Release devices belonging to the power domain.
		 */
		for (i = 1; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
			/*
			 * Note: XPmDevice_GetByIndex() assumes that the caller
			 * is responsible for validating the Node ID attributes
			 * other than node index.
			 */
			Device = XPmDevice_GetByIndex(i);
			if ((NULL == Device) ||
			    ((u32)XPM_DEVSTATE_UNUSED == Device->Node.State)) {
				continue;
			}

			/*
			 * Check power topology of this device to identify
			 * if it belongs to the power domain.
			 */
			Power = Device->Power;
			while (NULL != Power) {
				if (NodeId == Power->Node.Id) {
					if ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Device->Node.Id)) {
						Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
						if ((NULL != Core) &&
						    (NULL != Core->CoreOps) &&
						    (NULL != Core->CoreOps->PowerDown)) {
							PmDbg("Powering down core 0x%x\r\n", Device->Node.Id);
							Status = Core->CoreOps->PowerDown(Core);
							if (XST_SUCCESS != Status) {
								goto done;
							}
							/*
							 * Disable the direct
							 * wake in case of force
							 * power down
							 */
							DISABLE_WAKE(Core->SleepMask);
						} else {
							Status = XST_FAILURE;
							goto done;
						}
					}

					Status = XPmRequirement_Release(
					    Device->Requirements, RELEASE_DEVICE);
					if (XST_SUCCESS != Status) {
						Status = XPM_PM_INVALID_NODE;
						goto done;
					}
				}
				Power = Power->Parent;
			}
		}
	} else if ((u32)XPM_NODECLASS_SUBSYSTEM == NODECLASS(NodeId)) {
		TargetSubsystem = XPmSubsystem_GetById(NodeId);
		if (NULL == TargetSubsystem) {
			Status = XPM_INVALID_SUBSYSID;
			goto done;
		}
		Reqm = TargetSubsystem->Requirements;
		while (NULL != Reqm) {
			if ((1U == Reqm->Allocated) &&
			    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Reqm->Device->Node.Id))) {
				DeviceId = Reqm->Device->Node.Id;
				Status = XPm_ForcePowerdown(SubsystemId, DeviceId, 0U);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
			Reqm = Reqm->NextDevice;
		}
		/* Idle the subsystem */
		Status = XPmSubsystem_Idle(TargetSubsystem->Id);
		if(XST_SUCCESS != Status) {
			Status = XPM_ERR_SUBSYS_IDLE;
			goto done;
		}

		Status = XPmSubsystem_ForceDownCleanup(TargetSubsystem->Id);
		if(XST_SUCCESS != Status) {
			Status = XPM_ERR_CLEANUP;
			goto done;
		}

		Status = XPmSubsystem_SetState(TargetSubsystem->Id, (u32)POWERED_OFF);
		if (XST_SUCCESS != Status) {
			goto done;
		}

	} else {
		Status = XPM_PM_INVALID_NODE;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to shutdown self or restart
 * 			self, Ps or system
 *
 * @param SubsystemId		Subsystem ID
 * @param  Type				Shutdown type
 * @param SubType			Shutdown subtype
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_SystemShutdown(u32 SubsystemId, const u32 Type, const u32 SubType)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;

	if ((PM_SHUTDOWN_TYPE_SHUTDOWN != Type) &&
	    (PM_SHUTDOWN_TYPE_RESET != Type)) {
		Status = XPM_INVALID_TYPEID;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/* For shutdown type the subtype is irrelevant: shut the caller down */
	if (PM_SHUTDOWN_TYPE_SHUTDOWN == Type) {
		/* Idle the subsystem first */
		Status = XPmSubsystem_Idle(SubsystemId);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_SUBSYS_IDLE;
			goto done;
		}
		/* Release devices and power down cores */
		Status = XPmSubsystem_ForceDownCleanup(SubsystemId);
		if (XST_SUCCESS != Status) {
			Status = XPM_ERR_CLEANUP;
			goto done;
		}

		Status = XPmSubsystem_SetState(SubsystemId, (u32)POWERED_OFF);
		goto done;
	}

	switch (SubType) {
	case PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM:

		Status = XPmSubsystem_Restart(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SubsystemPwrUp(SubsystemId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		break;
	case PM_SHUTDOWN_SUBTYPE_RST_PS_ONLY:
		/* TODO */
		break;
	case PM_SHUTDOWN_SUBTYPE_RST_SYSTEM:
		Status = XPmReset_SystemReset();
		break;
	default:
		Status = XPM_INVALID_TYPEID;
		break;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function can be used by a subsystem to to set wake up
 * source
 *
 * @param SubsystemId Initiator of the request
 * @param TargetNodeId  Core to be woken-up (currently must be same as initiator)
 * @param SourceNodeId  Source of the wake-up (Device that generates interrupt)
 * @param Enable      Flag stating should event be enabled or disabled
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   This function does not block.  A successful return code means that
 * the request has been received.
 *
 ****************************************************************************/
XStatus XPm_SetWakeUpSource(const u32 SubsystemId, const u32 TargetNodeId,
			    const u32 SourceNodeId, const u32 Enable)
{
	int Status = XST_FAILURE;
	XPm_Periph *Periph = NULL;
	XPm_Subsystem *Subsystem;

	/* Check if given target node is valid */
	if(NODECLASS(TargetNodeId) != (u32)XPM_NODECLASS_DEVICE ||
	   NODESUBCLASS(TargetNodeId) != (u32)XPM_NODESUBCL_DEV_CORE)
	{
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* The call applies only to peripheral nodes */
	if (NODECLASS(SourceNodeId) != (u32)XPM_NODECLASS_DEVICE ||
	    NODESUBCLASS(SourceNodeId) != (u32)XPM_NODESUBCL_DEV_PERIPH) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Is subsystem allowed to use resource (slave)? */
	Status = XPm_IsAccessAllowed(SubsystemId, SourceNodeId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Periph = (XPm_Periph *)XPmDevice_GetById(SourceNodeId);
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if((NULL == Periph) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check whether the device has wake-up capability */
	Status = XPm_CheckCapabilities(&Periph->Device, (u32)PM_CAP_WAKEUP);
	if (XST_SUCCESS != Status) {
		Status = XST_NO_FEATURE;
		goto done;
	}

	Periph->WakeProcId = TargetNodeId;

	if (NULL != Periph->PeriphOps->SetWakeupSource) {
		Periph->PeriphOps->SetWakeupSource(Periph, Enable);
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Request the usage of a device. A subsystem requests access to a
 * device and asserts its requirements on that device. The platform
 * management controller will enable access to the memory mapped region
 * containing the control registers of that device. For devices that can only
 * be used by one subsystem, any other subsystems will now be blocked from
 * accessing this device until it is released.
 *
 * @param SubsystemId	Target subsystem ID (can be the same subsystem)
 * @param Device		ID of the device
 * @param Capabilities		Capability requirements (1-hot)
 * @param QoS			Quality of Service (0-100) required
 * @param Ack			Ack request
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_RequestDevice(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 Ack)
{
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Ack);

	Status = XPmDevice_Request(SubsystemId, DeviceId, Capabilities,
				   QoS);

	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a subsystem to release the usage of a
 * device. This will tell the platform management controller that the device
 * is no longer needed, allowing the device to be placed into an inactive
 * state.
 *
 * @param SubsystemId	Subsystem ID
 * @param  DeviceId	ID of the device.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_ReleaseDevice(const u32 SubsystemId, const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = NULL;
	XPm_Device* Device = NULL;
	u32 Usage = 0U;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_Release(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Usage = XPmDevice_GetUsageStatus(Subsystem, Device);
	if (0U == Usage) {
		XPmNotifier_Event(Device->Node.Id, (u32)EVENT_ZERO_USERS);
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used by a subsystem to announce a change in
 * requirements for a specific device which is currently in use.
 *
 * @param SubsystemId	Subsystem ID.
 * @param DeviceId	ID of the device.
 * @param Capabilities	Capabilities required
 * @param QoS		Quality of Service (0-100) required.
 * @param Ack		Ack request
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If this function is called after the last awake CPU within the
 * subsystem calls RequestSuspend, the requirement change shall be performed
 * after the CPU signals the end of suspend to the platform management
 * controller, (e.g. WFI interrupt).
 *
 ****************************************************************************/
XStatus XPm_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
			   const u32 Capabilities, const u32 QoS, const u32 Ack)
{
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Ack);

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_SetRequirement(SubsystemId, DeviceId,
					  Capabilities, QoS);
done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Set maximum allowed latency for the device
 *
 * @param  SubsystemId	Initiator of the request who must previously requested
 *			the device
 * @param  DeviceId	Device whose latency is specified
 * @param  Latency	Maximum allowed latency in micro sec
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code or
 * a reason code
 *
 ****************************************************************************/
int XPm_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
		      const u32 Latency)
{
	int Status = XPM_ERR_SET_LATENCY;

	PmInfo("(%x, %lu)\r\n", DeviceId, Latency);

	Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
	if (XST_SUCCESS != Status) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmDevice_SetMaxLatency(SubsystemId, DeviceId, Latency);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to obtain information about the current state
 * of a device. The caller must pass a pointer to an XPm_DeviceStatus
 * structure, which must be pre-allocated by the caller.
 *
 * @param  SubsystemId	Subsystem ID.
 * @param  DeviceId	Device ID
 * @param  DeviceStatus	Pointer to the device status
 *
 * - Status - The current power state of the device
 *  - For CPU nodes:
 *   - 0 : if CPU is powered down,
 *   - 1 : if CPU is active (powered up),
 *   - 2 : if CPU is suspending (powered up)
 *  - For power islands and power domains:
 *   - 0 : if island is powered down,
 *   - 1 : if island is powered up
 *  - For slaves:
 *   - 0 : if slave is powered down,
 *   - 1 : if slave is powered up,
 *   - 2 : if slave is in retention
 *
 * - Requirement - Requirements placed on the device by the caller
 *
 * - Usage
 *  - 0 : node is not used by any PU,
 *  - 1 : node is used by caller exclusively,
 *  - 2 : node is used by other PU(s) only,
 *  - 3 : node is used by caller and by other PU(s)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetDeviceStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;


	switch(NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_GetStatus(SubsystemId, DeviceId, DeviceStatus);
		break;
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_GetStatus(SubsystemId, DeviceId, DeviceStatus);
		break;
	case (u32)XPM_NODECLASS_SUBSYSTEM:
		Status = XPmSubsystem_GetStatus(SubsystemId, DeviceId,
						DeviceStatus);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function queries information about the platform resources.
 *
 * @param Qid		The type of data to query
 * @param Arg1		Query argument 1
 * @param Arg2		Query argument 2
 * @param Arg3		Query argument 3
 * @param Output	Pointer to the output data
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPm_Query(const u32 Qid, const u32 Arg1, const u32 Arg2,
		  const u32 Arg3, u32 *const Output)
{
	XStatus Status = XST_FAILURE;

	/* Warning Fix */
	(void) (Arg3);

	switch (Qid) {
	case (u32)XPM_QID_CLOCK_GET_NAME:
		Status = XPmClock_QueryName(Arg1,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_TOPOLOGY:
		Status = XPmClock_QueryTopology(Arg1,Arg2,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS:
		Status = XPmClock_QueryFFParams(Arg1,Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_MUXSOURCES:
		if (ISPLL(Arg1)) {
			Status = XPmClockPll_QueryMuxSources(Arg1,Arg2,Output);
		} else {
			Status = XPmClock_QueryMuxSources(Arg1,Arg2,Output);
		}
		break;
	case (u32)XPM_QID_CLOCK_GET_ATTRIBUTES:
		Status = XPmClock_QueryAttributes(Arg1,Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_NUM_PINS:
		Status = XPmPin_GetNumPins(Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTIONS:
		Status = XPmPinFunc_GetNumFuncs(Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS:
		Status = XPmPinFunc_GetNumFuncGroups(Arg1, Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_NAME:
		Status = XPmPinFunc_GetFuncName(Arg1, (char *)Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_FUNCTION_GROUPS:
		Status = XPmPinFunc_GetFuncGroups(Arg1, Arg2, (u16 *)Output);
		break;
	case (u32)XPM_QID_PINCTRL_GET_PIN_GROUPS:
		Status = XPmPin_GetPinGroups(Arg1, Arg2, (u16 *)Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_NUM_CLOCKS:
		Status = XPmClock_GetNumClocks(Output);
		break;
	case (u32)XPM_QID_CLOCK_GET_MAX_DIVISOR:
		Status = XPmClock_GetMaxDivisor(Arg1, Arg2, Output);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function enables or disables the clock.
 *
 * @param SubsystemId	Subsystem ID
 * @param ClockId	ID of the clock node
 * @param Enable	Enable (1) or disable (0)
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To enable a clock, the subsystem must be using the clock.  To
 * disable a clock, the subsystem must be the only user of the clock, and the
 * clock must not have any downstream clock(s) that are enabled.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
XStatus XPm_SetClockState(const u32 SubsystemId, const u32 ClockId, const u32 Enable)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);
	u32 CurrState = 0U;

	/* HACK: Don't disable PLL clocks for now */
	if(Enable == 0U && ISPLL(ClockId))
	{
		Status = XST_SUCCESS;
		return Status;
	}

	/* Check if clock's state is already desired state */
	Status = XPm_GetClockState(ClockId, &CurrState);
	if ((XST_SUCCESS == Status) && (CurrState == Enable)) {
		goto done;
	}

	/* HACK: Allow enabling of PLLs for now */
	if ((1U == Enable) && (ISPLL(ClockId))) {
		goto bypass;
	}

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		goto done;
	}

bypass:
	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_SetGate((XPm_OutClockNode *)Clk, Enable);
	} else if (ISPLL(ClockId)) {
		u32 Mode;
		if (1U == Enable) {
			Mode = ((XPm_PllClockNode *)Clk)->PllMode;
		} else if (0U == Enable) {
			Mode = (u32)PM_PLL_MODE_RESET;
		} else {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Status = XPmClockPll_SetMode((XPm_PllClockNode *)Clk, Mode);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the clock state.
 *
 * @param ClockId	ID of the clock node
 * @param State		Pointer to the clock state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetClockState(const u32 ClockId, u32 *const State)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk;

	Clk = XPmClock_GetById(ClockId);
	if (NULL == Clk) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						(u32)TYPE_GATE, State);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetMode((XPm_PllClockNode *)Clk, State);
		if (*State == (u32)PM_PLL_MODE_RESET) {
			*State = 0;
		} else {
			*State = 1;
		}
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the divider value of the clock.
 *
 * @param SubsystemId	Subsystem ID.
 * @param ClockId	Clock node ID
 * @param Divider	Divider value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To change the clock divider, the clock must be disabled.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
XStatus XPm_SetClockDivider(const u32 SubsystemId, const u32 ClockId, const u32 Divider)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (0U == Divider) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_SetDivider((XPm_OutClockNode *)Clk, Divider);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_SetParam((XPm_PllClockNode *)Clk,
					      (u32)PM_PLL_PARAM_ID_FBDIV, Divider);
	} else {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the clock divider.
 *
 * @param ClockId	ID of the clock node
 * @param Divider	Address to store the divider values
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetClockDivider(const u32 ClockId, u32 *const Divider)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	if (ISOUTCLK(ClockId)) {
		Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk,
						(u32)TYPE_DIV1, Divider);
	} else if (ISPLL(ClockId)) {
		Status = XPmClockPll_GetParam((XPm_PllClockNode *)Clk,
					      (u32)PM_PLL_PARAM_ID_FBDIV, Divider);
	} else {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the parent of the clock.
 *
 * @param SubsystemId	Subsystem ID.
 * @param ClockId	Clock node ID
 * @param ParentIdx	Parent clock index
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To change the clock parent, the clock must be disabled.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
XStatus XPm_SetClockParent(const u32 SubsystemId, const u32 ClockId, const u32 ParentIdx)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	/* Set parent is allowed only on output clocks */
	if (!ISOUTCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_SetParent((XPm_OutClockNode *)Clk, ParentIdx);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the clock parent.
 *
 * @param ClockId	ID of the clock node
 * @param ParentIdx	Address to store the parent clock index
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetClockParent(const u32 ClockId, u32 *const ParentIdx)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockNode *Clk = XPmClock_GetById(ClockId);

	/* Get parent is allowed only on output clocks */
	if (!ISOUTCLK(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClock_GetClockData((XPm_OutClockNode *)Clk, (u32)TYPE_MUX, ParentIdx);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the parameter of PLL clock.
 *
 * @param SubsystemId	Subsystem ID
 * @param ClockId	ID of the clock node
 * @param ParmaId	ID of the parameter
 * @param Value		Value of the parameter
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_SetPllParameter(const u32 SubsystemId, const u32 ClockId, const u32 ParamId, const u32 Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	/* Check if subsystem is allowed to access requested clock or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Status = XPmClockPll_SetParam(Clock, ParamId, Value);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%xn\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the parameter of PLL clock.
 *
 * @param ClockId	ID of the clock node
 * @param ParmaId	ID of the parameter
 * @param Value		Address to store parameter value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetPllParameter(const u32 ClockId, const u32 ParamId, u32 *const Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XPM_INVALID_CLKID;
		goto done;
	}

	Status = XPmClockPll_GetParam(Clock, ParamId, Value);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the mode of PLL clock.
 *
 * @param SubsystemId	Subsystem ID
 * @param ClockId	ID of the clock node
 * @param Value		Pll mode value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_SetPllMode(const u32 SubsystemId, const u32 ClockId, const u32 Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Check if subsystem is allowed to access requested pll or not */
	Status = XPm_IsAccessAllowed(SubsystemId, ClockId);
	if (Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_SetMode(Clock, Value);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the mode of PLL clock.
 *
 * @param ClockId	ID of the clock node
 * @param Value		Address to store mode value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetPllMode(const u32 ClockId, u32 *const Value)
{
	XStatus Status = XST_FAILURE;
	XPm_PllClockNode* Clock;

	if (!ISPLL(ClockId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Clock = (XPm_PllClockNode *)XPmClock_GetById(ClockId);
	if (NULL == Clock) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmClockPll_GetMode(Clock, Value);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reset or de-reset a device.
 *
 * @param SubsystemId	Subsystem ID
 * @param ResetId	Reset ID
 * @param IpiMask	IPI Mask currently being used
 * @param Action	Reset (true) or de-reset (false) the device
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   To de-reset a device, the subsystem must be using the device, and
 * all the upstream reset line(s) must be de-asserted.  To reset a device, the
 * subsystem must be the only user of the device, and all downstream devices
 * (in terms of reset dependency) must be already in reset state.  Otherwise,
 * this request will be denied.
 *
 ****************************************************************************/
XStatus XPm_SetResetState(const u32 SubsystemId, const u32 IpiMask,
			  const u32 ResetId, const u32 Action)
{
	int Status = XST_FAILURE;
	XPm_ResetNode* Reset;
	u32 SubClass = NODESUBCLASS(ResetId);
	u32 SubType = NODETYPE(ResetId);

	Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * XSDB is a privileged master, allow unrestricted access.
	 */
	if (XSDB_IPI_INT_MASK != IpiMask) {
		/*
		 * Only peripheral and debug resets
		 * are allowed to control externally, on other masters.
		 */
		if ((u32)XPM_NODESUBCL_RESET_PERIPHERAL == SubClass) {
			if ((u32)XPM_NODETYPE_RESET_PERIPHERAL != SubType) {
				Status = XPM_PM_NO_ACCESS;
				goto done;
			}
		} else if ((u32)XPM_NODESUBCL_RESET_DBG == SubClass) {
			if ((u32)XPM_NODETYPE_RESET_DBG != SubType) {
				Status = XPM_PM_NO_ACCESS;
				goto done;
			}
		} else {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}

		/* Check if subsystem is allowed to access requested reset */
		Status = XPm_IsAccessAllowed(SubsystemId, ResetId);
		if (XST_SUCCESS != Status) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		}
	}

	Status = Reset->Ops->SetState(Reset, Action);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the device reset state.
 *
 * @param  ResetId		Reset ID
 * @param State		Pointer to the reset state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetResetState(const u32 ResetId, u32 *const State)
{
	int Status = XST_FAILURE;
	XPm_ResetNode* Reset;

	Reset = XPmReset_GetById(ResetId);
	if (NULL == Reset) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	*State = Reset->Ops->GetState(Reset);

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function requests the pin.
 *
 * @param SubsystemId	Subsystem ID
 * @param PinId		ID of the pin node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_PinCtrlRequest(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_Request(SubsystemId, PinId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function releases the pin.
 *
 * @param SubsystemId	Subsystem ID
 * @param PinId		ID of the pin node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_PinCtrlRelease(const u32 SubsystemId, const u32 PinId)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_Release(SubsystemId, PinId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the pin function.
 *
 * @param SubsystemId	Subsystem ID
 * @param PinId			Pin node ID
 * @param FunctionId	Function for the pin
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If no change to the pin function setting is required (the pin is
 * already set up for this function), this call will be successful.
 * Otherwise, the request is denied unless the subsystem has already
 * requested this pin.
 *
 ****************************************************************************/
XStatus XPm_SetPinFunction(const u32 SubsystemId,
	const u32 PinId, const u32 FunctionId)
{
	XStatus Status = XST_FAILURE;

	/* Check if subsystem is allowed to access or not */
	Status = XPm_IsAccessAllowed(SubsystemId, PinId);
	if(Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmPin_CheckPerms(SubsystemId, PinId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmPin_SetPinFunction(PinId, FunctionId);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the pin function.
 *
 * @param PinId			ID of the pin node
 * @param FunctionId	Address to store the function
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetPinFunction(const u32 PinId, u32 *const FunctionId)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinFunction(PinId, FunctionId);

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the pin parameter value.
 *
 * @param  SubsystemId	Subsystem ID
 * @param PinId			Pin node ID
 * @param ParamId		Pin parameter ID
 * @param ParamVal		Pin parameter value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If no change to the pin parameter setting is required (the pin
 * parameter is already set up for this value), this call will be successful.
 * Otherwise, the request is denied unless the subsystem has already
 * requested this pin.
 *
 ****************************************************************************/
XStatus XPm_SetPinParameter(const u32 SubsystemId, const u32 PinId,
			const u32 ParamId,
			const u32 ParamVal)
{
	XStatus Status = XST_FAILURE;

	/* Check if subsystem is allowed to access or not */
	Status = XPm_IsAccessAllowed(SubsystemId, PinId);
	if(Status != XST_SUCCESS) {
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	Status = XPmPin_CheckPerms(SubsystemId, PinId);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmPin_SetPinConfig(PinId, ParamId, ParamVal);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function reads the pin parameter value.
 *
 * @param PinId		ID of the pin node
 * @param ParamId	Pin parameter ID
 * @param ParamVal	Address to store the pin parameter value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_GetPinParameter(const u32 PinId,
			const u32 ParamId,
			u32 * const ParamVal)
{
	XStatus Status = XST_FAILURE;

	Status = XPmPin_GetPinConfig(PinId, ParamId, ParamVal);

	return Status;
}

/****************************************************************************/
/**
 * @brief  Enable/Disable tap delay bypass
 *
 * @param  DeviceId	ID of the device
 * @param  Type		Type of tap delay to enable/disable (QSPI)
 * @param  Value	Enable/Disable
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_SetTapdelayBypass(const u32 DeviceId, const u32 Type,
				 const u32 Value)
{
	int Status = XST_FAILURE;
	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 BaseAddress;

	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* QSPI base address */
	BaseAddress = Device->Node.BaseAddress;

	if (((XPM_TAPDELAY_BYPASS_ENABLE != Value) &&
	    (XPM_TAPDELAY_BYPASS_DISABLE != Value)) ||
	    (XPM_TAPDELAY_QSPI != Type)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PmRmw32(BaseAddress + TAPDLY_BYPASS_OFFSET, XPM_TAP_DELAY_MASK,
		Value << Type);

	Status = XST_SUCCESS;

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function resets DLL logic for the SD device.
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Reset type
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_SdDllReset(const u32 DeviceId, const u32 Type)
{
	int Status = XST_FAILURE;
	XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	u32 BaseAddress;
	u32 Offset;

	if (NULL == Pmc) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Pmc->PmcIouSlcrBaseAddr;

	if (PM_DEV_SDIO_0 == DeviceId) {
		Offset = SD0_CTRL_OFFSET;
	} else if (PM_DEV_SDIO_1 == DeviceId) {
		Offset = SD1_CTRL_OFFSET;
	} else {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	switch (Type) {
	case XPM_DLL_RESET_ASSERT:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			XPM_SD_DLL_RST_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_DLL_RESET_RELEASE:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			~XPM_SD_DLL_RST_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_DLL_RESET_PULSE:
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			XPM_SD_DLL_RST_MASK);
		PmRmw32(BaseAddress + Offset, XPM_SD_DLL_RST_MASK,
			~XPM_SD_DLL_RST_MASK);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPM_INVALID_TYPEID;
		break;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets input/output tap delay for the SD device.
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Type of tap delay to set (input/output)
 * @param  Value	Value to set for the tap delay
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_SetSdTapDelay(const u32 DeviceId, const u32 Type,
			     const u32 Value)
{
	int Status = XST_FAILURE;
	XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 BaseAddress;

	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	/*Check for Type */
	if (XPM_TAPDELAY_INPUT != Type && XPM_TAPDELAY_OUTPUT != Type) {
		Status = XPM_INVALID_TYPEID;
		goto done;
	}
	/* SD0/1 base address */
	BaseAddress = Device->Node.BaseAddress;

	Status = XPm_SdDllReset(DeviceId, XPM_DLL_RESET_ASSERT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	switch (Type) {
	case XPM_TAPDELAY_INPUT:
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPCHGWIN_MASK,
			XPM_SD_ITAPCHGWIN_MASK);
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPDLYENA_MASK,
			XPM_SD_ITAPDLYENA_MASK);
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPDLYSEL_MASK,
			Value);
		PmRmw32(BaseAddress + ITAPDLY_OFFSET, XPM_SD_ITAPCHGWIN_MASK,
			~XPM_SD_ITAPCHGWIN_MASK);
		break;
	case XPM_TAPDELAY_OUTPUT:
		PmRmw32(BaseAddress + OTAPDLY_OFFSET, XPM_SD_OTAPDLYENA_MASK,
			XPM_SD_OTAPDLYENA_MASK);
		PmRmw32(BaseAddress + OTAPDLY_OFFSET, XPM_SD_OTAPDLYSEL_MASK,
			Value);
		break;
	default:
		/*No action taken because we check for type earlier in the function
		 * but present as part of defensive programming in case we reach here */
		break;
	}

	Status = XPm_SdDllReset(DeviceId, XPM_DLL_RESET_RELEASE);

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs read/write operation on probe counter
 *         registers of LPD/FPD.
 *
 * @param  DeviceId	DeviceId of the LPD/FPD
 * @param  Arg1		- Counter Number (0 to 7 bit)
 *			- Register Type (8 to 15 bit)
 *                         0 - LAR_LSR access (Request Type is ignored and
 *                                             Counter Number is ignored)
 *                         1 - Main Ctl       (Counter Number is ignored)
 *                         2 - Config Ctl     (Counter Number is ignored)
 *                         3 - State Period   (Counter Number is ignored)
 *                         4 - PortSel
 *                         5 - Src
 *                         6 - Val
 *                      - Request Type (16 to 23 bit)
 *                         0 - Read Request
 *                         1 - Read Response
 *                         2 - Write Request
 *                         3 - Write Response
 *                         4 - lpd Read Request (For LPD only)
 *                         5 - lpd Read Response (For LPD only)
 *                         6 - lpd Write Request (For LPD only)
 *                         7 - lpd Write Response (For LPD only)
 * @param  Value	Register value to write (if Write flag is 1)
 * @param  Response	Value of register read (if Write flag is 0)
 * @param  Write	Operation type (0 - Read, 1 - Write)
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_ProbeCounterAccess(u32 DeviceId, u32 Arg1, u32 Value,
				  u32 *const Response, u8 Write)
{
	int Status = XST_INVALID_PARAM;
	XPm_Power *Power;
	u32 Reg;
	u32 CounterIdx;
	u32 ReqType;
	u32 ReqTypeOffset;
	u32 FpdReqTypeOffset[] = {
		PROBE_COUNTER_FPD_RD_REQ_OFFSET,
		PROBE_COUNTER_FPD_RD_RES_OFFSET,
		PROBE_COUNTER_FPD_WR_REQ_OFFSET,
		PROBE_COUNTER_FPD_WR_RES_OFFSET,
	};

	CounterIdx = Arg1 & PROBE_COUNTER_IDX_MASK;
	ReqType = ((Arg1 >> PROBE_COUNTER_REQ_TYPE_SHIFT) &
		   PROBE_COUNTER_REQ_TYPE_MASK);
	switch (NODEINDEX(DeviceId)) {
	case (u32)XPM_NODEIDX_POWER_LPD:
		if (CounterIdx > PROBE_COUNTER_CPU_R5_MAX_IDX) {
			goto done;
		}
		if (ReqType > PROBE_COUNTER_LPD_MAX_REQ_TYPE) {
			goto done;
		} else if ((ReqType > PROBE_COUNTER_CPU_R5_MAX_REQ_TYPE) &&
			   (CounterIdx > PROBE_COUNTER_LPD_MAX_IDX)) {
			goto done;
		} else {
			/* Required due to MISRA */
			PmDbg("[%d] Unknown else case\r\n", __LINE__);
		}
		Reg = CORESIGHT_LPD_ATM_BASE;
		ReqTypeOffset = (ReqType * PROBE_COUNTER_LPD_REQ_TYPE_OFFSET);
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODEIDX_POWER_FPD:
		if (CounterIdx > PROBE_COUNTER_FPD_MAX_IDX) {
			goto done;
		}
		if (ReqType > PROBE_COUNTER_FPD_MAX_REQ_TYPE) {
			goto done;
		}
		Reg = CORESIGHT_FPD_ATM_BASE;
		ReqTypeOffset = FpdReqTypeOffset[ReqType];
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPM_PM_INVALID_NODE;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Power = XPmPower_GetById(DeviceId);
	if ((NULL == Power) || ((u8)XPM_POWER_STATE_ON != Power->Node.State)) {
		goto done;
	}

	switch ((Arg1 >> PROBE_COUNTER_TYPE_SHIFT) & PROBE_COUNTER_TYPE_MASK) {
	case XPM_PROBE_COUNTER_TYPE_LAR_LSR:
		if (1U == Write) {
			Reg += PROBE_COUNTER_LAR_OFFSET;
		} else {
			Reg += PROBE_COUNTER_LSR_OFFSET;
		}
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_MAIN_CTL:
		Reg += ReqTypeOffset + PROBE_COUNTER_MAIN_CTL_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_CFG_CTL:
		Reg += ReqTypeOffset + PROBE_COUNTER_CFG_CTL_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_STATE_PERIOD:
		Reg += ReqTypeOffset + PROBE_COUNTER_STATE_PERIOD_OFFSET;
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_PORT_SEL:
		Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_PORT_SEL_OFFSET);
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_SRC:
		Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_SRC_OFFSET);
		Status = XST_SUCCESS;
		break;
	case XPM_PROBE_COUNTER_TYPE_VAL:
		if (1U == Write) {
			/* This type doesn't support write operation */
			goto done;
		}
		Reg += (ReqTypeOffset + (CounterIdx * 20U) +
			PROBE_COUNTER_VAL_OFFSET);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (0U == Write) {
		if (NULL == Response) {
			Status = XST_FAILURE;
			goto done;
		}
		PmIn32(Reg, *Response);
	} else {
		PmOut32(Reg, Value);
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function sets the controller into either D3 or D0 state
 *
 * @param  DeviceId	DeviceId of the device
 * @param  ReqState	Requested State (0 - D0, 3 - D3)
 * @param  TimeOut	TimeOut value in micro secs to wait for D3/D0 entry
 *
 * @return XST_SUCCESS if successful else error code
 *
 ****************************************************************************/
static int XPm_USBDxState(const u32 DeviceId, const u32 ReqState,
			  const u32 TimeOut)
{
	int Status = XST_FAILURE;
	XPm_Pmc *Pmc;
	u32 BaseAddress;
	u32 Offset;
	u32 CurState;
	u32 LocalTimeOut;

	(void)DeviceId;
	LocalTimeOut = TimeOut;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);

	if (NULL == Pmc) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Only (D0 == 0U) or (D3 == 3U) states allowed */
	if ((ReqState != 0U) && (ReqState != 3U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Pmc->PmcIouSlcrBaseAddr;;

	/* power state request */
	Offset = XPM_USB_PWR_REQ_OFFSET;
	PmOut32(BaseAddress + Offset, ReqState);

	/* current power state */
	Offset = XPM_USB_CUR_PWR_OFFSET;
	PmIn32(BaseAddress + Offset, CurState);

	while((CurState != ReqState) && (LocalTimeOut > 0U)) {
		LocalTimeOut--;
		PmIn32(BaseAddress + Offset, CurState);
		usleep(1U);
	}

	Status = ((LocalTimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function selects/returns the AXI interface to OSPI device
 *
 * @param  DeviceId	DeviceId of the device
 * @param  Type		Reset type
 * @param  Response	Output Response (0 - DMA, 1 - LINEAR)
 *
 * @return XST_SUCCESS if successful else error code or a reason code
 *
 ****************************************************************************/
static int XPm_OspiMuxSelect(const u32 DeviceId, const u32 Type, u32 *Response)
{
	int Status = XST_FAILURE;
	XPm_Pmc *Pmc;
	u32 BaseAddress;
	u32 Offset;

	(void)DeviceId;

	Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* PMC_IOU_SLCR base address */
	BaseAddress = Pmc->PmcIouSlcrBaseAddr;
	Offset = XPM_OSPI_MUX_SEL_OFFSET;

	switch (Type) {
	case XPM_OSPI_MUX_SEL_DMA:
		PmRmw32(BaseAddress + Offset, XPM_OSPI_MUX_SEL_MASK,
				~XPM_OSPI_MUX_SEL_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_OSPI_MUX_SEL_LINEAR:
		PmRmw32(BaseAddress + Offset, XPM_OSPI_MUX_SEL_MASK,
				XPM_OSPI_MUX_SEL_MASK);
		Status = XST_SUCCESS;
		break;
	case XPM_OSPI_MUX_GET_MODE:
		if (NULL == Response) {
			Status = XST_INVALID_PARAM;
			goto done;
		}
		PmIn32(BaseAddress + Offset, *Response);
		*Response = (((*Response) & XPM_OSPI_MUX_SEL_MASK) >>
			XPM_OSPI_MUX_SEL_SHIFT);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function performs driver-like IOCTL functions on shared system
 * devices.
 *
 * @param SubsystemId		ID of the subsystem
 * @param DeviceId		ID of the device
 * @param IoctlId		IOCTL function ID
 * @param Arg1			Argument 1
 * @param Arg2			Argument 2
 * @param Response		Ioctl response
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   If no change to the pin parameter setting is required (the pin
 * parameter is already set up for this value), this call will be successful.
 * Otherwise, the request is denied unless the subsystem has already
 * requested this pin.
 *
 ****************************************************************************/
XStatus XPm_DevIoctl(const u32 SubsystemId, const u32 DeviceId,
			const u32 IoctlId,
			const u32 Arg1,
			const u32 Arg2, u32 *const Response)
{
	XStatus Status = XPM_ERR_IOCTL;

	switch (IoctlId) {
	case (u32)IOCTL_GET_RPU_OPER_MODE:
		if ((DeviceId != PM_DEV_RPU0_0) &&
		    (DeviceId != PM_DEV_RPU0_1)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		XPm_RpuGetOperMode(DeviceId, Response);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_SET_RPU_OPER_MODE:
		if ((DeviceId != PM_DEV_RPU0_0) &&
		    (DeviceId != PM_DEV_RPU0_1)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		XPm_RpuSetOperMode(DeviceId, Arg1);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_RPU_BOOT_ADDR_CONFIG:
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
			goto done;
		}
		Status = XPm_RpuBootAddrConfig(DeviceId, Arg1);
		break;
	case (u32)IOCTL_TCM_COMB_CONFIG:
		if ((PM_DEV_RPU0_0 != DeviceId) &&
		    (PM_DEV_RPU0_1 != DeviceId)) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
		Status = XPm_RpuTcmCombConfig(DeviceId, Arg1);
		break;
	case (u32)IOCTL_SET_TAPDELAY_BYPASS:
		if (PM_DEV_QSPI != DeviceId) {
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}

		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SetTapdelayBypass(DeviceId, Arg1, Arg2);
		break;
	case (u32)IOCTL_SD_DLL_RESET:
		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SdDllReset(DeviceId, Arg1);
		break;
	case (u32)IOCTL_SET_SD_TAPDELAY:
		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_SetSdTapDelay(DeviceId, Arg1, Arg2);
		break;
	case (u32)IOCTL_WRITE_GGS:
		if (Arg1 >= GGS_NUM_REGS) {
			goto done;
		}
		PmOut32((GGS_BASEADDR + (Arg1 << 2)), Arg2);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_READ_GGS:
		if (Arg1 >= GGS_NUM_REGS) {
			goto done;
		}
		PmIn32((GGS_BASEADDR + (Arg1 << 2)), *Response);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_WRITE_PGGS:
		if (Arg1 >= PGGS_NUM_REGS) {
			goto done;
		}
		PmOut32((PGGS_BASEADDR + (Arg1 << 2)), Arg2);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_READ_PGGS:
		if (Arg1 >= PGGS_NUM_REGS) {
			goto done;
		}
		PmIn32((PGGS_BASEADDR + (Arg1 << 2)), *Response);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_SET_BOOT_HEALTH_STATUS:
		PmRmw32(GGS_BASEADDR + GGS_4_OFFSET,
			XPM_BOOT_HEALTH_STATUS_MASK, Arg1);
		Status = XST_SUCCESS;
		break;
	case (u32)IOCTL_PROBE_COUNTER_READ:
		Status = XPm_ProbeCounterAccess(DeviceId, Arg1, Arg2,
						Response, 0U);
		break;
	case (u32)IOCTL_PROBE_COUNTER_WRITE:
		Status = XPm_ProbeCounterAccess(DeviceId, Arg1, Arg2,
						Response, 1U);
		break;
	case (u32)IOCTL_OSPI_MUX_SELECT:
		if (PM_DEV_OSPI != DeviceId) {
			goto done;
		}

		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		Status = XPm_OspiMuxSelect(DeviceId, Arg1, Response);
		break;
	case (u32)IOCTL_USB_SET_STATE:
		if (PM_DEV_USB_0 != DeviceId) {
			goto done;
		}

		Status = XPm_IsAccessAllowed(SubsystemId, DeviceId);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_USBDxState(DeviceId, Arg1, Arg2);
		break;
	default:
		/* Not supported yet */
		Status = XPM_ERR_IOCTL;
		break;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function initializes subsystem and releases unused devices
 *
 * @param SubsystemId  ID of the subsystem
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
int XPm_InitFinalize(const u32 SubsystemId)
{
	return XPmSubsystem_InitFinalize(SubsystemId);
}

/****************************************************************************/
/**
 * @brief  This function provides topology information
 *
 * @param  Args		topology information data
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_DescribeNodes(u32 NumArgs)
{
	int Status = XST_FAILURE;
	/*u32 NumPwrNodes, NumClkNodes, NumRstNodes, NumMioNodes, NumDevices;*/

	if(NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Uncomment when AllocNodes will implement */
	/*NumPwrNodes = (Args[0] >> 16) & 0xFFFF;
	NumClkNodes = Args[0] & 0xFFFF;
	NumRstNodes = (Args[1] >> 16) & 0xFFFF;
	NumMioNodes = Args[1] & 0xFFFF;
	NumDevices = Args[2] & 0xFFFF;

	if(NumClkNodes != 0)
		Status = XPmClock_AllocNodes(NumClkNodes);
	if(NumPwrNodes != 0)
		Status = XPmPower_AllocNodes(NumPwrNodes);
	if(NumRstNodes != 0)
		Status = XPmReset_AllocNodes(NumRstNodes);
	if (NumMioNodes != 0)
		Status = XPmMio_AllocNodes(NumMioNodes);
	if (NumDevices != 0)
		Status = XPmDevice_Alloc(NumDevices);*/

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows adding parent to any node or device
 *
 * @param  Args		Parent ids
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddNodeParent(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 Id = Args[0];
	u32 *Parents;
	u32 NumParents;

	if (NumArgs < 2U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NumParents = NumArgs-1U;
	Parents = &Args[1];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_AddParent(Id, Parents, NumParents);
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		if (ISPLL(Id)) {
			Status = XPmClockPll_AddParent(Id, Parents, (u8)NumParents);
		} else {
			Status = XPmClock_AddParent(Id, Parents, (u8)NumParents);
		}
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_MEMIC:
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_STMIC:
		Status = XST_SUCCESS;
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPmDevice_AddParent(Id, Parents, NumParents);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function adds sub nodes for clocks having custom topology
 *
 * @param  Args		topology node arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddClockSubNode(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 ClockId, ControlReg, Type, Flags;
	u8 Param1, Param2;

	if (NumArgs < 5U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ClockId = Args[0];
	if (ISOUTCLK(ClockId)) {
		Type = Args[1];
		ControlReg = Args[2];
		Param1 =  (u8)(Args[3] & 0xFFU);
		Param2 =  (u8)((Args[3] >> 8U) & 0xFFU);
		Flags = Args[4];
		Status = XPmClock_AddSubNode(ClockId, Type, ControlReg, Param1, Param2, Flags);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add clock node to clock topology database
 *
 * @param  Args		clock arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeClock(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 ClockId, ControlReg;
	u32 PowerDomainId;
	u8 TopologyType, NumCustomNodes=0, NumParents, ClkFlags;

	if (NumArgs < 4U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	ClockId = Args[0];

	if (NODETYPE(ClockId) == (u32)XPM_NODETYPE_CLOCK_SUBNODE) {
		Status = XPm_AddClockSubNode(Args, NumArgs);
		goto done;
	}
	if (ISOUTCLK(ClockId) || ISREFCLK(ClockId) || ISPLL(ClockId)) {
		ControlReg = Args[1];
		TopologyType = (u8)(Args[2] & 0xFFU);
		NumCustomNodes = (u8)((Args[2] >> 8U) & 0xFFU);
		NumParents = (u8)((Args[2] >> 16U) & 0xFFU);
		ClkFlags = (u8)((Args[2] >> 24U) & 0xFFU);
		PowerDomainId = Args[3];
		if (ISPLL(ClockId)) {
			u16 *Offsets = (u16 *)&Args[4];
			Status = XPmClockPll_AddNode(ClockId, ControlReg,
						     TopologyType, Offsets,
						     PowerDomainId, ClkFlags);
		} else {
			Status = XPmClock_AddNode(ClockId, ControlReg,
						  TopologyType, NumCustomNodes,
						  NumParents, PowerDomainId,
						  ClkFlags);
		}
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function adds node name
 *
 * @param  Args		name
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddNodeName(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 NodeId;
	char Name[MAX_NAME_BYTES] = {0};
	u32 i=0, j=0;

	if (NumArgs == 0U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	NodeId = Args[0];
	if (ISOUTCLK(NodeId) || ISREFCLK(NodeId) || ISPLL(NodeId)) {
		for (i = 1U; i < NumArgs; i++) {
			(void)memcpy(&Name[j], (char *)((UINTPTR)&Args[i]), 4U);
			j += 4U;
		}
		Status = XPmClock_AddClkName(NodeId, Name);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add power node to power topology database
 *
 * @param  Args		power arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodePower(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 PowerId;
	u32 PowerType;
	u8 Width;
	u8 Shift;
	u32 BitMask;
	u32 ParentId;
	XPm_Power *Power;
	XPm_Power *PowerParent = NULL;
	XPm_PsFpDomain *PsFpDomain;
	XPm_PmcDomain *PmcDomain;
	XPm_PsLpDomain *PsLpDomain;
	XPm_NpDomain *NpDomain;
	XPm_PlDomain *PlDomain;
	XPm_AieDomain *AieDomain;
	XPm_CpmDomain *CpmDomain;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PowerId = Args[0];
	PowerType = NODETYPE(PowerId);
	Width = (u8)(Args[1] >> 8) & 0xFFU;
	Shift = (u8)(Args[1] & 0xFFU);
	ParentId = Args[2];

	if (NODECLASS(PowerId) != (u32)XPM_NODECLASS_POWER) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else if (NODEINDEX(PowerId) >= (u32)XPM_NODEIDX_POWER_MAX) {
		Status = XST_INVALID_PARAM;
		goto done;
	} else {
		/* Required by MISRA */
	}

	BitMask = BITNMASK(Shift, Width);

	if (ParentId != (u32)XPM_NODEIDX_POWER_MIN) {
		if (NODECLASS(ParentId) != (u32)XPM_NODECLASS_POWER) {
			Status = XST_INVALID_PARAM;
			goto done;
		} else if (NODEINDEX(ParentId) >= (u32)XPM_NODEIDX_POWER_MAX) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		} else {
			/* Required by MISRA */
		}

		PowerParent = XPmPower_GetById(ParentId);
		if (NULL == PowerParent) {
			Status = XST_DEVICE_NOT_FOUND;
			goto done;
		}
	}

	switch (PowerType) {
	case (u32)XPM_NODETYPE_POWER_ISLAND:
	case (u32)XPM_NODETYPE_POWER_ISLAND_XRAM:
		Power = (XPm_Power *)XPm_AllocBytes(sizeof(XPm_Power));
		if (NULL == Power) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPower_Init(Power, PowerId, BitMask,
			PowerParent);
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PMC:
		PmcDomain =
			(XPm_PmcDomain *)XPm_AllocBytes(sizeof(XPm_PmcDomain));
		if (NULL == PmcDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPmcDomain_Init((XPm_PmcDomain *)PmcDomain, PowerId);
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PS_FULL:
		PsFpDomain =
			(XPm_PsFpDomain *)XPm_AllocBytes(sizeof(XPm_PsFpDomain));
		if (NULL == PsFpDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPsFpDomain_Init(PsFpDomain, PowerId,
					    BitMask, PowerParent, &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PS_LOW:
		PsLpDomain =
			(XPm_PsLpDomain *)XPm_AllocBytes(sizeof(XPm_PsLpDomain));
		if (NULL == PsLpDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPsLpDomain_Init(PsLpDomain, PowerId,
					    BitMask, PowerParent,
					    &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_NOC:
		NpDomain = (XPm_NpDomain *)XPm_AllocBytes(sizeof(XPm_NpDomain));
		if (NULL == NpDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmNpDomain_Init(NpDomain, PowerId, 0x00000000,
					  PowerParent);
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_PL:
		PlDomain = (XPm_PlDomain *)XPm_AllocBytes(sizeof(XPm_PlDomain));
		if (NULL == PlDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPlDomain_Init(PlDomain, PowerId, 0x00000000,
					  PowerParent, &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_CPM:
		CpmDomain = (XPm_CpmDomain *)XPm_AllocBytes(sizeof(XPm_CpmDomain));
		if (NULL == CpmDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmCpmDomain_Init(CpmDomain, PowerId, 0x00000000, PowerParent,
					   &Args[3], (NumArgs - 3U));
		break;
	case (u32)XPM_NODETYPE_POWER_DOMAIN_ME:
		AieDomain = (XPm_AieDomain *)XPm_AllocBytes(sizeof(XPm_AieDomain));
		if (NULL == AieDomain) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmAieDomain_Init(AieDomain, PowerId, BitMask, PowerParent);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add reset node to reset topology database
 *
 * @param  Args		reset arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeReset(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 ResetId, ControlReg;
	u8 Shift, Width, ResetType, NumParents;
	u32 *Parents;

	if (NumArgs < 4U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	ResetId = Args[0];
	ControlReg = Args[1];
	Shift = (u8)(Args[2] & 0xFFU);
	Width = (u8)((Args[2] >> 8U) & 0xFFU);
	ResetType = (u8)((Args[2] >> 16U) & 0xFFU);
	NumParents = (u8)((Args[2] >> 24U) & 0xFFU);
	Parents = &Args[3];

	Status = XPmReset_AddNode(ResetId, ControlReg, Shift, Width, ResetType, NumParents, Parents);

done:
	return Status;
}

static XStatus AddProcDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 Index;

	XPm_Psm *Psm;
	XPm_Pmc *Pmc;
	XPm_ApuCore *ApuCore;
	XPm_RpuCore *RpuCore;
	XPm_Power *Power;
	u32 BaseAddr[MAX_BASEADDR_LEN];
	u32 Ipi;

	DeviceId = Args[0];
	BaseAddr[0] = Args[2];
	Ipi = Args[3];
	BaseAddr[1] = Args[4];
	BaseAddr[2] = Args[5];

	Type = NODETYPE(DeviceId);
	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (Index >= (u32)XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_CORE_PSM:
		Psm = (XPm_Psm *)XPm_AllocBytes(sizeof(XPm_Psm));
		if (NULL == Psm) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPsm_Init(Psm, Ipi, BaseAddr, Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_CORE_APU:
		ApuCore = (XPm_ApuCore *)XPm_AllocBytes(sizeof(XPm_ApuCore));
		if (NULL == ApuCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmApuCore_Init(ApuCore, DeviceId, Ipi, BaseAddr, Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_CORE_RPU:
		RpuCore = (XPm_RpuCore *)XPm_AllocBytes(sizeof(XPm_RpuCore));
		if (NULL == RpuCore) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmRpuCore_Init(RpuCore, DeviceId, Ipi, BaseAddr, Power, NULL, NULL);
		break;
	case (u32)XPM_NODETYPE_DEV_CORE_PMC:
		Pmc = (XPm_Pmc *)XPm_AllocBytes(sizeof(XPm_Pmc));
		if (NULL == Pmc) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmPmc_Init(Pmc, DeviceId, 0, BaseAddr, Power, NULL, NULL);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus AddPeriphDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Index;
	u32 GicProxyMask;
	u32 GicProxyGroup;

	XPm_Periph *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	GicProxyMask = Args[3];
	GicProxyGroup = Args[4];

	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (Index >= (u32)XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	Device = (XPm_Periph *)XPm_AllocBytes(sizeof(XPm_Periph));
	if (NULL == Device) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmPeriph_Init(Device, DeviceId, BaseAddr, Power, NULL, NULL,
				GicProxyMask, GicProxyGroup);

done:
	return Status;
}

static XStatus AddMemDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	u32 Index;

	XPm_MemDevice *Device;
	XPm_Power *Power;
	u32 BaseAddr;
	u32 StartAddr;
	u32 EndAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];
	StartAddr = Args[3];
	EndAddr = Args[4];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);
	Index = NODEINDEX(DeviceId);

	if (Index >= (u32)XPM_NODEIDX_DEV_MAX) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_OCM:
	case (u32)XPM_NODETYPE_DEV_XRAM:
	case (u32)XPM_NODETYPE_DEV_L2CACHE:
	case (u32)XPM_NODETYPE_DEV_DDR:
	case (u32)XPM_NODETYPE_DEV_TCM:
	case (u32)XPM_NODETYPE_DEV_EFUSE:
		Device = (XPm_MemDevice *)XPm_AllocBytes(sizeof(XPm_MemDevice));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmMemDevice_Init(Device, DeviceId, BaseAddr, Power, NULL, NULL, StartAddr, EndAddr);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus AddMemCtrlrDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_DDR:
		Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmDevice_Init(Device, DeviceId, BaseAddr,
					Power, NULL, NULL);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static XStatus AddPhyDevice(u32 *Args, u32 PowerId)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	u32 Type;
	XPm_Device *Device;
	XPm_Power *Power;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Type = NODETYPE(DeviceId);

	if (NULL != XPmDevice_GetById(DeviceId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_GT:
		Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
		if (NULL == Device) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmDevice_Init(Device, DeviceId, BaseAddr,
					Power, NULL, NULL);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

static int AddPlDevice(u32 *Args, u32 PowerId)
{
	int Status = XST_FAILURE;
	u32 DeviceId;
	u32 Index;
	XPm_Power *Power;
	XPm_Device *Device;
	u32 BaseAddr;

	DeviceId = Args[0];
	BaseAddr = Args[2];

	Index = NODEINDEX(DeviceId);

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	if ((u32)XPM_NODEIDX_DEV_PLD_MAX <= Index) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* Check if Device is already added or not. */
	if (NULL != XPmDevice_GetById(DeviceId)) {
		PmWarn("0x%x Device is already added\r\n", DeviceId);
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	Device = (XPm_Device *)XPm_AllocBytes(sizeof(XPm_Device));
	if (NULL == Device) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Status = XPmDevice_Init(Device, DeviceId, BaseAddr, Power, NULL, NULL);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function adds device node to device topology database
 *
 * @param  Args		device specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddDevice(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 DeviceId;
	u32 SubClass;
	u32 PowerId;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DeviceId = Args[0];
	SubClass = NODESUBCLASS(DeviceId);
	PowerId = Args[1];

	if (NULL == XPmPower_GetById(PowerId)) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	switch (SubClass) {
	case (u32)XPM_NODESUBCL_DEV_CORE:
		Status = AddProcDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_PERIPH:
		Status = AddPeriphDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_MEM:
		Status = AddMemDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_MEM_CTRLR:
		Status = AddMemCtrlrDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_PHY:
		Status = AddPhyDevice(Args, PowerId);
		break;
	case (u32)XPM_NODESUBCL_DEV_PL:
		Status = AddPlDevice(Args, PowerId);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add memic node to the topology database
 *
 * @param Args		MEMIC arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMemIc(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 MemIcId;
	u32 BaseAddress;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MemIcId = Args[0];
	BaseAddress = Args[2];

	if ((u32)XPM_NODECLASS_MEMIC != NODECLASS(MemIcId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_MEMIC_NOC != NODESUBCLASS(MemIcId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPmNpDomain_MemIcInit(MemIcId, BaseAddress);

done:
	return Status;
}


/****************************************************************************/
/**
 * @brief  This function add monitor node to the topology database
 *
 * @param Args		arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMonitor(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 NodeId, BaseAddress, NodeType;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeId = Args[0];
	BaseAddress = Args[2];

	if ((u32)XPM_NODECLASS_MONITOR != NODECLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_MONITOR_SYSMON != NODESUBCLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeType = NODETYPE(NodeId);

	if ((((u32)XPM_NODETYPE_MONITOR_SYSMON_PMC != NodeType) &&
	    ((u32)XPM_NODETYPE_MONITOR_SYSMON_PS != NodeType)
		&& ((u32)XPM_NODETYPE_MONITOR_SYSMON_NPD != NodeType)) ||
	    ((u32)XPM_NODEIDX_MONITOR_MAX <= NODEINDEX(NodeId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	SysmonAddresses[NODEINDEX(NodeId)] = BaseAddress;
	Status = XST_SUCCESS;
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add xmpu/xppu node to the topology database
 *
 * @param Args		Node arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeProt(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 NodeId;
	u32 BaseAddress;
	u32 SubClass;
	XPm_ProtPpu *PpuNode;
	XPm_ProtMpu *MpuNode;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	NodeId = Args[0];
	BaseAddress = Args[2];
	SubClass = NODESUBCLASS(NodeId);

	if ((u32)XPM_NODECLASS_PROTECTION != NODECLASS(NodeId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	switch (SubClass) {
	case (u32)XPM_NODESUBCL_PROT_XPPU:
		PpuNode = XPm_AllocBytes(sizeof(XPm_ProtPpu));
		if (NULL == PpuNode) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmProtPpu_Init(PpuNode, NodeId, BaseAddress);
		break;
	case (u32)XPM_NODESUBCL_PROT_XMPU:
		MpuNode = XPm_AllocBytes(sizeof(XPm_ProtMpu));
		if (NULL == MpuNode) {
			Status = XST_BUFFER_TOO_SMALL;
			goto done;
		}
		Status = XPmProtMpu_Init(MpuNode, NodeId, BaseAddress);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function add mio pin node to the topology database
 *
 * @param  Args		mio arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
static XStatus XPm_AddNodeMio(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 MioId;
	u32 BaseAddress;
	XPm_PinNode *MioPin;

	if (NumArgs < 3U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MioId = Args[0];
	BaseAddress = Args[1];

	if ((u32)XPM_NODECLASS_STMIC != NODECLASS(MioId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_PIN != NODESUBCLASS(MioId)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (((u32)XPM_NODETYPE_LPD_MIO != NODETYPE(MioId)) &&
	    ((u32)XPM_NODETYPE_PMC_MIO != NODETYPE(MioId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	MioPin = (XPm_PinNode *)XPm_AllocBytes(sizeof(XPm_PinNode));
	if (NULL == MioPin) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	Status = XPmPin_Init(MioPin, MioId, BaseAddress);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function allows adding node to clock, power, reset, mio
 * 			or device topology
 *
 * @param  Args		Node specific arguments
 * @param NumArgs	number of arguments
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddNode(u32 *Args, u32 NumArgs)
{
	int Status = XST_FAILURE;
	u32 Id = Args[0];

	switch (NODECLASS(Id)) {
	case (u32)XPM_NODECLASS_POWER:
		Status = XPm_AddNodePower(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_CLOCK:
			Status = XPm_AddNodeClock(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_RESET:
		Status = XPm_AddNodeReset(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_MEMIC:
		Status = XPm_AddNodeMemIc(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_STMIC:
		Status = XPm_AddNodeMio(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		Status = XPm_AddDevice(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_PROTECTION:
		Status = XPm_AddNodeProt(Args, NumArgs);
		break;
	case (u32)XPM_NODECLASS_MONITOR:
		Status = XPm_AddNodeMonitor(Args, NumArgs);
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function links a device to a subsystem so requirement
 *         assignment could be made by XPm_RequestDevice() or
 *         XPm_SetRequirement() call.
 *
 * @param  SubsystemId	Subsystem Id
 * @param  DeviceId 	Device Id
 * @param  Flags 		Bit0-2 - No restriction/ Shared/Time Shared/Nonshared/ - 0,1,2,3
 *						Bit 3 -Secure(1)/Nonsecure(0) (Device mode)
 * @param  Params 		Optional: XPPU- master id mask for peripherals
 * @param  NumParams 	Number of params
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_AddRequirement(const u32 SubsystemId, const u32 DeviceId, u32 Flags, u32 *Params, u32 NumParams)
{
	XStatus Status = XST_INVALID_PARAM;
	XPm_Device *Device = NULL;
	XPm_Subsystem *Subsystem;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (Subsystem == NULL || Subsystem->State != (u8)ONLINE) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Device = (XPm_Device *)XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	Status = XPmRequirement_Add(Subsystem, Device, Flags, Params, NumParams);
done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}

static int XPm_GetTemperature(u32 const DeviceId, u32 *Result)
{
	int Status = XST_FAILURE;
	static XSysMonPsv SysMonInst;
	XSysMonPsv *SysMonInstPtr = &SysMonInst;
	XSysMonPsv_Config *ConfigPtr;

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		goto done;
	}

	/*
	 * TODO - need to implement getting temperature, beside the
	 * temperature of entire SoC.
	 */
	if ((u32)XPM_NODETYPE_DEV_SOC != NODETYPE(DeviceId)) {
		Status = XST_NO_FEATURE;
		goto done;
	}

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsv_LookupConfig();
	if (ConfigPtr == NULL) {
		goto done;
	}

	Status = XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	*Result = XSysMonPsv_ReadDeviceTemp(SysMonInstPtr,
					    XSYSMONPSV_VAL_VREF_MAX);
	Status = XST_SUCCESS;

done:
	return Status;
}

static int XPm_GetLatency(const u32 DeviceId, u32 *Latency)
{
	int Status = XST_SUCCESS;

	switch (NODECLASS(DeviceId)) {
	case (u32)XPM_NODECLASS_DEVICE:
		if ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(DeviceId)) {
			Status = XPmCore_GetWakeupLatency(DeviceId, Latency);
		} else {
			Status = XPmDevice_GetWakeupLatency(DeviceId, Latency);
		}
		break;
	case (u32)XPM_NODECLASS_POWER:
		Status = XPmPower_GetWakeupLatency(DeviceId, Latency);
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		if ((u32)XPM_NODESUBCL_CLOCK_PLL == NODESUBCLASS(DeviceId)) {
			Status = XPmClockPll_GetWakeupLatency(DeviceId, Latency);
		} else {
			Status = XST_INVALID_PARAM;
		}
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function gets operating characteristics of a device
 *
 * @param  DeviceId  Targeted device Id.
 * @param  Type      Type of the operating characteristics:
 *                       power, temperature, and latency
 * @param  Result    Returns the value of operating characteristic type
 *
 * @return XST_SUCCESS if successful else either XST_NO_FEATURE or XST_FAILURE.
 *
 * @note   Temperature reported in Celsius (signed Q8.7 format)
 *
 ****************************************************************************/
XStatus XPm_GetOpCharacteristic(u32 const DeviceId, u32 const Type, u32 *Result)
{
	XStatus Status = XST_FAILURE;

	switch(Type) {
	case (u32)PM_OPCHAR_TYPE_TEMP:
		Status = XPm_GetTemperature(DeviceId, Result);
		break;
	case (u32)PM_OPCHAR_TYPE_LATENCY:
		Status = XPm_GetLatency(DeviceId, Result);
		break;
	case (u32)PM_OPCHAR_TYPE_POWER:
		Status = XST_NO_FEATURE;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Register a subsystem to be notified about the device event
 *
 * @param  IpiMask      IPI mask of current subsystem
 * @param  SubsystemId  Subsystem to be notified
 * @param  NodeId     Node to which the event is related
 * @param  Event        Event in question
 * @param  Wake         Wake subsystem upon capturing the event if value 1
 * @param  Enable       Enable the registration for value 1, disable for value 0
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 ****************************************************************************/
int XPm_RegisterNotifier(const u32 SubsystemId, const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 Enable,
			 const u32 IpiMask)
{
	int Status = XST_FAILURE;
	XPm_Subsystem* Subsystem = NULL;


	/* Validate SubsystemId */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		goto done;
	}

	/* Only Event and Device Nodes are supported */
	if (((u32)XPM_NODECLASS_EVENT != NODECLASS(NodeId)) &&
		((u32)XPM_NODECLASS_DEVICE != NODECLASS(NodeId))) {
		goto done;
	}

	/* Validate other parameters */
	if ((((u32)XPM_NODECLASS_EVENT == NODECLASS(NodeId)) &&
			(Event >= XPLMI_NODEIDX_ERROR_PSMERR2_MAX)) ||
		(((u32)XPM_NODECLASS_DEVICE == NODECLASS(NodeId)) &&
			(((0U != Wake) && (1U != Wake)) ||
			((0U != Enable) && (1U != Enable)) ||
			(((u32)EVENT_STATE_CHANGE != Event) &&
			((u32)EVENT_ZERO_USERS != Event))))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (0U == Enable) {
		XPmNotifier_Unregister(Subsystem, NodeId, Event);
		Status = XST_SUCCESS;
	} else {
		Status = XPmNotifier_Register(Subsystem, NodeId, Event, Wake,
					      IpiMask);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function returns supported version of the given API.
 *
 * @param  ApiId	API ID to check
 * @param  Version	Supported version number
 *
 * @return XST_SUCCESS if successful else XST_NO_FEATURE.
 *
 * @note   None
 *
 ****************************************************************************/
int XPm_FeatureCheck(const u32 ApiId, u32 *const Version)
{
	int Status = XST_FAILURE;

	if (NULL == Version) {
		Status = XPM_ERR_VERSION;
		goto done;
	}

	switch (ApiId) {
	case PM_GET_API_VERSION:
	case PM_GET_NODE_STATUS:
	case PM_GET_OP_CHARACTERISTIC:
	case PM_REGISTER_NOTIFIER:
	case PM_REQUEST_SUSPEND:
	case PM_SELF_SUSPEND:
	case PM_FORCE_POWERDOWN:
	case PM_ABORT_SUSPEND:
	case PM_REQUEST_WAKEUP:
	case PM_SET_WAKEUP_SOURCE:
	case PM_SYSTEM_SHUTDOWN:
	case PM_REQUEST_NODE:
	case PM_RELEASE_NODE:
	case PM_SET_REQUIREMENT:
	case PM_SET_MAX_LATENCY:
	case PM_RESET_ASSERT:
	case PM_RESET_GET_STATUS:
	case PM_INIT_FINALIZE:
	case PM_GET_CHIPID:
	case PM_PINCTRL_REQUEST:
	case PM_PINCTRL_RELEASE:
	case PM_PINCTRL_GET_FUNCTION:
	case PM_PINCTRL_SET_FUNCTION:
	case PM_PINCTRL_CONFIG_PARAM_GET:
	case PM_PINCTRL_CONFIG_PARAM_SET:
	case PM_IOCTL:
	case PM_CLOCK_ENABLE:
	case PM_CLOCK_DISABLE:
	case PM_CLOCK_GETSTATE:
	case PM_CLOCK_SETDIVIDER:
	case PM_CLOCK_GETDIVIDER:
	case PM_CLOCK_SETPARENT:
	case PM_CLOCK_GETPARENT:
	case PM_PLL_SET_PARAMETER:
	case PM_PLL_GET_PARAMETER:
	case PM_PLL_SET_MODE:
	case PM_PLL_GET_MODE:
	case PM_ADD_SUBSYSTEM:
	case PM_DESTROY_SUBSYSTEM:
	case PM_DESCRIBE_NODES:
	case PM_ADD_NODE:
	case PM_ADD_NODE_PARENT:
	case PM_ADD_NODE_NAME:
	case PM_ADD_REQUIREMENT:
	case PM_SET_CURRENT_SUBSYSTEM:
	case PM_INIT_NODE:
	case PM_FEATURE_CHECK:
		*Version = XST_API_BASE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_QUERY_DATA:
		*Version = XST_API_QUERY_DATA_VERSION;
		Status = XST_SUCCESS;
		break;
	default:
		*Version = 0U;
		Status = XPM_NO_FEATURE;
		break;
	}

done:
	if(Status != XST_SUCCESS) {
		PmErr("Returned: 0x%x\n\r", Status);
	}
	return Status;
}
