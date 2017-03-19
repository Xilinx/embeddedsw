/** \page example Examples
You can refer to the below stated example applications for more details on how to use nandps driver.

@section ex1 xnandps_example.c
Contains an example on how to use the XNandps driver directly.
This example tests the block erase, block read and block write features.
The flash blocks are erased and written. The data is read back and
compared with the data written for correctness. The bad blocks are not
erased/programmed.

For details, see xnandps_example.c.

@section ex2 xnandps_cache_example.c
Contains an example on how to use the XNandps driver directly.
This example tests NAND page cache read and write commands. The page cache
commands are not supported by OnDie ECC flash since ECC is enabled by
default.

For details, see xnandps_cache_example.c.

@section ex3 xnandps_skip_example.c
Contains an example on how to use the XNandps driver directly.
This example tests the skip block method of erase/read/write operations.
The skip block method is useful while reading/writing images on to the flash.
The flash is erased and programming by considering the bad blocks. The data is
read back and compared with the data written for correctness.

For details, see xnandps_skip_example.c.
*/
