/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "pm_callbacks.h"

/** @cond xilpm_internal */
#define NODE_CLASS_SHIFT        26U
#define NODE_CLASS_MASK_BITS    0x3FU
#define NODE_CLASS_MASK         ((u32)NODE_CLASS_MASK_BITS << NODE_CLASS_SHIFT)
#define NODECLASS(ID)           (((ID) & NODE_CLASS_MASK) >> NODE_CLASS_SHIFT)
#define XPM_NODECLASS_EVENT     10

static XPm_Notifier* NotifierList = NULL;

/****************************************************************************/
/**
 * @brief  Add notifier into the list
 *
 * @param  Notifier Pointer to notifier object which needs to be added
 *  in the list
 *
 * @return Returns XST_SUCCESS if notifier is added /
 *  XST_INVALID_PARAM if given notifier argument is NULL
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_NotifierAdd(XPm_Notifier* const Notifier)
{
	XStatus Status = (s32)XST_FAILURE;
	const XPm_Notifier* Curr;

	if (NULL == Notifier) {
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	/* Prevention of adding same Notifier in NotifierList */
	Curr = NotifierList;
	while (NULL != Curr) {
		if (Notifier == Curr) {
			break;
		}
		Curr = Curr->next;
	}

	if (NULL == Curr) {
		/* New notifiers are added at the front of list */
		Notifier->received = 0U;
		Notifier->next = NotifierList;
		NotifierList = Notifier;
	}

	Status = (s32)XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Remove notifier from the list
 *
 * @param  Notifier Pointer to notifier object to be removed from list
 *
 * @return Returns XST_SUCCESS if notifier is removed /
 *  XST_INVALID_PARAM if given notifier pointer is NULL /
 *  XST_FAILURE if notifier is not found
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_NotifierRemove(XPm_Notifier* const Notifier)
{
	XStatus Status = (s32)XST_FAILURE;
	XPm_Notifier* Curr;
	XPm_Notifier* Prev = NULL;

	if (NULL == Notifier) {
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Curr = NotifierList;
	while (NULL != Curr) {
		if (Notifier == Curr) {
			if (NULL != Prev) {
				Prev->next = Curr->next;
			} else {
				NotifierList = Curr->next;
			}

			Status = (s32)XST_SUCCESS;
			break;
		}
		Prev = Curr;
		Curr = Curr->next;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Call to process notification event
 *
 * @param  Node     Device which is the subject of notification
 * @param  Event    Event which is the subject of notification
 * @param  Oppoint  Operating point of the device in question
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void XPm_NotifierProcessEvent(const u32 Node, const u32 Event,
			      const u32 Oppoint)
{
	XPm_Notifier* Notifier = NULL;
	XStatus Status = (s32)XST_FAILURE;
	u32 ApiVersionServer = XPm_GetRegisterNotifierVersionServer();

	/* Validate the notifier list */
	if (NULL != NotifierList) {
		Notifier = NotifierList;
	}

	while (NULL != Notifier) {
		if ((Node == Notifier->node) &&
		    (((((u32)XST_API_REG_NOTIFIER_VERSION <= ApiVersionServer) && /* For new version of XilPM Server */
		      ((((u32)XPM_NODECLASS_EVENT == NODECLASS(Node)) && (0U != (Event & Notifier->event))) ||
		       (((u32)XPM_NODECLASS_EVENT != NODECLASS(Node)) && (Event == Notifier->event))))) ||
		     (((u32)XST_API_BASE_VERSION == ApiVersionServer) && /* For old version of XilPM Server */
		      (Event == Notifier->event)))) {
			Notifier->oppoint = Oppoint;
			Notifier->received++;
			Notifier->received_event = Notifier->event & Event;

			if (NULL != Notifier->callback) {
				Notifier->callback(Notifier);

				if ((u32)XPM_NODECLASS_EVENT == NODECLASS(Node)) {
					Status = XPm_RegisterNotifier(Notifier);
				        if ((s32)XST_SUCCESS != Status) {
						xil_printf("Re-registration failed Status = %d\n", Status);
					}
				}
			}
			/* There could be multiple pairs with different notifiers */
		}
		Notifier = Notifier->next;
	}
}
/** @endcond */
