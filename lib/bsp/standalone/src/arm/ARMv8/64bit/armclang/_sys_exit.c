#include "xil_types.h"

/* Stub for exit() sys-call */
__attribute__((weak)) void _sys_exit(__attribute__((unused)) s32 rc)
{
	while(1) {
		;
	}
}
