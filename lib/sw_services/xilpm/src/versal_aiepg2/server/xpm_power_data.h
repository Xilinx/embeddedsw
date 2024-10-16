/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/* Define the APU macro table entries */
#define X_APU_MACRO_ENTRIES(HandlerTable) \
HandlerTable(0, 0) \
HandlerTable(0, 1) \
HandlerTable(1, 0) \
HandlerTable(1, 1) \
HandlerTable(2, 0) \
HandlerTable(2, 1) \
HandlerTable(3, 0) \
HandlerTable(3, 1)

/* Define the RPU macro table entries */
#define X_RPU_MACRO_ENTRIES(HandlerTable) \
HandlerTable(A, 0) \
HandlerTable(A, 1) \
HandlerTable(B, 0) \
HandlerTable(B, 1) \
HandlerTable(C, 0) \
HandlerTable(C, 1) \
HandlerTable(D, 0) \
HandlerTable(D, 1) \
HandlerTable(E, 0) \
HandlerTable(E, 1)

/* Define the macro to generate the APU power up/down handler table entries */
#define X_APU_PWR_HANDLER(cluster, core) \
{PSXC_LPX_SLCR_REQ_PWRUP0_STATUS_APU##cluster##_CORE##core##_MASK, PSXC_LPX_SLCR_REQ_PWRDWN0_STATUS_APU##cluster##_CORE##core##_MASK, &Acpu##cluster##_Core##core##PwrCtrl},

/* Define the macro to generate the RPU power up/down handle table entries */
#define X_RPU_PWR_HANDLER(cluster, core) \
{PSXC_LPX_SLCR_REQ_PWRUP1_STATUS_RPU##cluster##_CORE##core##_MASK, PSXC_LPX_SLCR_REQ_PWRDWN1_STATUS_RPU##cluster##_CORE##core##_MASK, &Rpu##cluster##_Core##core##PwrCtrl},

/* Define the macro to generate the APU wakeup handler table entries */
#define X_APU_WAKEUP_HANDLER(cluster, core) \
{PM_DEV_ACPU_##cluster##_##core, PSXC_LPX_SLCR_WAKEUP0_IRQ_STATUS_APU##cluster##_CORE##core##_MASK, &Acpu##cluster##_Core##core##PwrCtrl},

/* Define the macro to generate the RPU wakeup handler table entries */
#define X_RPU_WAKEUP_HANDLER(cluster, core) \
{PM_DEV_RPU_##cluster##_##core, PSXC_LPX_SLCR_WAKEUP1_IRQ_STATUS_RPU##cluster##_CORE##core##_MASK, &Rpu##cluster##_Core##core##PwrCtrl},

/* Define the macro to generate the APU sleep handler table entries */
#define X_APU_SLEEP_HANDLER(cluster, core) \
{PM_DEV_ACPU_##cluster##_##core, PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS_APU##cluster##_CORE##core##_MASK, &Acpu##cluster##_Core##core##PwrCtrl},

/* Define the macro to generate the RPU sleep handler table entries */
#define X_RPU_SLEEP_HANDLER(cluster, core) \
{PM_DEV_RPU_##cluster##_##core, PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS_RPU##cluster##_CORE##core##_MASK, &Rpu##cluster##_Core##core##PwrCtrl},

/* Define the macro to create the APU power up/down handler table */
#define CREATE_TABLE_APU_PWRUPDOWNHANDLER(NumClusters, NumCores) \
static struct PwrHandlerTable_t ApuPwrUpDwnHandlerTable[] = { \
X_APU_MACRO_ENTRIES(X_APU_PWR_HANDLER) \
};

/* Define the macro to create the RPU power up/down handler table */
#define CREATE_TABLE_RPU_PWRUPDOWNHANDLER(NumClusters, NumCores) \
static struct PwrHandlerTable_t RpuPwrUpDwnHandlerTable[] = { \
X_RPU_MACRO_ENTRIES(X_RPU_PWR_HANDLER) \
};

/* Define the macro to create the APU wakeup handler table */
#define CREATE_TABLE_APU_WAKEUP_HANDLER(NumClusters, NumCores) \
static struct PwrCtlWakeupHandlerTable_t ApuWakeupHandlerTable[] = { \
X_APU_MACRO_ENTRIES(X_APU_WAKEUP_HANDLER) \
};

/* Define the macro to create the RPU wakeup handler table */
#define CREATE_TABLE_RPU_WAKEUP_HANDLER(NumClusters, NumCores) \
static struct PwrCtlWakeupHandlerTable_t RpuWakeupHandlerTable[] = { \
X_RPU_MACRO_ENTRIES(X_RPU_WAKEUP_HANDLER) \
};

/* Define the macro to create the APU sleep handler table */
#define CREATE_TABLE_APU_SLEEP_HANDLER(NumClusters, NumCores) \
static struct PwrCtlWakeupHandlerTable_t ApuSleepHandlerTable[] = { \
X_APU_MACRO_ENTRIES(X_APU_SLEEP_HANDLER) \
};

/* Define the macro to create the RPU sleep handler table */
#define CREATE_TABLE_RPU_SLEEP_HANDLER(NumClusters, NumCores) \
static struct PwrCtlWakeupHandlerTable_t RpuSleepHandlerTable[] = { \
X_RPU_MACRO_ENTRIES(X_RPU_SLEEP_HANDLER) \
};
