#include "xil_types.h"

/* Stub for istty sys-call */
__attribute__((weak)) s32 _sys_istty(__attribute__((unused)) u32* f)
{
	/* cannot read/write files */
	return 1;
}
