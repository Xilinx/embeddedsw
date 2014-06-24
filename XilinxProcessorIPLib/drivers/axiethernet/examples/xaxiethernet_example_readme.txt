xaxiethernet_example_readme.txt
-------------------------------

The examples in this directory are provided to give the user some idea of
how the Axi Ethernet and its driver are intended to be used.

SYSTEM REQUIREMENTS

The system containing the Axi Ethernet should have the following capabilities:

  - An interrupt controller
  - An external memory controller with at least 200KB of RAM available
  - A UART to display messages
  - The reset line of the device connected to the AxiEthernet AXI4-Stream
    interface (AXIDMA or AXIFIFO) must be connected to the reset line of AxiEthernet.
    (By default for BSB generated systems this is the case.)

FILES

1. xaxiethernet_example.h - Top level include for all examples.

2. xaxiethernet_example_util.c - Provide various utilities for debugging, and
   ethernet frame construction.

3. xaxiethernet_example_polled.c - Examples using the L1 API found in xaxiethernet.h
   in polled mode. HW must be setup for FIFO direct mode.

4. xaxiethernet_example_intr_fifo.c - Examples using the L1 API found in
   xaxiethernet.h in interrupt driven FIFO direct mode. HW must be setup for
   FIFO direct mode.

5. xaxiethernet_example_intr_sgdma.c - Examples using the L1 API found in
   xaxiethernet.h in interrupt driven scatter-gather DMA mode. HW must be setup
   for SGDMA mode. HW must be setup for checksum offloading for that
   specific example to properly execute.


INCLUDING EXAMPLES IN EDK

Each example is independent from the others except for common code found in
xaxiethernet_example_util.c. When including source code files in an EDK SW
application, select xaxiethernet_example_util.c along with one other example
source code file.


IMPORTANT NOTES

* Included HW features are critical as to which examples will run properly.
* The device connected to the AXI4-Stream interface (AXIFIFO or AXIDMA)
  of the AxiEthernet must be initialized before AxiEthernet initialization.
  Since the reset line of AXIFIFO or AXIDMA is connected to the reset line
  of AxiEthernet, AXIDMA/AXIFIFO initialization would reset AxiEthernet.
  AxiEthernet hardware initialization routines in the AxiEthernet driver do
  not reset the AxiEthernet hardware.

