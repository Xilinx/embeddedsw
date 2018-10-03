#include "xil_types.h"
/* Stuv for exit() sys-call */
__weak void _sys_exit(s32 rc)
{
   while(1) {
		;
	}
}
