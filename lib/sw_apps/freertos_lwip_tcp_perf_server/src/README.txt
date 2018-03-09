FreeRTOS LwIP TCP Server
------------------------

The FreeRTOS LwIP TCP Server application creates TCP server for handling
TCP clients. It can connect to any number of clients but can serve only
1 connection at a time. Once IP is assigned, it will start listening
for any client connection.

Following TCP server options can be changed in file freertos_lwip_tcp_server.h,
1) INTERIM_REPORT_INTERVAL - time interval in sec in which intermediate report
can be displayed (default 5 secs).
2) TCP_SERVER_PORT - Port to be used for connecting with client (default 5001)

If LWIP_DHCP enabled then board should get IP address from DHCP server.
If DHCP timeout happens or LWIP_DHCP disabled then, the program assigns the
following IP settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02

Procedure to enable IPv6
------------------------
A) From SDK GUI.
1. Create lwip_tcp_perf application from XSDK
2. Go to BSP Settings from XSDK
   --> Overview
      --> freertos10_xilinx
        --> lwip202
           --> ipv6_enable
               - Select "true" from value tab.
3. Build complete project.

B) From mss file.
1. Before creating project update below parameter.
   - FILE: freertos_lwip_tcp_perf_server.mss
     - PARAMETER ipv6_enable = true

If LWIP_IPV6 enabled then board should configured with IPv6 link local address.
following IPv6 settings to the board:
link local IPv6 Address: FE80:0:0:0:20A:35FF:FE00:102

These settings can be changed in the file main.c.

The TCP server connection and statistics logic is present in the file
freertos_lwip_tcp_server.c.

Running the Freertos LwIP TCP server example
--------------------------------------------

To connect and test the TCP server, download and run the program on the board,
and then issue the following command from your host machine:

For IPv4,
$ iperf -c <Board IP address> -i 5 -t 300 -w 2M

For IPv6,
$ iperf -V -c <Board IP address>%<interface> -i 5 -t 300 -w 2M

[Note: For Link local IPv6 address, we need to specify interface in iperf to
define the scope where the link local address is valid]
