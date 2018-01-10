/*
 * Demo on how to initialse LwIP TCP/IP MAC and start LwIP tasks
 *
 *  If you are connecting the Xplorer board directly to your host computer
 *  then a point to point (cross over) network cable is required.
 *  If you are connecting the Xplorer board to a network hub or switch
 *  then a standard network cable can be used.
 *
 *  (WARNING) currently this driver can only run in point-to-point mode.
 *  going through a hub/switch is errornos
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Standard includes. */
#include "stdlib.h"
#include <string.h>
#include <math.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

/* include ethernet drivers */
#include "freertos_eth.h"
#include "freertos_buffer.h"

/* include the port-dependent configuration */
#define LWIP_PORT_INIT_IPADDR(addr)   IP4_ADDR((addr), configIP_ADDR0,configIP_ADDR1,configIP_ADDR2,configIP_ADDR3)
#define LWIP_PORT_INIT_GW(addr)       IP4_ADDR((addr), 192,168,1,1)
#define LWIP_PORT_INIT_NETMASK(addr)  IP4_ADDR((addr), 255,255,255,0)

/* The maximum time to block waiting to obtain the xTxBufferMutex to become
available. */
#define lwipappsMAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS   ( 29 / portTICK_RATE_MS )

/* maximum connections */
#define BACKLOG             6

/* service port */
#define ECHO_PORT           27015

/* enable debug xil_printf */
#define DBG_SERVER          1

/* registered functions */
void echo_server( void *pvParameters );

/*-----------------------------------------------------------*/

/* print the IP autonegotiation result of MAC */
void vStatusCallback( struct netif *pxNetIf )
{
char pcMessage[20];
struct in_addr* pxIPAddress;

    if( netif_is_up( pxNetIf ) != 0 )
    {
        strcpy( pcMessage, "IP=" );
        pxIPAddress = ( struct in_addr* ) &( pxNetIf->ip_addr );
        strcat( pcMessage, inet_ntoa( ( *pxIPAddress ) ) );
        xil_printf( pcMessage );
        xil_printf("\n");
    }
    else
    {
        xil_printf( "Network is down" );
    }
}

/*
 * The function that implements the lwIP based sockets command interpreter
 * server.
 */
extern void echo_server( void *pvParameters );

/* Called from the TCP/IP thread. */
void lwIPAppsInit( void *pvArgument )
{
ip_addr_t xIPAddr, xNetMask, xGateway;
extern err_t xemacpsif_init( struct netif *netif );
extern void xemacif_input_thread( void *netif );
static struct netif xNetIf;

    ( void ) pvArgument;

    /* Set up the network interface. */
    ip_addr_set_zero( &xGateway );
    ip_addr_set_zero( &xIPAddr );
    ip_addr_set_zero( &xNetMask );

    LWIP_PORT_INIT_GW( &xGateway );
    LWIP_PORT_INIT_IPADDR( &xIPAddr );
    LWIP_PORT_INIT_NETMASK( &xNetMask );

    /* Set MAC address */
    xNetIf.hwaddr_len = 6;
    xNetIf.hwaddr[ 0 ] = configMAC_ADDR0;
    xNetIf.hwaddr[ 1 ] = configMAC_ADDR1;
    xNetIf.hwaddr[ 2 ] = configMAC_ADDR2;
    xNetIf.hwaddr[ 3 ] = configMAC_ADDR3;
    xNetIf.hwaddr[ 4 ] = configMAC_ADDR4;
    xNetIf.hwaddr[ 5 ] = configMAC_ADDR5;

    /* setup the network interface */
    netif_set_default( netif_add( &xNetIf, &xIPAddr, &xNetMask, &xGateway, ( void * ) XPAR_XEMACPS_0_BASEADDR, xemacpsif_init, tcpip_input ) );
    netif_set_status_callback( &xNetIf, vStatusCallback );
    netif_set_up( &xNetIf );

    /* parsing the network interface :
     * The priority for the task that unblocked by the MAC interrupt to process received packets,
     * which should be REALTIME */
    sys_thread_new( "lwIP_In", xemacif_input_thread, &xNetIf, configMINIMAL_STACK_SIZE, REALTIME_PRIORITY );

    /* Create the FreeRTOS defined basic command server.  This demonstrates use
    of the lwIP sockets API. */
    xTaskCreate( echo_server, "MFEIT-ECH", configMINIMAL_STACK_SIZE * 5, NULL, HIGHEST_PRIORITY, NULL );

}

