/** \page example Examples
You can refer to the below stated example applications for more details on how to use qspips driver.

@section ex1 xqspips_selftest_example.c
Contains an example on how to use the XQspips driver directly.
This example performs the basic selftest using the driver.

For details, see xqspips_selftest_example.c.

@section ex2 xqspips_dual_flash_lqspi_example.c
Contains an example on how to use the XQspips driver directly.
This example shows the usage of the QSPI driver in Linear QSPI mode,
with two serial FLASH devices on seperate buses. With two flash memories
on seperate buses, even numbered bits in data words are written to the
lower memory and odd numbered bits are written to the upper memory. This
example writes to the two flash memories in  QSPI mode and reads the data
back from the flash memories, in Linear QSPI mode.

For details, see xqspips_dual_flash_lqspi_example.c.

@section ex3 xqspips_dual_flash_stack_lqspi_example.c
Contains an example on how to use the XQspips driver directly.
This example shows the usage of the QSPI driver in Linear QSPI mode,
with two serial Flash devices in stacked mode. One flash is accessed at
a time on a common bus by using separate selects. This example writes to
the two flash memories in  QSPI mode and reads the data back from the
flash memories, in Linear QSPI mode.

For details, see xqspips_dual_flash_stack_lqspi_example.c.

@section ex4 xqspips_flash_intr_example.c
Contains an example on how to use the XQspips driver directly.
This example shows the usage of the  QSPI driver in interrupt mode with
a serial FLASH device. This examples performs some transfers in
Manual Chip Select and Start mode.

For details, see xqspips_flash_intr_example.c.

@section ex5 xqspips_flash_lqspi_example.c
Contains an example on how to use the XQspips driver directly.
This example shows the usage of the QSPI driver in Linear QSPI mode
with a serial FLASH device. The example writes to the flash in QSPI
mode and reads it back in Linear QSPI mode.

For details, see xqspips_flash_lqspi_example.c.

@section ex6 xqspips_flash_polled_example.c
Contains an example on how to use the XQspips driver directly.
This example shows the usage of the QSPI driver in polled mode with a
serial FLASH device.

For details, see xqspips_flash_polled_example.c.

@section ex7 xqspips_g128_flash_example.c
Contains an example on how to use the XQspips driver directly.
This example shows the usage of the QSPI driver with a serial Flash
device greater than 128Mb. The example writes to flash and reads
it back in I/O mode.

For details, see xqspips_g128_flash_example.c.
*/
