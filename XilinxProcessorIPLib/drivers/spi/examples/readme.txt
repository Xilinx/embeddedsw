/** \page example Examples
You can refer to the below stated example applications for more details on how to use spi driver.

@section ex1 xspi_selftest_example.c
Contains an example on how to use the XSpi driver directly.
This example performs the basic selftest using the driver.

For details, see xspi_selftest_example.c.

@section ex2 xspi_atmel_flash_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the SPI driver and hardware device
with an Atmel Serial Flash Device (AT45XX series). This example erases the
Page, writes to the Page, reads back from the Page and compares the data.

For details, see xspi_atmel_flash_example.c.

@section ex3 xspi_eeprom_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the SPI driver and hardware device
with a serial EEPROM device.

For details, see xspi_eeprom_example.c.

@section ex4 xspi_intel_flash_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the SPI driver and hardware device with
an Intel Serial Flash Memory(S33) in the interrupt mode. This example
erases a sector, writes to a Page within the sector, reads back from that
Page and compares the data.

For details, see xspi_intel_flash_example.c.

@section ex5 xspi_intr_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the Spi driver and the Spi device
using the interrupt mode.

For details, see xspi_intr_example.c.

@section ex6 xspi_low_level_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the low-level driver of the SPI driver.
These macros are found in xspi_l.h.  A simple loopback test is done
within an SPI device in polled mode. This example works only with 8-bit
wide data transfers.

For details, see xspi_low_level_example.c.

@section ex7 xspi_numonyx_flash_quad_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the SPI driver and axi_qspi device with a
Numonyx quad serial flash device in the interrupt mode. This example
erases a Sector, writes to a Page within the Sector, reads back from that
Page and compares the data.

For details, see xspi_numonyx_flash_quad_example.c.

@section ex8 xspi_polled_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the Spi driver and the Spi device
using the polled mode.

For details, see xspi_polled_example.c.

@section ex9 xspi_slave_intr_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the Spi driver and the Spi device as a
Slave, in interrupt mode.

For details, see xspi_slave_intr_example.c.

@section ex10 xspi_slave_polled_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the Spi driver and the Spi device as a
Slave, in polled mode.

For details, see xspi_slave_polled_example.c.

@section ex11 xspi_stm_flash_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the SPI driver and hardware device with
an STM serial Flash device (M25P series) in the interrupt mode. This
example erases a Sector, writes to a Page within the Sector, reads back
from that Page and compares the data.

For details, see xspi_stm_flash_example.c.

@section ex12 xspi_winbond_flash_quad_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the SPI driver and axi_qspi device with
a Winbond quad serial flash device in the interrupt mode. This example
erases a Sector, writes to a Page within the Sector, reads back from
that Page and compares the data.

For details, see xspi_winbond_flash_quad_example.c.

@section ex13 xspi_winbond_flash_xip_example.c
Contains an example on how to use the XSpi driver directly.
This example shows the usage of the Spi driver and the Spi device
configured in XIP Mode. This example reads data from the Flash Memory
in the way RAM is accessed.

For details, see xspi_winbond_flash_xip_example.c.
*/
