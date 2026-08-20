/*
 * APU bare-metal: receive LPX WWDT0 expiry notification via PLM IPI.
 *
 * Uses XPm_RegisterNotifier with an EVENT-class node over the VISP mailbox
 * IPI5 path used by the APU-side application in this driver tree.
 */

#ifndef WDT_NOTIFIER_H
#define WDT_NOTIFIER_H

#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus WdtNotifier_Init(void);
void WdtNotifier_Deinit(void);

/* Send the WDT test command to the RPU (bare-metal mailbox path). */
int WdtSendTest(void);

/* Print debug/validation counters (IPI IRQs, PM_NOTIFY_CB, WWDT0 expiries). */
void WdtNotifier_PrintStats(void);

#ifdef __cplusplus
}
#endif

#endif
