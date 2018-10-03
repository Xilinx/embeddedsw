#include "xil_types.h"
/* Stuv for close() sys-call */
__weak s32 _sys_close(s32 fh)
{
   return -1;
}
