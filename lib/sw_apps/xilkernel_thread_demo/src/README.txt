Xilkernel Thread Demo
---------------------

This demo provides a simple example of how to create multiple POSIX threads and synchronize with them when they are complete. This example creates an initial master thread. The master thread creates 4 worker threads that go off compute parts of a sum and return the partial sum as the result. The master thread accumulates the partial sums and prints the result. This example can serve as your starting point for your end application thread structure.

References
----------

1. The complete Xilkernel API can be found in the OS and Libraries Reference
   Manual, shipped with your Xilinx Software Installation.


