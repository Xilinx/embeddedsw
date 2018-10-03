/** \page example Examples
You can refer to the below stated example applications for more details on how to use qspipsu driver.

@section ex1 xqspipsu_generic_flash_interrupt_example.c
Contains an example on how to use the XQspipsu driver directly
with a serial Flash device greater than or equal to 128Mb.
This example writes to flash and reads it back in DMA mode.This
examples runs with GENFIFO Manual start. It runs in interrupt mode.
This example illustrates single, parallel and stacked modes.
The hardware which this example runs on, must have a serial Flash(Micron
N25Q or Spansion S25FL) for it to run.

For details, see xqspipsu_generic_flash_interrupt_example.c.

@section ex2 xqspipsu_generic_flash_lqspi_example.c
Contains an example on how to use the XQspipsu driver directly
with a serial Flash device greater than or equal to 128Mb.
The example writes to flash in GQSPI mode and reads it back in Linear
QSPI mode.This examples runs with GENFIFO Manual start. It runs in
interrupt mode.This example runs in single mode.
The hardware which this example runs on, must have a serial Flash(Micron
N25Q or Spansion S25FL) for it to run.

For details, see xqspipsu_generic_flash_lqspi_example.c.

@section ex3 xqspipsu_generic_flash_polled_example.c
Contains an example on how to use the XQspipsu driver directly
with a serial Flash device greater than or equal to 128Mb.
The example writes to flash and reads it back in DMA mode.
This examples runs with GENFIFO Manual start. It runs in polled mode.
This example illustrates single, parallel and stacked modes.
The hardware which this example runs on, must have a serial Flash(Micron
N25Q or Spansion S25FL) for it to run.

For details, see xqspipsu_generic_flash_polled_example.c.

@section ex4 xqspipsu_polldata_polltimeout_interrupt_example.c
Contains an example on how to use the XQspipsu driver directly
with a serial Flash device greater than or equal to 128Mb.
The example writes to flash and reads it back in DMA mode.
This examples runs with GENFIFO Manual start. It runs in interrupt mode.
This example illustrates single, parallel and stacked modes.
The hardware which this example runs on, must have a serial Flash(Micron
N25Q or Spansion S25FL) for it to run. This application will configure
GQSPI controller to send status command to know the flash status,
instead of sending status command from the application.

For details, see xqspipsu_polldata_polltimeout_interrupt_example.c.
*/
