//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f10x_gpio.h"
#include "datatype.h"
#include "datatype.h"

/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL)
 #define USE_STM3210E_EVAL
#endif

#if !defined (USE_STM32F103RD)
    #define USE_STM32F103RD
#endif

/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM32F103RD
//  #define FLASH_PAGE_SIZE    ((u16)0x400)	//1K for stm32f103rb
#elif defined USE_STM3210E_EVAL
//  #define FLASH_PAGE_SIZE    ((u16)0x800) //2K
#endif /* USE_STM3210B_EVAL */


/* Uncomment the line corresponding to the select the USART used to run the example */
#define  USARTx                     USART1
#define  GPIOx                      GPIOA
#define  RCC_APB2Periph_GPIOx       RCC_APB2Periph_GPIOA
#define  GPIO_RxPin                 GPIO_Pin_10
#define  GPIO_TxPin                 GPIO_Pin_9

/*
 **********************************************************
 * Export Area
 **********************************************************
 */
void  putch(UINT8 ch);
UINT8 getch(BOOL block);

void InitGpio(void);
void InitUart1(void);
void InitCounterTimer(void);
void InitWatchDog(void);
void WaitMs(UINT32 millisecond);

void DataBusMode(GPIOMODE mode);

void StartCounter(tagCounter *counter,INT32 ms);
BOOL CounterArrived(tagCounter *counter);

#endif
