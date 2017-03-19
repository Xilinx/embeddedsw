/** \page example Examples
You can refer to the below stated example applications for more details on how to use iic driver.

@section ex1 xiic_selftest_example.c
Contains an example on how to use the XIic driver directly.
This example performs the basic selftest using the driver.

For details, see xiic_selftest_example.c.

@section ex2 xiic_dynamic_eeprom_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a Interrupt mode design which shows the
usage of the Xilinx iic device and XIic driver to exercise the EEPROM
in Dynamic controller mode. The XIic driver uses the complete
FIFO functionality to transmit/receive data. This example
writes/reads from the lower 256 bytes of the IIC EEPROMS.

For details, see xiic_dynamic_eeprom_example.c.

@section ex3 xiic_eeprom_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a Interrupt mode design which shows the
usage of the Xilinx iic device and XIic driver to exercise the EEPROM.
The XIic driver uses the complete FIFO functionality to
transmit/receive data. This example writes/reads from the lower
256 bytes of the IIC EEPROMS.

For details, see xiic_eeprom_example.c.

@section ex4 xiic_low_level_dynamic_eeprom_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a polled mode design which shows the usage of
the Xilinx iic device in dynamic mode and low-level driver to exercise
the EEPROM.

For details, see xiic_low_level_dynamic_eeprom_example.c.

@section ex5 xiic_low_level_eeprom_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a polled mode design which shows the usage of
the Xilinx iic device and low-level driver to exercise the EEPROM.
This example writes/reads from the lower 256 bytes of the IIC EEPROMS.

For details, see xiic_low_level_eeprom_example.c.

@section ex6 xiic_low_level_tempsensor_example.c
Contains an example on how to use the XIic driver directly.
This example contains a polled mode design which shows the usage of
the Xilinx iic device and low-level driver to execise the temperature
sensor. This example only performs read operations (receive) from the
iic temperature sensor of the platform.

For details, see xiic_low_level_tempsensor_example.c.

@section ex7 xiic_multi_master_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a Interrupt mode design which shows the usage
of the Xilinx iic device and XIic driver to exercise the EEPROM in a multi
master mode. This example writes/reads from the lower 256 bytes of
the IIC EEPROMS.

For details, see xiic_multi_master_example.c.

@section ex8 xiic_repeated_start_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a interrupt mode design to demonstrate
the use of repeated start using the XIic driver.

For details, see xiic_repeated_start_example.c.

@section ex9 xiic_slave_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a Interrupt mode design which shows the usage of
the Xilinx iic device and XIic driver to exercise the slave functionality
of the iic device.

For details, see xiic_slave_example.c.

@section ex10 xiic_tempsensor_example.c
Contains an example on how to use the XIic driver directly.
This example contains an interrupt based design which shows the usage of the
Xilinx iic device and driver to exercise the temperature sensor.
This example only performs read operations(receive) from the iic temperature
sensor of the platform.

For details, see xiic_tempsensor_example.c.

@section ex11 xiic_tenbitaddr_example.c
Contains an example on how to use the XIic driver directly.
This example consists of a Interrupt mode design which shows the usage of the Xilinx
iic device and XIic driver to exercise the 10-bit Address functionality of the
iic device.

For details, see xiic_tenbitaddr_example.c.
*/
