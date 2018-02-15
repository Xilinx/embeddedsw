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

These settings can be changed in the file main.c.

The TCP server connection and statistics logic is present in the file
freertos_lwip_tcp_server.c.

Running the Freertos LwIP TCP server example
--------------------------------------------

To connect and test the TCP server, download and run the program on the board,
and then issue the following command from your host machine:

$ iperf -c <Server IP address> -i 5 -t 300 -w 2M
