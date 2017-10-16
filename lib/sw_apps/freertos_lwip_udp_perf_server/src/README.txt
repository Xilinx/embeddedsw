FreeRTOS LwIP UDP Perf Server
-----------------------------

The FreeRTOS LwIP UDP Perf Server application creates UDP server for providing
connections to UDP client (Iperf 2.0.5) running on host machine.
Once remote client connects with this server, UDP server will start receiving
data from client.
Connection details and data transfer statistics will be displayed by server
on serial console.

Following UDP server options can be changed in file udp_perf_server.h,
1) INTERIM_REPORT_INTERVAL: time interval (in secs) for intermediate report
display interval. (default 5 secs)
2) UDP_CONN_PORT - Port on which server will listen for client connection.

If LWIP_DHCP enabled then board should get IP address from DHCP server.
If DHCP timeout happens or LWIP_DHCP is disabled then, the program assigns the
following IP settings to the board:
IP Address: 192.168.1.10
Netmask   : 255.255.255.0
Gateway   : 192.168.1.1
MAC address:  00:0a:35:00:01:02

These settings can be changed in the file main.c.

The UDP server connection and statistics logic is present in the file
udp_perf_server.c

Running the FreeRTOS LwIP UDP server example
--------------------------------------------

To connect and test the UDP server, download and run the application on
the board, and then issue the following command from your host machine:

$ iperf -c <Board IP address> -i 5 -t 300 -u -b <bandwidth>
