/*-----------------------------------------------------------
 * Simple IO routines to control the LEDs.
 * This file is called ParTest.c for historic reasons.
 * Originally it stood for PARallel port TEST.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 *-----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo includes. */
#include "freertos_gpio.h"

/*
 * TODO:
 *     Support multiple AXI GPIO instance,
 *     and arbitrary GPIO bus width
 */
#if(USE_AXI_GPIO)
static XGpio axi_gpio1;
#endif

/*-----------------------------------------------------------*/

#if(USE_PS_GPIO)
static XGpioPs xGpio;
#endif

/*-----------------------------------------------------------*/

void GPIO_Init( void )
{

    /* Initialise AXI GPIO */
#if(USE_AXI_GPIO)
    XGpio_Initialize(&axi_gpio1, XPAR_AXI_GPIO_1_DEVICE_ID);
    /* xil_printf("AXI GPIO = %x\n", axi_gpio1.BaseAddress); */
#endif

    /* Operating on PS GPIO
     * PS GPIO is PS point's GPIO, here is the example on how to use them */
#if(USE_PS_GPIO)
    /* xGpio is a static variable in the top */
    XGpioPs_Config *pxConfigPtr;
    BaseType_t xStatus;

    /* Initialise the GPIO driver. */
    pxConfigPtr = XGpioPs_LookupConfig( XPAR_XGPIOPS_0_DEVICE_ID );
    xStatus = XGpioPs_CfgInitialize( &xGpio, pxConfigPtr, pxConfigPtr->BaseAddr );
    configASSERT( xStatus == XST_SUCCESS );
    /* Remove compiler warning if configASSERT() is not defined. */
    ( void ) xStatus;

    /* Enable led and set low. */
    XGpioPs_SetDirectionPin( &xGpio, partstLED_OUTPUT, partstDIRECTION_OUTPUT );
    XGpioPs_SetOutputEnablePin( &xGpio, partstLED_OUTPUT, partstOUTPUT_ENABLED );
    XGpioPs_WritePin( &xGpio, partstLED_OUTPUT, 0x0 );

    /* enable buttons */
    XGpioPs_SetDirectionPin( &xGpio, partstBTN_INPUT, partstDIRECTION_INPUT );
#endif

}
/*-----------------------------------------------------------*/

void GPIO_Write( UBaseType_t uxLED, BaseType_t xValue )
{
    ( void ) uxLED;

    /* for AXI GPIO, you can */
#if(USE_AXI_GPIO)
    XGpio_DiscreteWrite(&axi_gpio1, AXI_GPIO1_CHANNEL, 0x0);
#endif

    /* for PS GPIO, you can */
#if(USE_PS_GPIO)
    XGpioPs_WritePin( &xGpio, partstLED_OUTPUT, xValue );
#endif
}
/*-----------------------------------------------------------*/

unsigned int GPIO_Read( UBaseType_t uxLED )
{
    ( void ) uxLED;
    unsigned int val = 0x0;

    /* for AXI GPIO, you can */
#if(USE_AXI_GPIO)
    val = XGpio_DiscreteRead(&axi_gpio1, AXI_GPIO1_CHANNEL);
#endif

    /* for PS GPIO, you can */
#if(USE_PS_GPIO)
    val = XGpioPs_ReadPin( &xGpio, partstBTN_INPUT );
#endif

    return val;
}
/*-----------------------------------------------------------*/

void GPIO_Toggle( unsigned portBASE_TYPE uxLED )
{
    ( void ) uxLED;

    static int xLEDState = 0x1;

    /* for AXI GPIO, you may */
#if(USE_AXI_GPIO)
    xLEDState = !xLEDState;
    XGpio_DiscreteWrite(&axi_gpio1, AXI_GPIO1_CHANNEL, xLEDState);
#endif

    /* while for PS GPIO, you may */
#if(USE_PS_GPIO)
    xLEDState = XGpioPs_ReadPin( &xGpio, partstLED_OUTPUT );
    XGpioPs_WritePin( &xGpio, partstLED_OUTPUT, !xLEDState );
#endif

}
/*-----------------------------------------------------------*/

