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

#include "datatype.h"

// Uncomment the line corresponding to the STMicroelectronics evaluation board
// used to run the example 
//#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL)
// #define USE_STM3210E_EVAL
//#endif

//#if !defined (USE_STM32F103RD)
//    #define USE_STM32F103RD
//#endif

// Define the STM32F10x hardware depending on the used evaluation board
//#ifdef USE_STM32F103RD
//  #define FLASH_PAGE_SIZE    ((u16)0x400)	//1K for stm32f103rb
//#elif defined USE_STM3210E_EVAL
//  #define FLASH_PAGE_SIZE    ((u16)0x800) //2K
//#endif


//=============================================================================
// Export Function
//=============================================================================
void InitGpio(void);
void InitCounterTimer(void);
void InitWatchDog(void);
void WaitMs(UINT32 millisecond);
void DataBusMode(GPIOMODE mode);
void StartCounter(tagCounter *counter,INT32 ms);
BOOL CounterArrived(tagCounter *counter);

#endif
