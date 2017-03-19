/** \page example Examples
You can refer to the below stated example applications for more details which gives an idea of
	- How the EmacPs and its driver are intended to be used.
	- How the protocol packets flow and processed for IEEE1588 (PTP).

SYSTEM REQUIREMENTS
The system containing the EmacPs should have the following capabilities:

  - An interrupt controller
  - A UART to display messages
  - A Timer that is used to send protocol packets at regular intervals

For testing the example in xemacps_example_intr_dma.c which uses PHY loopback,
a single hardware system is sufficient.

To run and test the binary created for IEEE1588  (PTP), the user
needs to have two EmacPs based hardware systems connected back-to-back.
One would act as a Master and other as a slave.

The user must include the three files(xemacps_example.h, xemacps_example_util.c,
xemacps_example_intr_dma.c) for building the binary to see how Ethernet packets are sent and
received in PHY loopback mode.

The user must include the three files(xemacps_ieee1588_example.c, xemacps_ieee1588.h,
xemacps_ieee1588.c) for building the binary to see how IEEE1588 (PTP) works.

@section ex1 xemacps_example.h
This file demonstrate the usage of EmacPs. This headerfile defines
common data types, prototypes, and includes the proper headers for
use with the Emacps examples.

For details, see xemacps_example.h.

@section ex2 xemacps_example_intr_dma.c
Contains an example on how to use the XEmacps driver directly.
This example uses the EmacPs's interrupt driven DMA packet transfer
mode to send and receive frames.

For details, see xemacps_example_intr_dma.c.

@section ex3 xemacps_example_util.c
Contains an example on how to use the XEmacps driver directly.
This example implements the utility functions for debugging, and
 ethernet frame construction.

For details, see xemacps_example_util.c.

@section ex4 xemacps_ieee1588.c
Contains an example on how to demonstrate the IEEE1588 protocol.
This example is the C file that has the programe entry "main".
It also has initialization code for EmacPs, Timer etc and has initialization
code for PTP packets.

For details, see xemacps_ieee1588.c.

@section ex5 xemacps_ieee1588.h
Contains an example on how to demonstrate the IEEE1588 protocol.
This headerfile defines hash defines, common data types and prototypes
to be used with the PTP standalone examples source code.

For details, see xemacps_ieee1588.h.

@section ex6 xemacps_ieee1588_example.c
Contains an example on how to demonstrate the IEEE1588 protocol.
This example processes the protocol packets.

For details, see xemacps_ieee1588_example.c.

@subsection HOW THE IEEE1588 EXAMPLE WORKS

 - The example should be run between two boards, both having capability to
   time stamp the PTP packets. For example, it can be run between two Zynq
   boards, e.g. zc706 0r zc702. Additionally this example is also tested
   between a Zynq board and a ML605 board.
   However, on ML605 board we need to run a slightly modified AVB example
   (for AxiEthernet) available in Perforce for AxiEthernet driver.
 - The example can be run in PTP MASTER mode or PTP SLAVE mode. To run it in
   PTP MASTER mode please define the flag IEEE1588_MASTER
   (with -DIEEE1588_MASTER) in the compiler options or just define it in
   xemacps_ieee1588.h. If this flag is defined then the software runs in
   MASTER mode or else in SLAVE mode.
 - The example by default initializes the PHY and GEM for 100 Mbps speed.
   Additionally autonegotiation can be used. To use autonegotiation, please
   define the flag PHY_AUTONEGOTIATION. However this feature is applicable
   only for Zynq boards.
 - ScuTimer is used to generate interrupts every 500 mseconds. This timer
   interrupt is used to send various protocol packets. For example, Sync/
   FollowUp packets are sent every 1 seconds. Announce frames are sent every
   5 seconds and PDelay packets are sent every 4 seconds.
 - In a typical test setup, 2 Zynq boards are connected back to back using a
   crossed cable. Please build the SDK projects (with the above 3 files)
   in MASTER mode and download the elf to one Zynq board. Then build the
   SDK project in SLAVE mode and download and run it in the other board.
   You can start seeing the console messages.
 - There are two debug levels. By default debug level 1 is defined to see
   some of the important PTP messages. For example, the link delay and the
   time correction values. The other debug level can be defined to see the
   messages from ISR. It can be handy to debug issues if any. To define the
   1st debug level, define the flag DEBUG_XEMACPS_LEVEL1 in
   xemacps_ieee1588.h. To define the other debug level, define the flag
   DEBUG_LEVEL_TWO.
 - Signalling frames are not implemented. So at runtime, we cannot change
   mode between MASTER and SLAVE. Clock adjustment is implemented, but
   clock rate adjustment is not implemented.
*/