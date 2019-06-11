#include "xil_types.h"

/* Stub for close() sys-call */
__attribute__((weak)) s32 _sys_close(__attribute__((unused)) s32 fh)
{
	return -1;
}
