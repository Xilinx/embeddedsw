
// Stub for read() sys-call
int $Sub$$_sys_read(unsigned int fh, unsigned char *buf, unsigned int len, int mode)
{
   // Return the number of character NOT read
   return len;
}
