vdma_readme.txt
---------------

The examples in this directory are provided to give the user some idea of how the VDMA
and its driver are intended to be used.

Examples:

1. AXI VDMA SELFTEST EXAMPLE:

This example does basic reset of the VDMA core.

FILES:
     * xaxivdma_example_selftest.c - This file does the basic reset of vdma core.

2.AXI VDMA INTERRUPT EXAMPLE:

This example demonstrates how to use the AXI Video DMA with other video IPs to do video frame transfers.
This example does not work by itself. It needs two other Video IPs, one for writing video frames to the
memory and one for reading video frames from the memory.

FILES:
      * xaxivdma_example_intr.c  - This file runs interrupt example for data transfer.

3.AXI VDMA TRIPLE BUFFER EXAMPLE:

This VDMA example demonstrates how to use the VDMA triple buffer API.

FILES:
     * vdma_api.c - This file has high level API's to configure and start the VDMA transfer.
     * vdma.c - This file comprises sample application to the usage of VDMA API's in vdma_api.c.

NOTE:
* These examples assumes that the design has VDMA with both MM2S and S2MM path enable.
