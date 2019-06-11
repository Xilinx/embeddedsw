#include "xil_types.h"

/* Stub for open sys-call */
__attribute__((weak)) s32 _sys_open(__attribute__((unused)) const char8* name,
					__attribute__((unused)) s32 openmode)
{
	return 0;
}
