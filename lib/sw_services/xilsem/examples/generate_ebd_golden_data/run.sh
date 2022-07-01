#! usr/bin/bash
mv ./*.ebd_cfi ./mypdi.ebd_cfi #
gcc "./xsem_decode_ebdcfi.c" #
sleep 5 #
ls #
./a.out #
sleep 5 #
make #
./xsem_reformat_ebdcfi #