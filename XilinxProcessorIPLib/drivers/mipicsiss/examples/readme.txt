/** \page example Examples
You can refer to the below stated example applications for more details on how to use mipicsiss driver.

@section ex1 xcsiss_intr_example.c
Contains an example on how to use the XMipicsiss driver directly.
This example shows the usage of the driver in interrupt mode.
On receiving a frame received interrupt, it will print frames received count.
On receiving a short packet FIFO not empty interrupt, it will print the
contents of the short packet received. On receiving DPHY, protocol or Packet
level error, it will print the same. On receiving any type of error interrupt
the sub-system will reset.

For details, see xcsiss_intr_example.c.

@section ex2 xcsiss_selftest_example.c
Contains an example on how to use the XMipicsiss driver directly.
This example performs the basic selftest to test the sub-core functions.

For details, see xcsiss_selftest_example.c.
*/
