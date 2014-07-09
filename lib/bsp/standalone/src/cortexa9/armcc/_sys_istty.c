
// Stub for istty sys-call
__weak int _sys_istty(unsigned int* f)
{ 
   /* cannot read/write files */
   return 1;
}
