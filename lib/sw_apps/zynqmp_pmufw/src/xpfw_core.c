/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_interrupts.h"
#include "xpfw_ipi_manager.h"
#include "pmu_lmb_bram.h"

#define CORE_IS_READY	((u32)0x5AFEC0DEU)
#define CORE_IS_DEAD	((u32)0xDEADBEAFU)

static XPfw_Core_t XPfwCore = { .IsReady = CORE_IS_DEAD };

/* Declare the Core Pointer as constant, since we don't intend to change it */
static XPfw_Core_t * const CorePtr = &XPfwCore;

XStatus XPfw_CoreInit(u32 Options)
{
	u32 Index;
	XStatus Status;
	if (CorePtr == NULL) {
		Status = XST_FAILURE;
		goto Done;
	}

	XPfw_InterruptInit();

	/* Clear the DONT_SLEEP bit */
	XPfw_RMW32(PMU_GLOBAL_GLOBAL_CNTRL,
		PMU_GLOBAL_GLOBAL_CNTRL_DONT_SLEEP_MASK, 0U);

	CorePtr->ModCount = (u8)0U;

	for (Index = 0U; Index < ARRAYSIZE(CorePtr->ModList); Index++) {
		Status = XPfw_ModuleInit(&CorePtr->ModList[Index], (u8) 0U);
		/* If there was an error, then just get out of here */
		if (XST_SUCCESS != Status) {
			goto Done;
		}
	}

	Status = XPfw_SchedulerInit(&CorePtr->Scheduler,
		PMU_IOMODULE_PIT1_PRELOAD);

	if (XST_SUCCESS != Status) {
		goto Done;
	}

	Status = XPfw_IpiManagerInit();

Done:
	return Status;
}

XStatus XPfw_CoreConfigure(void)
{
	u32 Idx;
	XStatus Status;

	if (CorePtr != NULL) {
		for (Idx = 0U; Idx < CorePtr->ModCount; Idx++) {

			if (CorePtr->ModList[Idx].CfgInitHandler != NULL) {
				CorePtr->ModList[Idx].CfgInitHandler(&CorePtr->ModList[Idx],
						NULL, 0U);
			}
		}

		/* We are ready to take interrupts now */
		CorePtr->IsReady = CORE_IS_READY;
		/* Clear IPI0 status and Enable IPI0 */
		XPfw_Write32(IPI_PMU_0_ISR, MASK32_ALL_HIGH);
		XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_IPI0_MASK);
		/* Clear PMU LMB BRAM ECC status and Enable this interrupt */
		XPfw_Write32(PMU_LMB_BRAM_ECC_STATUS_REG, PMU_LMB_BRAM_CE_MASK);
		XPfw_Write32(PMU_LMB_BRAM_ECC_IRQ_EN_REG, PMU_LMB_BRAM_CE_MASK);
		XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_CORRECTABLE_ECC_MASK);

		XPfw_InterruptStart();
		/* Set the FW_IS_PRESENT bit to flag that PMUFW is up and ready */
		XPfw_RMW32(PMU_GLOBAL_GLOBAL_CNTRL, PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
			PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK);
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}
	return Status;
}

XStatus XPfw_CoreDispatchEvent(u32 EventId)
{
	XStatus Status;
	u32 Idx;
	u32 CallCount = 0U;
	if ((CorePtr != NULL) && (EventId < XPFW_EV_MAX)) {

		for (Idx = 0U; Idx < CorePtr->ModCount; Idx++) {
			/**
			 * Check if Mod[Idx] and event handler are registered for this event
			 */
			if (((XPfw_EventGetModMask(EventId) & ((u32) 1U << Idx))
					== ((u32) 1U << Idx)) && CorePtr->ModList[Idx].EventHandler != NULL) {
				CorePtr->ModList[Idx].EventHandler(&CorePtr->ModList[Idx],
						EventId);
				CallCount++;
			}
		}
	}
	/* XPfw_Printf(DEBUG_INFO,"%s: Event(%d) dispatched to  %d Mods\r\n",
	 * __func__, EventId,CallCount); */
	if (CallCount > 0U) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}
	return Status;
}

