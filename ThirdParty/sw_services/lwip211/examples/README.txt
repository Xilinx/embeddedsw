Requirements for all examples
------------------------------
1. lwip211 library should included in the bsp with the following
configurable options:
dhcp_does_arp_check = true
lwip_dhcp = true
API_MODE = RAW_API if raw example if being used;
API_MODE = SOCKET_API if freertos example if being used.
igmp_options = true for IGMP example

2. xilffs file system is necessary to run webserver and tftp server/client
examples. Include xilffs in the bsp with the following options:
fs_interface = 2
API_MODE = RAW_API if raw example if being used;
API_MODE = SOCKET_API if freertos example if being used.

3. Enable "SELECT_TFTPAPP" in platform_config.h except for IGMP tests.

4. To select 16550 as STDOUT, enable SELECT_STDOUT16550 in platform_config.h

5. To use axi ethernet on zynq, enable SELECT_USESOFTETH in platform_config.h


lwIP tftp server
----------------

Files to be included:
lwip_example_i2c_access.c
lwip_example_iic_phyreset.c
lwip_example_platform.c
lwip_example_platform.h
lwip_example_platform_config.h
lwip_example_platform_mb.c
lwip_example_platform_zynq.c
lwip_example_platform_zynqmp.c
lwip_example_sfp.c
lwip_example_si5324.c
lwip_example_tftp_platform_fs.c
lwip_example_tftp_platform_fs.h
lwip_example_tftp_server.c
lwip_example_tftpserver_common.h
lwip_example_tftpserver_main.c

The lwIP tftp server application starts a tftp server at port 69.

The program assigns the following settings to the board if DHCP is disabled or
DHCP request times out:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp server logic is present in the file tftp_server.c.

Running the tftp server example
-------------------------------

To connect and test the tftp server.

Run tftp client on host machine:
$ tftp 192.168.1.10
$ tftp > put sample1.txt
Sent 60 bytes in 0.0 seconds

Logs on board:
TFTP WRQ (write request): sample1.txt
TFTP_WRQ: Transfer completed


lwIP tftp client
----------------

Files to be included:
lwip_example_i2c_access.c
lwip_example_iic_phyreset.c
lwip_example_platform.c
lwip_example_platform.h
lwip_example_platform_config.h
lwip_example_platform_mb.c
lwip_example_platform_zynq.c
lwip_example_platform_zynqmp.c
lwip_example_sfp.c
lwip_example_si5324.c
lwip_example_tftp_platform_fs.c
lwip_example_tftp_platform_fs.h
lwip_example_tftp_client.c
lwip_example_tftpclient_common.h
lwip_example_tftpclient_main.c

The lwIP tftp client application gets/puts files from/to tftp server running on host machine.

The program assigns the following settings to the board if DHCP is disabled or
DHCP request times out:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp client logic is present in the file tftp_client.c.
TFTP client application will put and get 3 files from TFTP server(default: 10.10.70.101).
TFTP server ip can be changed from tftp_common.h.

Running the tftp client example
-------------------------------

To test the tftp client application tftp server should be running on the host machine.

Log on board:
TFTP_WRQ: tftp get sample1.txt
TFTP_WRQ: Transfer completed

TFTP_WRQ: tftp get sample2.txt
TFTP_WRQ: Transfer completed

TFTP_WRQ: tftp get sample3.txt
TFTP_WRQ: Transfer completed

TFTP_RRQ: tftp get sample1.txt
TFTP_RRQ: Transfer completed

TFTP_RRQ: tftp get sample2.txt
TFTP_RRQ: Transfer completed

TFTP_RRQ: tftp get sample3.txt
TFTP_RRQ: Transfer completed


lwIP Webserver
--------------

Files to be included:
lwip_example_i2c_access.c
lwip_example_iic_phyreset.c
lwip_example_platform.c
lwip_example_platform.h
lwip_example_platform_config.h
lwip_example_platform_mb.c
lwip_example_platform_zynq.c
lwip_example_platform_zynqmp.c
lwip_example_sfp.c
lwip_example_si5324.c
lwip_example_ws_platform_fs.c
lwip_example_tftp_platform_fs.h
lwip_example_web_utils.c
lwip_example_ws_http_response.c
lwip_example_webserver.c
lwip_example_webserver.h
lwip_example_ws_main.c

The lwIP webserver application starts a webserver at port 80.

By default, the program assigns the following settings to the board:
IP Address : 192.168.1.10
Netmask    : 255.255.255.0
Gateway    : 192.168.1.1
MAC address: 00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main webserver logic is present in the file webserver.c.

platform.c implements certain processor and platform dependent functions.

