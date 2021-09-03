/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file pm_callbacks.c
 *
 * @addtogroup xpm_apis XilPM APIs
 * @{
 *****************************************************************************/
#include <stdlib.h>
#include "pm_callbacks.h"
#include "pm_client.h"

/** @cond xilpm_internal */
static XPm_Notifier* notifierList = NULL;

/****************************************************************************/
/**
 * @brief  Add notifier into the list
 *
 * @param  notifier Pointer to notifier object which needs to be added
 *  in the list
 *
 * @return Returns XST_SUCCESS if notifier is added /
 *  XST_INVALID_PARAM if given notifier argument is NULL
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_NotifierAdd(XPm_Notifier* const notifier)
{
	XStatus status = (XStatus)XST_FAILURE;

	if (NULL == notifier) {
		status = (XStatus)XST_INVALID_PARAM;
		goto done;
	}

	notifier->received = 0U;

	/* New notifiers are added at the front of list */
	notifier->next = notifierList;
	notifierList = notifier;

	status = (XStatus)XST_SUCCESS;

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Remove notifier from the list
 *
 * @param  notifier Pointer to notifier object to be removed from list
 *
 * @return Returns XST_SUCCESS if notifier is removed /
 *  XST_INVALID_PARAM if given notifier pointer is NULL /
 *  XST_FAILURE if notifier is not found
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_NotifierRemove(XPm_Notifier* const notifier)
{
	XStatus status = (XStatus)XST_FAILURE;
	XPm_Notifier* curr;
	XPm_Notifier* prev = NULL;

	if (NULL == notifier) {
		status = (XStatus)XST_INVALID_PARAM;
		goto done;
	}

	curr = notifierList;
	while (curr != NULL) {
		if (notifier == curr) {
			if (prev != NULL) {
				prev->next = curr->next;
			}
			else {
				notifierList = curr->next;
			}

			status = (XStatus)XST_SUCCESS;
			break;
		}
		prev = curr;
		curr = curr->next;
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Call to process notification event
 *
 * @param  node    Node which is the subject of notification
 * @param  event   Event which is the subject of notification
 * @param  oppoint Operating point of the node in question
 *
 * @return None
 *
 * @note   None
 *
 ****************************************************************************/
void XPm_NotifierProcessEvent(const enum XPmNodeId node,
			      const enum XPmNotifyEvent event,
			      const u32 oppoint)
{
	XPm_Notifier* notifier;

	/* Validate the notifier list */
	if (NULL != notifierList) {
		notifier = notifierList;
	}
	else {
		notifier = NULL;
	}
	while (notifier != NULL) {
		if ((node == notifier->node) &&
		    (event == notifier->event)) {
			notifier->oppoint = oppoint;
			notifier->received++;
			if (notifier->callback != NULL) {
				notifier->callback(notifier);
			}
			/*
			 * Don't break here, there could be multiple pairs of
			 * (node, event) with different notifiers
			 */
		}
		notifier = notifier->next;
	}
}
/** @endcond */
 /** @} */
