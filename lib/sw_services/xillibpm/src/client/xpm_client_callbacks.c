/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xpm_client_callbacks.h"

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
	XStatus Status;

	if (NULL == Notifier) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Notifier->received = 0U;

	/* New notifiers are added at the front of list */
	Notifier->next = NotifierList;
	NotifierList = Notifier;

	Status = XST_SUCCESS;

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
	XStatus Status = XST_FAILURE;
	XPm_Notifier* Curr;
	XPm_Notifier* Prev = NULL;

	if (NULL == Notifier) {
		Status = XST_INVALID_PARAM;
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

			Status = XST_SUCCESS;
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
