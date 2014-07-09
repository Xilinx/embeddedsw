
// Stub for read() sys-call
__weak int _sys_read(unsigned int fh, unsigned char *buf, unsigned int len, int mode)
{
   // Return the number of character NOT read
   return len;
}
