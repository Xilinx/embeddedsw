/*
 * APU bare-metal: receive LPD WWDT0 expiry notification via PLM IPI using
 * XPm_RegisterNotifier over the IPI0 path.
 */

#include "wdt_notifier.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <xil_printf.h>
#include <xipipsu.h>
#include <xipipsu_hw.h>
#include <xparameters.h>
#include <xinterrupt_wrap.h>
#include <xil_exception.h>
#include "pm_api_sys.h"
#include "xil_error_node.h"
#include "mbox_cmd.h"
#include "sensor_cmd.h"

/* Seconds the RPU should keep petting its WWDT for after receiving WDT_TEST. */
#define VISP_MBOX_WDT_TEST_PET_SECONDS 60

/* IPI0 base address used for notifier callbacks. */
#define WDT_NOTIFY_IPI_BASEADDR    0xEB330000U

/* Use the local processor IPI instance for XilPM request identity. */
#if defined(XPAR_XIPIPSU_0_BASE_ADDRESS)
#define WDT_PM_IPI_BASEADDR        XPAR_XIPIPSU_0_BASE_ADDRESS
#elif defined(XPAR_XIPIPSU_0_BASEADDR)
#define WDT_PM_IPI_BASEADDR        XPAR_XIPIPSU_0_BASEADDR
#else
#error "Unable to resolve local XilPM IPI base address"
#endif

/* Runtime test variant: observed incoming IPI0 source bit for PLM callback. */
#define WDT_PLM_IPI_MASK    0x02U

/* IPI message buffer length in words. */
#define WDT_IPI_PAYLOAD_LEN XIPIPSU_MAX_MSG_LEN

static XIpiPsu s_ipi_inst;
static XIpiPsu s_pm_ipi_inst;
static int s_initialised;
static volatile int s_unregister_requested;

/* Debug/validation counters -- visible via WdtNotifier_PrintStats(). */
static volatile u32 s_ipi_irq_count;
static volatile u32 s_pm_notify_cb_count;
static volatile u32 s_wdt_expiry_count;
static volatile u32 s_unexpected_ipi_count;
static volatile u32 s_ipi_read_err_count;
static volatile u32 s_wait_poll_count;
static volatile u32 s_last_wait_isr;

static void WdtExpiredCb(XPm_Notifier *const notifier)
{
	s_wdt_expiry_count++;

	xil_printf("\r\n[APU WDT] *** LPD WWDT0 EXPIRED -- PM_NOTIFY_CB #%u ***\r\n",
		   (unsigned)s_wdt_expiry_count);
	xil_printf("[APU WDT] node=0x%08X event=0x%08X oppoint=%u\r\n",
		   (unsigned)notifier->node,
		   (unsigned)notifier->event,
		   (unsigned)notifier->oppoint);
	xil_printf("[APU WDT] received count=%u\r\n",
		   (unsigned)notifier->received);
	xil_printf("[APU WDT] stats: ipi_irq=%u notify_cb=%u expiry=%u unexpected_ipi=%u read_err=%u\r\n",
		   (unsigned)s_ipi_irq_count, (unsigned)s_pm_notify_cb_count,
		   (unsigned)s_wdt_expiry_count, (unsigned)s_unexpected_ipi_count,
		   (unsigned)s_ipi_read_err_count);

	if (notifier->received == 1U) {
		s_unregister_requested = 1;
	}
}

static XPm_Notifier s_wwdt0_notifier = {
	.callback = WdtExpiredCb,
	.node = XIL_NODETYPE_EVENT_ERROR_LPD_SLCR_ERR3,
	.event = XIL_EVENT_ERROR_MASK_LPX_WWDT0,
	.flags = 1U,
};

