#include "xparameters.h"

int $Sub$$_sys_write(unsigned int fh, const unsigned char *buf, unsigned int len, int mode)
{
#ifdef STDOUT_BASEADDRESS
  unsigned int volatile *uart_base = (unsigned int *)STDOUT_BASEADDRESS;
  int i;

  for (i =0; i < len;i++) {
    /* wait if TNFUL */
    while (*(uart_base + 11) & (1 << 14)) ;
    *(uart_base + 12) = buf[i];
  }
#endif
  return 0;
}
