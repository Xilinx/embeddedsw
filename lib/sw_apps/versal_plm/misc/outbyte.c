#include "xparameters.h"
#include "xuartpsv_hw.h"

#ifdef __cplusplus
extern "C" {
#endif
void outbyte(char c); 

#ifdef __cplusplus
}
#endif 

#ifndef VERSAL_PLM
void outbyte(char c)
{
	 XUartPsv_SendByte(STDOUT_BASEADDRESS, c);
}
#endif
