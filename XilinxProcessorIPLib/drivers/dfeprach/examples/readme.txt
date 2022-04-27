/** \page example Examples
You can refer to the below stated example applications for more details on how to use the driver.

@section ex1 xdfeprach_examples.c
Examples are:
1. Self test on one instance:
    - initialisation from reset to activation
    - close the driver

1. Self test on one instance:
    - initialisation from reset to activation
    - close the driver

2. Set 2 CC and 3 RC example:
    - Create and system initialize the device driver instance.
    - Read SW and HW version numbers.
    - Reset the device.
    - Configure the device.
    - Initialize the device.
    - Activate the device.
    - Get current Component Carrier (CC) configuration.
    - Get current RACH Channel (RC) configuration.
    - add 2 Component Carriers: CCID={5,3}
    - add 3 RACH Channel: RCID={2,4,6}
    - Trigger configuration update.
    - Deactivate the device.

3. Set 2 CC and 3 RC and reconfigure example:
    - Create and system initialize the device driver instance.
    - Read SW and HW version numbers.
    - Reset the device.
    - Configure the device.
    - Initialize the device.
    - Activate the device.
    - Get current Component Carrier (CC) configuration.
    - Get current RACH Channel (RC) configuration.
    - add 2 Component Carriers: CCID={5,3}
    - add 3 RACH Channel: RCID={2,4,6}
    - remove one RACH Channel
    - configure and add new RACH channel
    - Trigger configuration update.
    - Deactivate the device.

4. Set 2 CC and 3 RC dynamically example:
    - Create and system initialize the device driver instance.
    - Read SW and HW version numbers.
    - Reset the device.
    - Configure the device.
    - Initialize the device.
    - Activate the device.
    - Get current Component Carrier (CC) configuration.
    - Get current RACH Channel (RC) configuration.
    - add 2 Component Carriers: CCID={5,3}
    - add 3 RACH Channel: RCID={2,4,6}
    - Trigger configuration update.
    - Deactivate the device.

The example code is written and tested for zcu670 board.
Running example in a Bare metal environment includes setting an output clock on si570.
For details, see xdfeprach examples code.
*/
