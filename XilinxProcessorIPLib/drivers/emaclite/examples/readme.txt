/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea of
how the EmacLite and its driver/examples are intended to be used.

SYSTEM REQUIREMENTS:
The system containing the EmacLite should have the following capabilities:

  - Processor based system
  - At least one EmacLite core
  - An interrupt controller
  - An external memory controller with at least 200KB of RAM available
  - A UART to display messages

@section ex1 xemaclite_example.h
This headerfile defines common data types, prototypes, and includes the proper
headers for use with the EmacLite examples.
This file needs to be used with xemaclite_phy_loopback_example,
xemaclite_polled_example, xemaclite_intr_example and
xemaclite_internal_loopback_example.c

For details, see xemaclite_example.h.

@section ex2 xemaclite_example_util.c
Contains an example on how to use the XEmaclite driver directly.
This example implements the utility functions for the EmacLite example code.
This file needs to be included in xemaclite_phy_loopback_example,
xemaclite_polled_example, xemaclite_intr_example and
xemaclite_internal_loopback_example.c

For details, see xemaclite_example_util.c.

@section ex3 xemaclite_internal_loopback_example.c
Contains an example on how to use the XEmaclite driver directly.
This is an interrupt example outlining the use of interrupts and callbacks
in the transmission/reception of Ethernet frames using internal loop back
with an incrementing payload from 1 byte to 1500 bytes (excluding Ethernet
Header and FCS).

For details, see xemaclite_internal_loopback_example.c.

@section ex4 xemaclite_intr_example.c
Contains an example on how to use the XEmaclite driver directly.
This is an interrupt example outlining the use of interrupts and callbacks
in the transmission/reception of an Ethernet frame of 1000 bytes of payload.

For details, see xemaclite_intr_example.c.

@section ex5 xemaclite_phy_loopback_example.c
Contains an example on how to use the XEmaclite driver directly.
This is an interrupt example outlining the use of interrupts and callbacks
in the transmission/reception of Ethernet frames using MAC loop back in
the PHY device with an incrementing payload from 1 byte to 1500 bytes
(excluding Ethernet Header and FCS).
This example can be run only when the MDIO interface is configured in the
EmacLite core.

For details, see xemaclite_phy_loopback_example.c.

@section ex6 xemaclite_ping_reply_example.c
Contains an example on how to use the XEmaclite driver directly.
This is an EmacLite ping reply example in polled mode. This example will
generate a ping reply when it receives a ping request packet from the
external world.

For details, see xemaclite_ping_reply_example.c.

@section ex7 xemaclite_ping_req_example.c
Contains an example on how to use the XEmaclite driver directly.
This is an EmacLite Ping request example in polled mode. This
example will generate a ping request for the specified IP address.

For details, see xemaclite_ping_req_example.c.

@section ex8 xemaclite_polled_example.c
Contains an example on how to use the XEmaclite driver directly.
This is an polled mode example outlining the transmission/reception
of an Ethernet frame of 1000 bytes of payload.

For details, see xemaclite_polled_example.c.

@section ex9 xemaclite_selftest_example.c
Contains an example on how to use the XEmaclite driver directly.
This example performs the basic selftest using the emaclite driver.

For details, see xemaclite_selftest_example.c.

@subsection INCLUDING EXAMPLES IN EDK/SDK:
 - Each example is independent from the others except for common code found in
   xemaclite_example_util.c. When including source code files in an EDK/SDK SW
   application, select xemaclite_example_util.c along with one other example
   source code file.
*/
