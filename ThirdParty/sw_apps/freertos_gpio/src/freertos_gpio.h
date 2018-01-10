#ifndef FREERTOS_GPIO_H
#define FREERTOS_GPIO_H

/* XILINX includes. */
#include "xil_io.h"
#include "xparameters.h"

/* use AXI GPIO or GPIOPS */
#define USE_AXI_GPIO           0
#define USE_PS_GPIO            1

/*-----------------------------------------------------------*/

#if(USE_AXI_GPIO)
#include "xgpio.h"
#define AXI_GPIO1_CHANNEL      ( 1 )
#endif

/*-----------------------------------------------------------*/

#if(USE_PS_GPIO)
#include "xgpiops.h"
#define partstOUTPUT_ENABLED   ( 1 )
#define partstDIRECTION_OUTPUT ( 1 )
#define partstDIRECTION_INPUT  ( 0 )
#define partstLED_OUTPUT       ( 47 )
#define partstBTN_INPUT        ( 51 )
#endif

/*-----------------------------------------------------------*/

/* INIT, Write, Read, and Toggle */
void GPIO_Init( void );
void GPIO_Write( UBaseType_t uxLED, BaseType_t xValue );
unsigned int GPIO_Read( UBaseType_t uxLED );
void GPIO_Toggle( unsigned portBASE_TYPE uxLED );

#endif