XStatus XPfw_CoreDispatchIpi(u32 IpiNum, u32 SrcMask)
{
	XStatus Status;
	u32 Idx;
	u32 MaskIndex;
	u32 CallCount = 0U;
	u32 Payload[XPFW_IPI_MAX_MSG_LEN] = {0U};

	if ((CorePtr == NULL) || (IpiNum > 3U)) {
		Status = XST_FAILURE;
		goto Done;
	}

	/* For each of the IPI sources */
	for (MaskIndex = 0U; MaskIndex < XPFW_IPI_MASK_COUNT; MaskIndex++) {
		/* Check if the Mask is set */
		if ((SrcMask & IpiMaskList[MaskIndex]) != 0U) {
			/* If set, read the message into buffer */
			Status = XPfw_IpiReadMessage(IpiMaskList[MaskIndex],
						&Payload[0], XPFW_IPI_MAX_MSG_LEN);
			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_ERROR, "IPI payload read error\r\n");
				goto Done;
			}
			/* Dispatch based on IPI ID (MSB 16 bits of Word-0) of the module */
			for (Idx = 0U; Idx < CorePtr->ModCount; Idx++) {
				/* If API ID matches and IpiHandler is set */
				if ( (CorePtr->ModList[Idx].IpiId == (Payload[0] >> 16U)) &&
					(CorePtr->ModList[Idx].IpiHandler != NULL)) {
					/* Call the module's IPI handler */
					CorePtr->ModList[Idx].IpiHandler(&CorePtr->ModList[Idx],
							IpiNum, IpiMaskList[MaskIndex],
							&Payload[0], XPFW_IPI_MAX_MSG_LEN);
					CallCount++;
				}
			}
		}
	}

	if (CallCount > 0U) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

Done:
	return Status;
}


const XPfw_Module_t *XPfw_CoreCreateMod(void)
{
	const XPfw_Module_t *ModPtr;

	if (CorePtr != NULL) {
		if (CorePtr->ModCount < XPFW_MAX_MOD_COUNT) {
			CorePtr->ModList[CorePtr->ModCount].ModId = CorePtr->ModCount;
			ModPtr = &CorePtr->ModList[CorePtr->ModCount];
			CorePtr->ModCount++;
		} else {
			ModPtr = NULL;
		}
	} else {
		ModPtr = NULL;
	}

	return ModPtr;
}

