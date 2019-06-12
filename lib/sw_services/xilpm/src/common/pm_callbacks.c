/******************************************************************************
*
* Copyright (C) 2015-2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
	XStatus status;

	if (NULL == notifier) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	notifier->received = 0U;

	/* New notifiers are added at the front of list */
	notifier->next = notifierList;
	notifierList = notifier;

	status = XST_SUCCESS;

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
	XStatus status = XST_FAILURE;
	XPm_Notifier* curr;
	XPm_Notifier* prev = NULL;

	if (NULL == notifier) {
		status = XST_INVALID_PARAM;
		goto done;
	}

	curr = notifierList;
	while (curr != NULL) {
		if (notifier == curr) {
<<<<<<< HEAD
<<<<<<< HEAD
			if (prev != NULL) {
=======
			if (prev != NULL)
>>>>>>> Xilpm: Fixed MISRA-C:2012 R.10.1, R.14.4 in pm_callback.c
=======
			if (prev != NULL) {
>>>>>>> Xilpm: Fixed MISRA-C:2012 R.15.6 in pm_callback.c
				prev->next = curr->next;
			}
			else {
				notifierList = curr->next;
			}

			status = XST_SUCCESS;
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
<<<<<<< HEAD
<<<<<<< HEAD
	while (notifier != NULL) {
=======
	while (notifier) {
>>>>>>> Xilpm: Fixed MISRA-C:2012 D.4.1 in pm_callback.c
=======
	while (notifier != NULL) {
>>>>>>> Xilpm: Fixed MISRA-C:2012 R.10.1, R.14.4 in pm_callback.c
		if ((node == notifier->node) &&
		    (event == notifier->event)) {
			notifier->oppoint = oppoint;
			notifier->received++;
<<<<<<< HEAD
<<<<<<< HEAD
			if (notifier->callback != NULL) {
=======
			if (notifier->callback != NULL)
>>>>>>> Xilpm: Fixed MISRA-C:2012 R.10.1, R.14.4 in pm_callback.c
=======
			if (notifier->callback != NULL) {
>>>>>>> Xilpm: Fixed MISRA-C:2012 R.15.6 in pm_callback.c
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
 /** @} */