static void IpiIsrHandler(void *CallbackRef)
{
	XIpiPsu *ipi = (XIpiPsu *)CallbackRef;
	u32 ipi_src_mask;
	u32 src_index;

	if (ipi == NULL) {
		xil_printf("[APU WDT] IpiIsrHandler: NULL CallbackRef -- ignoring\r\n");
		return;
	}

	s_ipi_irq_count++;
	ipi_src_mask = XIpiPsu_GetInterruptStatus(ipi);

	for (src_index = 0U; src_index < ipi->Config.TargetCount; src_index++) {
		u32 mask = ipi->Config.TargetList[src_index].Mask;

		if ((ipi_src_mask & mask) == 0U) {
			continue;
		}

		if (mask == WDT_PLM_IPI_MASK) {
			u32 payload[WDT_IPI_PAYLOAD_LEN];
			int status = XIpiPsu_ReadMessage(ipi, WDT_PLM_IPI_MASK,
						 payload, WDT_IPI_PAYLOAD_LEN,
						 XIPIPSU_BUF_TYPE_MSG);
			if (status == XST_SUCCESS) {
				switch ((XPmApiCbId_t)payload[0]) {
				case PM_NOTIFY_CB:
					s_pm_notify_cb_count++;
					XPm_NotifyCb(payload[1], payload[2], payload[3]);
					break;
				default:
					xil_printf("[APU WDT] Unknown PM cb id=0x%X\r\n",
						   (unsigned)payload[0]);
					break;
				}
			} else {
				s_ipi_read_err_count++;
				xil_printf("[APU WDT] PLM IPI read error: %d (read_err_count=%u)\r\n",
					   status, (unsigned)s_ipi_read_err_count);
			}
		} else {
			s_unexpected_ipi_count++;
			xil_printf("[APU WDT] IPI from non-PLM mask=0x%08X (unexpected_count=%u)\r\n",
				   (unsigned)mask, (unsigned)s_unexpected_ipi_count);
		}

		XIpiPsu_ClearInterruptStatus(ipi, mask);
	}

	/*
	 * The RPU is dead (WWDT0 expired) and the error is persistent, so the PLM
	 * keeps re-notifying. We cannot reset from ISR context (a blocking XilPM/IPI
	 * reset returns garbage), so on a genuine WWDT0 notification we
	 * deterministically PARK this core: mask further notifier IPIs, disable
	 * interrupts, and spin in WFI. This stops the runaway APU immediately.
	 */
	if (s_pm_notify_cb_count > 0U) {
		XIpiPsu_InterruptDisable(&s_ipi_inst, XIPIPSU_ALL_MASK);
		s_unregister_requested = 1;
		xil_printf("[APU WDT] RPU stalled (watchdog expired): RPU firmware is not responding. "
			   "Halting this APU core to stop video processing. Please reset/reload the RPU firmware to recover.\r\n");
		Xil_ExceptionDisable();
		while (1) {
			__asm__ volatile ("wfi");
		}
	}
}

/**
 * @brief Print current WDT notifier debug/validation counters and state.
 */
void WdtNotifier_PrintStats(void)
{
	xil_printf("[APU WDT] ---- stats ----\r\n");
	xil_printf("[APU WDT] initialised=%d unregister_requested=%d\r\n",
		   s_initialised, s_unregister_requested);
	xil_printf("[APU WDT] ipi_irq_count=%u pm_notify_cb_count=%u wdt_expiry_count=%u\r\n",
		   (unsigned)s_ipi_irq_count, (unsigned)s_pm_notify_cb_count,
		   (unsigned)s_wdt_expiry_count);
	xil_printf("[APU WDT] unexpected_ipi_count=%u ipi_read_err_count=%u\r\n",
		   (unsigned)s_unexpected_ipi_count, (unsigned)s_ipi_read_err_count);
	xil_printf("[APU WDT] ---------------\r\n");
}