XStatus XPfw_CoreScheduleTask(const XPfw_Module_t *ModPtr, u32 Interval,
		VoidFunction_t CallbackRef)
{
	XStatus Status;

	if ((ModPtr != NULL) && (CorePtr != NULL)) {
		Status = XPfw_SchedulerAddTask(&CorePtr->Scheduler, ModPtr->ModId,
				Interval, CallbackRef);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

s32 XPfw_CoreRemoveTask(const XPfw_Module_t *ModPtr, u32 Interval,
		VoidFunction_t CallbackRef)
{
	s32 Status;

	if ((ModPtr == NULL) || (CorePtr == NULL)) {
		Status = XST_FAILURE;
		goto Done;
	}
	Status = XPfw_SchedulerRemoveTask(&CorePtr->Scheduler, ModPtr->ModId,
			Interval, CallbackRef);
Done:
	return Status;
}

void XPfw_CoreTickHandler(void)
{
	if(CorePtr != NULL){
		XPfw_SchedulerTickHandler(&CorePtr->Scheduler);
	}
	else {
		XPfw_Printf(DEBUG_ERROR,"ERROR: NULL pointer to Core\r\n");
	}
}

XStatus XPfw_CoreIsReady(void)
{
	XStatus Status;

	if (CorePtr != NULL) {
		if (CORE_IS_READY == CorePtr->IsReady) {
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPfw_CoreLoop(void)
{

	if(CORE_IS_READY == CorePtr->IsReady)
	{

		#ifdef ENABLE_SCHEDULER
		if(XPfw_SchedulerStart(&CorePtr->Scheduler) != XST_SUCCESS) {
			XPfw_Printf(DEBUG_DETAILED,"Warning: Scheduler has failed to"
					"Start\r\n");
		} else {
			XPfw_InterruptEnable(PMU_IOMODULE_IRQ_ENABLE_PIT1_MASK);
		}
		#endif
		do {

		#ifdef SLEEP_WHEN_IDLE
				/*Sleep. Will be waken up when a interrupt occurs*/
				mb_sleep();
		#endif

		#ifdef ENABLE_SCHEDULER
			if(TRUE == CorePtr->Scheduler.Enabled){
				(void)XPfw_SchedulerProcess(&CorePtr->Scheduler);
			}
		#endif

		} while (1);

	}
	/* If we reach this point then there was an error */
	return (XStatus)XST_FAILURE;
}


void XPfw_CorePrintStats(void)
{
	if(CorePtr != NULL) {
	XPfw_Printf(DEBUG_DETAILED,
			"######################################################\r\n");
	XPfw_Printf(DEBUG_DETAILED,"Module Count: %d (%d)\r\n", CorePtr->ModCount,
			XPFW_MAX_MOD_COUNT);
	XPfw_Printf(DEBUG_DETAILED,"Scheduler State: %s\r\n",
			((CorePtr->Scheduler.Enabled == TRUE)?"ENABLED":"DISABLED"));
	XPfw_Printf(DEBUG_DETAILED,"Scheduler Ticks: %lu\r\n",
			CorePtr->Scheduler.Tick);
	XPfw_Printf(DEBUG_DETAILED,
			"######################################################\r\n");
	}
}


XStatus XPfw_CoreRegisterEvent(const XPfw_Module_t *ModPtr, u32 EventId)
{
	XStatus Status;

	if (NULL == ModPtr) {
		Status = XST_FAILURE;
	} else {
		Status = XPfw_EventAddOwner(ModPtr->ModId, EventId);
	}
	return Status;
}

XStatus XPfw_CoreDeRegisterEvent(const XPfw_Module_t *ModPtr, u32 EventId)
{
	XStatus Status;

	if (NULL == ModPtr) {
		Status = XST_FAILURE;
	} else {
		Status = XPfw_EventRemoveOwner(ModPtr->ModId, EventId);
	}
	return Status;
}

XStatus XPfw_CoreStopScheduler(void)
{
	XStatus Status;
	if(CorePtr != NULL) {
		Status = XPfw_SchedulerStop(&CorePtr->Scheduler);
		XPfw_InterruptDisable(PMU_IOMODULE_IRQ_ENABLE_PIT1_MASK);
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPfw_CoreSetCfgHandler(const XPfw_Module_t *ModPtr, XPfwModCfgInitHandler_t CfgHandler)
{
	XStatus Status;
	if ((ModPtr != NULL) && (CorePtr != NULL)) {
		if (ModPtr->ModId < CorePtr->ModCount) {
			CorePtr->ModList[ModPtr->ModId].CfgInitHandler = CfgHandler;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}

XStatus XPfw_CoreSetEventHandler(const XPfw_Module_t *ModPtr, XPfwModEventHandler_t EventHandlerFn)
{
	XStatus Status;
	if ((ModPtr != NULL) && (CorePtr != NULL)) {
		if (ModPtr->ModId < CorePtr->ModCount) {
			CorePtr->ModList[ModPtr->ModId].EventHandler = EventHandlerFn;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}


XStatus XPfw_CoreSetIpiHandler(const XPfw_Module_t *ModPtr, XPfwModIpiHandler_t IpiHandlerFn, u16 IpiId)
{
	XStatus Status;
	if ((ModPtr != NULL) && (CorePtr != NULL)) {
		if (ModPtr->ModId < CorePtr->ModCount) {
			CorePtr->ModList[ModPtr->ModId].IpiHandler = IpiHandlerFn;
			CorePtr->ModList[ModPtr->ModId].IpiId = IpiId;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
	} else {
		Status = XST_FAILURE;
	}

	return Status;
}
