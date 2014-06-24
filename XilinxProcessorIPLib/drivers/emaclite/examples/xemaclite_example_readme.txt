xemaclite_example_readme.txt
-------------------------

The examples in this directory are provided to give the user some idea of
how the EmacLite and its driver/examples are intended to be used.

SYSTEM REQUIREMENTS

The system containing the EmacLite should have the following capabilities:

  - Processor based system
  - At least one EmacLite core
  - An interrupt controller
  - An external memory controller with at least 200KB of RAM available
  - A UART to display messages

FILES

1. xemaclite_example.h - Top level include for all examples.
   This file needs to be included in xemaclite_phy_loopback_example,
   xemaclite_polled_example, xemaclite_intr_example and 
   xemaclite_internal_loopback_example.c

2. xemaclite_example_util.c - Provide various functions for Phy setup.
   This file needs to be used with xemaclite_phy_loopback_example,
   xemaclite_polled_example, xemaclite_intr_example and 
   xemaclite_internal_loopback_example.c

3. xemaclite_polled_example.c - Example using the emaclite driver
   in polled mode. 

4. xemaclite_intr_example.c - Example using the emaclite driver
   in interrupt mode.

5. xemaclite_phy_loopback_example.c - Example using the emaclite driver
   in interrupt mode using the MAC loop back in the PHY. This example can 
   be run only when the MDIO interface is configured in the EmacLite core.

6. xemaclite_ping_rq_example.c - This is a polled mode example generating a 
   ping request for a specified IP address.

7. xemaclite_ping_reply_example.c - This is a polled mode example generating a 
   ping reply when it receives a ping packet from the external world.

8. xemaclite_selftest_example.c - This is a example based on the self test.

9.xemaclite_internal_loopback_example.c - 
  This file contains an interrupt example outlining the use of interrupts and
  callbacks in the transmission/reception of Ethernet frames using internal
  loop back with an incrementing payload from 1 byte to 1500 bytes (excluding
  Ethernet Header and FCS).



INCLUDING EXAMPLES IN EDK/SDK

Each example is independent from the others except for common code found in 
xemaclite_example_util.c. When including source code files in an EDK/SDK SW
application, select xemaclite_example_util.c along with one other example
source code file.