Setting RAMFS parameters
------------------------

As the webserver uses RAM based FAT FS to store webpages, it's important to set
correct RAMFS parameters according to your h/w. By default, xilffs library takes
following value for ramfs_size and ramfs_start_addr:

ramfs_size - 3145728
ramfs_start_addr - 0x10000000

These values should work for most Zynq and ZynqMP solutions, but are likely to
be different for MicroBlaze. Please set appropriate parameters in xilffs section
of MSS file for your h/w, if they should be different.

Creating FAT image on Linux
---------------------------

This requires root (sudo) access on the Linux host

Following commands can be used on terminal to create FAT image to be used with
webserver application:

# create image file of 3MB
dd if=/dev/zero of=example.img bs=512 count=6144

# format image with FAT
/sbin/mkfs.vfat example.img

# mount it
mkdir /tmp/fs
sudo mount -t vfat -o loop,rw example.img /tmp/fs/

# copy your webpages
sudo cp -r  webpages_dir/* /tmp/fs/
sudo umount /tmp/fs

Running the webserver example
-----------------------------

The application expects webpages to be stored in RAM based FAT FS. Please
tune RAM FS start address and size according to the h/w design and size of
webpages in MSS file. Please load FAT FS image file in the RAM at the same
address as the one specified in the MSS file while running the application.

To connect and test the webserver, download the program and FAT image file
on the board, and then run the program. The server can be accessed using the
URL - http://<board-ip>; e.g. http://192.168.1.10

lwIP IGMP
---------

Files to be included:
lwip_example_i2c_access.c
lwip_example_iic_phyreset.c
lwip_example_platform.c
lwip_example_platform.h
lwip_example_platform_config.h
lwip_example_platform_mb.c
lwip_example_platform_zynq.c
lwip_example_platform_zynqmp.c
lwip_example_sfp.c
lwip_example_si5324.c
lwip_example_igmp_app.c
lwip_example_igmp_app.h
lwip_example_igmp_main.c

Requires igmp options(LWIP_IGMP) in lwip library configuration parameters.
The lwIP IGMP application joins a default multicast group of 224.10.10.3.
Any data sent to this Multicast IP address and default 5001 port,
will be accepted.
Once IGMP_MULTICAST_RXPKT_COUNT (defined in igmp_app.h) packets are received,
then application will send single multicast packet and leave multicast group.

Following IGMP Mulitcast options can be changed in file igmp_app.h,
1) IGMP_MULTICAST_IP_ADDRESS
2) IGMP_MULTICAST_PORT
3) IGMP_MULTICAST_RXPKT_COUNT

If LWIP_DHCP enabled then board should get IP address from DHCP server.
If DHCP timeout happens or LWIP_DHCP disabled then, the program assigns the
following IP settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02

These settings can be changed in the file main.c.

The main IGMP logic is present in the file igmp_app.c.

Running the IGMP example
------------------------

To connect and test the IGMP, download and run the program on the board,
and then issue the following command from your host machine:

$ iperf -c 224.10.10.3 -t 2 -u -b 1M -B <Host IP address>


FreeRTOS lwIP tftp server
-------------------------

Files to be included:
lwip_example_iic_phyreset.c
lwip_example_platform_config.h
freertos_lwip_example_tftp_platform_fs.c
freertos_lwip_example_tftp_platform_fs.h
freertos_lwip_example_tftp_server.c
freertos_lwip_example_tftpserver_common.h
freertos_lwip_example_tftpserver_main.c

The FreeRTOS lwIP tftp server application starts a tftp server at port 69.

By default, the program assigns the following settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp server logic is present in the file tftp_server.c.

Running the tftp server example
-------------------------------

To connect and test the tftp server.

Run tftp client on host machine:
$ tftp 192.168.1.10
$ tftp > put sample1.txt
Sent 60 bytes in 0.0 seconds

Logs on board:
TFTP WRQ (write request): sample1.txt
TFTP_WRQ: Transfer completed

FreeRTOS lwIP tftp client
-------------------------

Files to be included:
lwip_example_iic_phyreset.c
lwip_example_platform_config.h
freertos_lwip_example_tftp_platform_fs.c
freertos_lwip_example_tftp_platform_fs.h
freertos_lwip_example_tftp_client.c
freertos_lwip_example_tftpclient_common.h
freertos_lwip_example_tftpclient_main.c

The FreeRTOS lwIP tftp client application gets/puts files from/to tftp server running on host machine.

By default, the program assigns the following settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main tftp client logic is present in the file tftp_client.c.
TFTP client application will put and get 3 files from TFTP server(default: 10.10.70.101).
TFTP server ip can be changed from tftp_common.h.

platform.c implements certain processor and platform dependent functions.
The file platform_config.h is generated based on the hardware design.
It makes two assumptions:
  - The timer has its interrupt line connected to the interrupt controller.
  - All the ethernet peripherals (xps_ethernetlite or xps_ll_temac) accessible from
    the processor can be used with lwIP.

Running the tftp client example
-------------------------------

To test the tftp client application tftp server should be running on the host machine.
And sample1.txt, sample2.txt and sample3.txt should be there on tftp server.

Log on board:
TFTP_WRQ: tftp get sample1.txt
TFTP_WRQ: Transfer completed

TFTP_WRQ: tftp get sample2.txt
TFTP_WRQ: Transfer completed

TFTP_WRQ: tftp get sample3.txt
TFTP_WRQ: Transfer completed

TFTP_RRQ: tftp get sample1.txt
TFTP_RRQ: Transfer completed

TFTP_RRQ: tftp get sample2.txt
TFTP_RRQ: Transfer completed

TFTP_RRQ: tftp get sample3.txt
TFTP_RRQ: Transfer completed


FreeRTOS lwIP Webserver
-----------------------

Files to be included:
lwip_example_iic_phyreset.c
lwip_example_platform_config.h
freertos_lwip_example_ws_platform_fs.c
freertos_lwip_example_web_utils.c
freertos_lwip_example_ws_http_response.c
freertos_lwip_example_webserver.c
freertos_lwip_example_webserver.h
freertos_lwip_example_ws_main.c

The FreeRTOS lwIP webserver application starts a webserver at port 80.

By default, the program assigns the following settings to the board:
IP Address : 192.168.1.10
Netmask    : 255.255.255.0
Gateway    : 192.168.1.1
MAC address: 00:0a:35:00:01:02
These settings can be changed in the file main.c.

The main webserver logic is present in the file webserver.c.

Setting RAMFS parameters
------------------------

As the webserver uses RAM based FAT FS to store webpages, it's important to set
correct RAMFS parameters according to your h/w. By default, xilffs library takes
following value for ramfs_size and ramfs_start_addr:

ramfs_size - 3145728
ramfs_start_addr - 0x10000000

These values should work for most Zynq and ZynqMP solutions, but are likely to
be different for MicroBlaze. Please set appropriate parameters in xilffs section
of MSS file for your h/w, if they should be different.

Creating FAT image on Linux
---------------------------

This requires root (sudo) access on the Linux host

Following commands can be used on terminal to create FAT image to be used with
webserver application:

# create image file of 3MB
dd if=/dev/zero of=example.img bs=512 count=6144

# format image with FAT
/sbin/mkfs.vfat example.img

# mount it
mkdir /tmp/fs
sudo mount -t vfat -o loop,rw example.img /tmp/fs/

# copy your webpages
sudo cp -r  webpages_dir/* /tmp/fs/
sudo umount /tmp/fs

Running the webserver example
-----------------------------

The application expects webpages to be stored in RAM based FAT FS. Please
tune RAM FS start address and size according to the h/w design and size of
webpages in MSS file. Please load FAT FS image file in the RAM at the same
address as the one specified in the MSS file while running the application.

To connect and test the webserver, download the program and FAT image file
on the board, and then run the program. The server can be accessed using the
URL - http://<board-ip>; e.g. http://192.168.1.10


FreeRTOS lwIP IGMP
------------------

Files to be included:
lwip_example_iic_phyreset.c
lwip_example_platform_config.h
freertos_lwip_example_igmp_app.c
freertos_lwip_example_igmp_app.h
freertos_lwip_example_igmp_main.c

The FreeRTOS lwIP IGMP application joins a default multicast group of
224.10.10.3.
Any data sent to this Multicast IP address and default 5001 port, will be
accepted.
Once IGMP_MULTICAST_RXPKT_COUNT (defined in igmp_app.h) packets are
received, then application will send single multicast packet and leave
multicast group.

Following IGMP Mulitcast options can be changed in file igmp_app.h,
1) IGMP_MULTICAST_IP_ADDRESS
2) IGMP_MULTICAST_PORT
3) IGMP_MULTICAST_RXPKT_COUNT

If LWIP_DHCP enabled then board should get IP address from DHCP server.
If DHCP timeout happens or LWIP_DHCP disabled then, the program assigns the
following IP settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02

These settings can be changed in the file main.c.

The main IGMP logic is present in the file igmp_app.c.

Running the IGMP example
------------------------

To connect and test the IGMP, download and run the program on the board,
and then issue the following command from your host machine:

$ iperf -c 224.10.10.3 -t 2 -u -b 1M -B <Host IP address>
