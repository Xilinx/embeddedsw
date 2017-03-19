/** \page example Examples
You can refer to the below stated example applications for more details on how to use gpio driver.

@section ex1 xgpio_example.c
Contains an example on how to use the XGpio driver directly.
This example performs the basic test on the gpio driver.
It only uses channel 1 of a GPIO device and assumes that
the bit 0 of the GPIO is connected to the LED on the HW board.

For details, see xgpio_example.c.

@section ex2 xgpio_intr_tapp_example.c
Contains an example on how to use the XGpio driver directly.
This example shows the usage of the driver in interrupt mode.

For details, see xgpio_intr_tapp_example.c.

@section ex3 xgpio_low_level_example.c
Contains an example on how to use the XGpio driver directly.
This example shows the usage of the gpio low level driver and hardware
device. It only uses a channel 1 of a GPIO device.

For details, see xgpio_low_level_example.c.

@section ex4 xgpio_tapp_example.c
Contains an example on how to use the XGpio driver directly.
This example shows the usage of the axi gpio driver and also
assumes that there is a UART Device or STDIO Device in the
hardware system.

For details, see xgpio_tapp_example.c.
*/
