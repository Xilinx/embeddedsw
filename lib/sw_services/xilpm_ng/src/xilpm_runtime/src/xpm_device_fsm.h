#ifndef __XPM_DEVICE_FSM_H__
#define __XPM_DEVICE_FSM_H__
#include "xstatus.h"
#include "xpm_fsm.h"
#ifdef __cplusplus
extern "C" {
#endif
extern const XPm_Fsm XPmGenericDeviceFsm;
XStatus XPmDeviceFsm_Init(XPmRuntime_DeviceOps* const DevOps);
#ifdef __cplusplus
}
#endif
#endif /* __XPM_DEVICE_FSM_H__ */
