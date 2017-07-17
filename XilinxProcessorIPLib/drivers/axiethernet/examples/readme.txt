/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea of
how the Axi Ethernet and its driver are intended to be used.

SYSTEM REQUIREMENTS:
The system containing the Axi Ethernet should have the following capabilities:
  - An interrupt controller
  - An external memory controller with at least 200KB of RAM available
  - A UART to display messages
  - The reset line of the device connected to the AxiEthernet AXI4-Stream
    interface (AXIDMA or AXIFIFO) must be connected to the reset line of AxiEthernet.
    (By default for BSB generated systems this is the case.)

@section ex1 xaxiethernet_example_extmulticast.c
Contains an example on how to use the XAxietherent driver directly.
This example uses the AXI Ethernet's interrupt driven DMA packet transfer mode
to send and receive multicast frames when XAE_EXT_MULTICAST_OPTION is enabled.

For details, see xaxiethernet_example_extmulticast.c.

@section ex2 xaxiethernet_example_extvlan.c
Contains an example on how to use the XAxietherent driver directly.
This example uses the Axi Ethernet's interrupt driven SGDMA packet transfer mode
to send and receive frames.

For details, see xaxiethernet_example_extvlan.c.

@section ex3 xaxiethernet_example_intr_fifo.c
Contains an example on how to use the XAxietherent driver directly.
This example uses the Axi Ethernet's interrupt driven FIFO direct packet transfer
mode to send and receive frames. HW must be setup for FIFO direct mode.

For details, see xaxiethernet_example_intr_fifo.c.

@section ex4 xaxiethernet_example_intr_sgdma.c
Contains an example on how to use the XAxietherent driver directly.
This example uses the Axi Ethernet's interrupt driven SGDMA packet transfer
mode to send and receive frames. HW must be setup for checksum offloading
for this example to properly execute.

For details, see xaxiethernet_example_intr_sgdma.c.

@section ex5 xaxiethernet_example_polled.c
Contains an example on how to use the XAxietherent driver directly.
This example uses the Axi Ethernet's FIFO direct frame transfer mode in a
polled fashion to send and receive frames. HW must be setup for FIFO direct
mode.

For details, see xaxiethernet_example_polled.c.

@section ex6 xaxiethernet_example_util.c
Contains an example on how to use the XAxietherent driver directly.
This example implements the utility functions for debugging, and ethernet
frame construction.

For details, see xaxiethernet_example_util.c.

@section ex7 xaxiethernet_example_mcdma_poll.c
Contains an example on how to use the XAxietherent driver directly.
This example shows how to use Axi Ethernet with MCDMA in polled mode
to send and receive frames.

For details, see xaxiethernet_example_mcdma_poll.c.

@section ex8 xaxiethernet_example_intr_mcdma.c
Contains an example on how to use the XAxietherent driver directly.
This example shows how to use Axi Ethernet with MCDMA in interrupt mode
to send and receive frames.

For details, see xaxiethernet_example_intr_mcdma.c.

@section ex9 xaxiethernet_mcdma_ping_req_example.c
Contains an example on how to use the XAxietherent driver directly.
This example shows how to use Axi Ethernet with MCDMA in polled mode
to send ping request's.

For details, see xaxiethernet_mcdma_ping_req_example.c.

@section ex10 xaxiethernet_example.h
This headerfile defines common data types, prototypes, and includes the proper
headers for use with the Axi Ethernet examples.

For details, see xaxiethernet_example.h.

@subsection INCLUDING EXAMPLES IN EDK:
 - Each example is independent from the others except for common code found in
   xaxiethernet_example_util.c. When including source code files in an EDK SW
   application, select xaxiethernet_example_util.c along with one other example
   source code file.


@subsection IMPORTANT NOTES:
 - Included HW features are critical as to which examples will run properly.
 - The device connected to the AXI4-Stream interface (AXIFIFO or AXIDMA or AXI MCDMA)
   of the AxiEthernet must be initialized before AxiEthernet initialization.
   Since the reset line of AXIFIFO or AXIDMA or AXI MCDMA is connected to the reset line
   of AxiEthernet, AXIDMA/AXIFIFO/AXI MCDMA initialization would reset AxiEthernet.
   AxiEthernet hardware initialization routines in the AxiEthernet driver do
   not reset the AxiEthernet hardware.
*/