/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "pm_callbacks.h"

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

	if (NULL == Notifier) {
		Status = (s32)XST_INVALID_PARAM;
		goto done;
	}

	Notifier->received = 0U;

	/* New notifiers are added at the front of list */
	Notifier->next = NotifierList;
	NotifierList = Notifier;

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
void XPm_NotifierProcessEvent(const u32 Node, const enum XPmNotifyEvent Event,
			      const u32 Oppoint)
{
	XPm_Notifier* Notifier = NULL;

	/* Validate the notifier list */
	if (NULL != NotifierList) {
		Notifier = NotifierList;
	}

	while (NULL != Notifier) {
		if ((Node == Notifier->node) &&
		    (Event == Notifier->event)) {
			Notifier->oppoint = Oppoint;
			Notifier->received++;
			if (NULL != Notifier->callback) {
				Notifier->callback(Notifier);
			}
			/* There could be multiple pairs with different notifiers */
		}
		Notifier = Notifier->next;
	}
}
