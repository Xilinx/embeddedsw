#! usr/bin/bash
slr=$1
if [ $slr == 1 ]
then
	echo "Taking type0_1.ebd_cfi as input"
	mv ./type0_1.ebd_cfi ./mypdi.ebd_cfi #
elif [ $slr == 2 ]
then
	echo "Taking type0_2.ebd_cfi as input"
	mv ./type0_2.ebd_cfi ./mypdi.ebd_cfi #
elif [ $slr == 0 ]
then
	echo "Taking type0_0.ebd_cfi as input"
	mv ./type0_0.ebd_cfi ./mypdi.ebd_cfi #
else
	echo "Provide Valid SLR ID"
fi

gcc "./xsem_decode_ebdcfi.c" #
sleep 5 #
ls #
./a.out #
sleep 5 #
make CFLAGS="-g -DXILSEM_ENABLE_SSIT" #
chmod u+x xsem_reformat_ebdcfi #
./xsem_reformat_ebdcfi "$slr" #

if [ $slr == 1 ]
then
	mv ./xsem_ebdintern.c ./xsem_ebdintern_slr1.c #
	mv ./mypdi.ebd_cfi ./type0_1.ebd_cfi #
elif [ $slr == 2 ]
then
	mv ./xsem_ebdintern.c ./xsem_ebdintern_slr2.c #
	mv ./mypdi.ebd_cfi ./type0_2.ebd_cfi #
elif [ $slr == 0 ]
then
	mv ./xsem_ebdintern.c ./xsem_ebdintern_slr0.c #
	mv ./mypdi.ebd_cfi ./type0_0.ebd_cfi #
else
	echo "Script done with Error SLR ID"
fi
