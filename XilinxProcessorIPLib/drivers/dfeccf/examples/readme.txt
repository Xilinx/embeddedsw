/** \page example Examples
You can refer to the below stated example applications for more details on how to use the driver.

@section ex1 xdfeccf_examples.c
Examples are:
1. Multi instances:
    - initialisation from reset to activation
    - initialise two instances of XDfeCcf driver.
    - close the driver
2. Initialise and set one instance of XDfeCcf driver in pass through mode.
    - initialisation from reset to activation
    - configure filter in pass true mode
    - close the driver
3. Configure channel filter to one NR100 carrier and three NR20
     - initialise driver from reset to activation
     - configure filters with the precalculated coeficients
     - add 4 carriers CCID={0,1,2,3}, 1xNR100(122.88Msps) and 3xNR20(30.72Msps)
     - at this point filters are set and readu to operate
     - close the driver

The example code is written and tested for zcu670 board.
Running example in a Bare metal environment includes setting an output clock on si570.
For details, see xdfeccf_selftest_example.c.
*/
