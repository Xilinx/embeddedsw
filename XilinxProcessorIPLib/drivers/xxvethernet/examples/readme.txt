/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea of
how the Xxv Ethernet or USXGMII and its driver are intended to be used.

SYSTEM REQUIREMENTS:
The system containing the Axi Ethernet should have the following capabilities:
  - An interrupt controller
  - An external memory controller with at least 200KB of RAM available
  - A UART to display messages
  - XxvEthernet or USXGMII device connected to a target MCDMA device.

@section ex1 xxxvethernet_example_intr_mcdma.c
Contains an example on how to use the XXxvetherent driver directly.
This example uses the Xxv Ethernet's interrupt driven packet transfer
mode with MCDMA to send and receive frames.
This example can be used on XXVEthernet IP.
Use supporting files xxxvethernet_example_util.c and xxxvethernet_example.h.

For details, see xxxvethernet_example_intr_mcdma.c.

@section ex1 xxxvethernet_usxgmii_mcdma_intr_example.c
Contains an example on how to use the XXxvetherent(USXGMII) driver directly.
This example uses the USXGMII's interrupt driven packet transfer
mode with MCDMA to send and receive frames.
This example can be used on USXGMII IP.
Use supporting files xxxvethernet_example_util.c and xxxvethernet_example.h.

For details, see xxxvethernet_usxgmii_mcdma_intr_example.c.

@section ex2 xxxvethernet_example_util.c
Contains an example on how to use the XXxvetherent driver directly.
This example implements the utility functions for debugging, and ethernet
frame construction.

For details, see xxxvethernet_example_util.c.

@section ex3 xxxvethernet_example.h
This headerfile defines common data types, prototypes, and includes the proper
headers for use with the Xxv Ethernet examples.

For details, see xxxvethernet_example.h.

@subsection INCLUDING EXAMPLES IN SDK:
 - Include all the above three files in the application project