XStatus WdtNotifier_Init(void)
{
	XIpiPsu_Config *notify_ipi_cfg;
	XIpiPsu_Config *pm_ipi_cfg;
	int status;
	u32 enable_mask;

	if (s_initialised != 0) {
		return XST_SUCCESS;
	}

	notify_ipi_cfg = XIpiPsu_LookupConfig(WDT_NOTIFY_IPI_BASEADDR);
	if (notify_ipi_cfg == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	pm_ipi_cfg = XIpiPsu_LookupConfig(WDT_PM_IPI_BASEADDR);
	if (pm_ipi_cfg == NULL) {
		xil_printf("[APU WDT] FAIL: XIpiPsu_LookupConfig returned NULL for pm IPI 0x%08X\r\n",
			   (unsigned)WDT_PM_IPI_BASEADDR);
		return XST_FAILURE;
	}

	status = XIpiPsu_CfgInitialize(&s_ipi_inst, notify_ipi_cfg, notify_ipi_cfg->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("[APU WDT] FAIL: XIpiPsu_CfgInitialize(notify) status=%d\r\n", status);
		return status;
	}
	status = XIpiPsu_CfgInitialize(&s_pm_ipi_inst, pm_ipi_cfg, pm_ipi_cfg->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("[APU WDT] FAIL: XIpiPsu_CfgInitialize(pm) status=%d\r\n", status);
		return status;
	}

	status = XPm_InitXilpm(&s_pm_ipi_inst);
	if (status != XST_SUCCESS) {
		xil_printf("[APU WDT] FAIL: XPm_InitXilpm status=%d\r\n", status);
		return status;
	}

	status = XSetupInterruptSystem(&s_ipi_inst,
				       (Xil_InterruptHandler)IpiIsrHandler,
				       s_ipi_inst.Config.IntId,
				       s_ipi_inst.Config.IntrParent,
				       0xA0U);
	if (status != XST_SUCCESS) {
		xil_printf("[APU WDT] FAIL: XSetupInterruptSystem status=%d\r\n", status);
		return XST_FAILURE;
	}

	status = XPm_RegisterNotifier(&s_wwdt0_notifier);
	if (status != XST_SUCCESS) {
		xil_printf("[APU WDT] WARN: XPm_RegisterNotifier status=0x%X\r\n",
			   (unsigned)status);
	}

	XIpiPsu_ClearInterruptStatus(&s_ipi_inst, XIPIPSU_ALL_MASK);

#ifdef WDT_TEST_SELF_IPI_ENABLE
	enable_mask = XIPIPSU_ALL_MASK;
#else
	enable_mask = WDT_PLM_IPI_MASK;
#endif
	XIpiPsu_InterruptEnable(&s_ipi_inst, enable_mask);
	XIpiPsu_ClearInterruptStatus(&s_ipi_inst, XIPIPSU_ALL_MASK);

	s_initialised = 1;
	s_unregister_requested = 0;

	return XST_SUCCESS;
}

/**
 * @brief Send the WDT test command to the RPU.
 *
 * Mirrors the Linux visp_mbox_wdt_send_test() flow, but uses the bare-metal
 * mailbox Send_Command() path (same format as SetATM / INIT_FIRMWARE). The
 * payload carries the instance id followed by the number of seconds the RPU
 * should keep petting its WWDT before deliberately stopping, which lets the
 * APU-side notifier observe the resulting WWDT0 expiry.
 *
 * @return RET_SUCCESS on success, error code otherwise.
 */
int WdtSendTest(void)
{
	RESULT result = RET_SUCCESS;
	Payload_packet packet;
	uint8_t *p_data;
	uint32_t instance_id = 0;
	uint32_t pet_seconds = VISP_MBOX_WDT_TEST_PET_SECONDS;

	memset(&packet, 0, sizeof(Payload_packet));

	/* Fire-and-forget (no ACK wait): bypass RPU cookie dedup, as APU Linux. */
	packet.cookie = MBOX_COOKIE_NOT_USED;
	packet.type = CMD;
	packet.payload_size = 0;

	p_data = packet.payload_data;

	/* Payload layout mirrors INIT_FIRMWARE / SetATM: instance_id first, then
	 * the command-specific data (here, how many seconds to pet the WWDT for). */
	memcpy(p_data, &instance_id, sizeof(instance_id));
	p_data += sizeof(instance_id);
	packet.payload_size += sizeof(instance_id);

	memcpy(p_data, &pet_seconds, sizeof(pet_seconds));
	p_data += sizeof(pet_seconds);
	packet.payload_size += sizeof(pet_seconds);

	if (packet.payload_size > MAX_ITEM) {
		return RET_OUTOFRANGE;
	}

	xil_printf("[APU WDT] Sending WDT test command to RPU core %u (pet=%us)\r\n",
		   (unsigned)dest_cpu_id, (unsigned)pet_seconds);

	result = Send_Command(APU_2_RPU_MB_CMD_WDT_TEST, &packet,
			      packet.payload_size + payload_extra_size,
			      dest_cpu_id, src_cpu_id);
	if (RET_SUCCESS != result) {
		xil_printf("[APU WDT] Failed to send WDT test command, ret=%d\r\n",
			   result);
		return RET_FAILURE;
	}

	xil_printf("[APU WDT] WDT test command sent to RPU core %u (pet=%us)\r\n",
		   (unsigned)dest_cpu_id, (unsigned)pet_seconds);

	return result;
}

void WdtNotifier_Deinit(void)
{
	if (s_initialised == 0) {
		return;
	}

	(void)XPm_UnregisterNotifier(&s_wwdt0_notifier);
	XIpiPsu_InterruptDisable(&s_ipi_inst, XIPIPSU_ALL_MASK);
	XDisableIntrId(s_ipi_inst.Config.IntId, s_ipi_inst.Config.IntrParent);
	s_initialised = 0;
	xil_printf("[APU WDT] WDT notifier unregistered\r\n");
}
