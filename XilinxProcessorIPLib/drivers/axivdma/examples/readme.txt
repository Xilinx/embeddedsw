/** \page example Examples
You can refer to the below stated example applications for more details on how to use axivdma driver.

@section ex1 xaxivdma_example_selftest.c
Contains an example on how to use the XAxivdma driver directly.
This example performs the basic selftest using the driver.

For details, see xaxivdma_example_selftest.c.

@section ex2 xaxivdma_example_intr.c
Contains an example on how to use the XAxivdma driver directly.
This example shows the usage of AXI Video DMA with other video IPs
to do video frame transfers. This example does not work by itself.
It needs two other Video IPs, one for writing video frames to the
memory and one for reading video frames from the memory.

For details, see xaxivdma_example_intr.c.

@section ex3 axivdma triple buffer example
Contains an example on how to use the XAxivdma triple buffer API.
This example contains 2 files.
		* vdma_api.c - This file has high level API's to configure and start the VDMA transfer.
		* vdma.c - This file comprises sample application to the usage of VDMA API's in vdma_api.c.

For details, see vdma.c.
For details, see vdma_api.c.

NOTE:
* These examples assumes that the design has VDMA with both MM2S and S2MM path enable.
*/
