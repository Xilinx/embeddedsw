/** \page example Examples
You can refer to the below stated example applications for more details on how to use mbox driver.

@section ex1 xmbox_example.c
Contains an example on how to use the XMbox driver directly.
This example shows the usage of driver and will illustrate how the
mbox driver can be used to 1.Initialize the Mailbox core, 2.Pass
data between two processors.

For details, see xmbox_example.c.

@section ex2 xmbox_intr_example.c
Contains an example on how to use the XMbox driver directly.
This example assumes there are two processors availabile in the system
that are expected to inter-communicate and attempts to send a known
message through the mailbox from one processor to the other processor.
The message is received by the receiver and the test passes.

For details, see xmbox_intr_example.c.

@section ex3 xmbox_tapp_example.c
Contains an example on how to use the XMbox driver directly.
This example attempts to send a known message through the mailbox
from one processor to the other processor.The message is received by
the receiver and the test passes.
This example assumes there are two processors availabile in the system
that are expected to inter-communicate. If Mailbox is connected to only one
Processor then Data has to be sent from one port and should be received
from another port.

For details, see xmbox_tapp_example.c.
*/
