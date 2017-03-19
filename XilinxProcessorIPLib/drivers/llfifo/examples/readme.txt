/** \page example Examples
You can refer to the below stated example applications for more details on how to use llfifo driver.

@section ex1 xllfifo_interrupt_example.c
Contains an example on how to use the XLlfifo driver directly.
This example is the interrupt example for the FIFO it assumes that at the
h/w level FIFO is connected in loopback.In these we write known amount of
data to the FIFO and wait for interrupts and after compltely receiving the
data compares it with the data transmitted.

For details, see xllfifo_interrupt_example.c.

@section ex2 xllfifo_polling_example.c
Contains an example on how to use the XLlfifo driver directly.
This example is the polling example for the FIFO it assumes that at the
h/w level FIFO is connected in loopback.In these we write known amount of
data to the FIFO and Receive the data and compare with the data transmitted.

For details, see xllfifo_polling_example.c.
*/