/*
 * basic MFEIT server code, a command & response DATA exchange system
 */
void echo_server( void *pvParameters )
{
    ( void ) pvParameters;

    /* static socket */
    static long lSocket, lClientFd;

    long lAddrLen = sizeof( struct sockaddr_in );
    struct sockaddr_in sLocalAddr;
    struct sockaddr_in client_addr;

    extern char *tcp_rx_ptr;

    int ret;
    int npacket;

    /* step 1. create */
    lSocket = lwip_socket(AF_INET, SOCK_STREAM, 0);

    if( lSocket >= 0 )
    {
        /* prepare bind on port */
        memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
        sLocalAddr.sin_family = AF_INET;
        sLocalAddr.sin_len = sizeof(sLocalAddr);
        sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sLocalAddr.sin_port = ntohs( ( ( unsigned short ) ECHO_PORT ) );

        /* step 2. bind */
        ret = lwip_bind( lSocket, ( struct sockaddr *) &sLocalAddr, sizeof( sLocalAddr ) );
        if( ret != 0 )
        {
            lwip_close( lSocket );
            vTaskDelete( NULL );
        }

        /* step 3. listen */
        ret = lwip_listen( lSocket, BACKLOG );
        if( ret != 0 )
        {
            lwip_close( lSocket );
            vTaskDelete( NULL );
        }

        for( ;; )
        {

            /* step 4. accept */
            lClientFd = lwip_accept(lSocket, ( struct sockaddr * ) &client_addr, ( u32_t * ) &lAddrLen );
            /* notify the user a new connection has been made */
#if(DBG_SERVER)
            xil_printf("(ZYNQ-ECH) accept connection\n");
#endif

            /* if client socket created */
            if( lClientFd > 0L )
            {

                npacket = 0;
                while(1)
                {

                    /* copy and wait for current BD */
                    u8 *RxBufferPtr = getValidRxBasePtr();

                    /* in this demo, we only update the first-8 u32 words in data */
                    static unsigned int cnt = 1;
                    u32 *buf_ptr = (u32 *)RxBufferPtr;
                    int i;

                    cnt ++;
                    unsigned int cnt_mod = (int) fmod( cnt, 16 );
                    for (i=0; i<8; i++)
                        *(buf_ptr + i) = cnt_mod;

                    /* 1. read command */
                    ret = mfeit_read(lClientFd, tcp_rx_ptr);
                    if (ret == -1) {
                        /* notify the user COMMAND read errors */
#if(DBG_SERVER)
                        xil_printf("(ZYNQ-ECH) read ERROR\n");
#endif
                        break;
                    }

                    /* 2. write buffer */
                    const unsigned int TX_SIZE = RX_REG_OFFSET + 16*MAX_PKT_LEN;
                    ret = mfeit_write( lClientFd, (char *)RxBufferPtr, TX_SIZE );
                    if (ret == -1) {
                        /* notify the user DATA write errors */
#if(DBG_SERVER)
                        xil_printf("(ZYNQ-ECH) write ERROR\n");
#endif
                        break;
                    }

                    npacket ++;
                }

                /* gracefully exit */
                if (lwip_shutdown(lClientFd, SHUT_WR) == 0)
                    /* notify the user the remote has shutdown the connection */
#if(DBG_SERVER)
                    xil_printf("(ZYNQ-ECH) shutdown write\n");
#endif

                /* continue to receive until the last packet */
                do {
                    ret = mfeit_read(lClientFd, tcp_rx_ptr);
                } while (ret == 0);

                /* notify the user the server acknowledge the shutdown */
#if(DBG_SERVER)
                xil_printf("(ZYNQ-ECH) close connection, total = %d\n", npacket);
#endif
                lwip_close( lClientFd );
            }
        }
    }

    /* Will only get here if a listening socket could not be created. */
#if(DBG_SERVER)
    xil_printf("(ZYNQ-ECH) task killed !\n");
#endif

    vTaskDelete( NULL );
}

