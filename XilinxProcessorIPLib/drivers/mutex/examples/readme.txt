/** \page example Examples
You can refer to the below stated example applications for more details on how to use mutex driver.

@section ex1 xmutex_tapp_example.c
Contains an example on how to use the XMutex driver directly.
This example attempts to lock the Mutex from the processor identified as 0
(XPAR_CPU_ID=0) to prevent the other processor from getting the lock.
Since the application is running on two seperate processors, the
initiator declares success when the Mutex locks the other processor
declares success when the Mutex is locked from its perspective. There is
no feedback to the initiator so a terminal is required for each processor
to verify that the test passed for both sides.

For details, see xmutex_tapp_example.c.
*/
