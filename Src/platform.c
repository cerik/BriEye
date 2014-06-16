//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#include <stdio.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"
#include "datatype.h"
#include "ucos_ii.h"
#include "misc.h"
#include "ad7731.h"

#pragma import(__use_no_semihosting_swi)

/*
 **********************************************************
 * Global Varibale & Macro & Function Declaration
 **********************************************************
 */
//#define SYSCLK  48000000    //48M
//#define HCLK    48000000    //48M
//#define APB1    24000000    //24M 

/*
 *---------------------------------------------------------
 * Function:
 *   Remap JTAG Port pin
 * Parameters:
 * Return:
 * Notes:
 *  remap PD0 & PD1 as IO port or as Jtag port.  
 * Edit Log:
 *  2011.08.06--CCZY--Create
 *---------------------------------------------------------
 */
#define jtag_unremap()\
    do{\
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;\
        AFIO->MAPR &= ~(7 << 24);\
    }while(0)
#define jtag_remap()\
    do{\
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;\
        AFIO->MAPR &= ~(7 << 24);\
        AFIO->MAPR |=  (4 << 24);\
    }while(0)

/*
 **********************************************************
 * Local Function Definition
 **********************************************************
 */
static UINT32 gSysTickCounter=0;

//**********************************************************
//  Tick Cycle = 1ms
//
UINT32  OS_CPU_SysTickClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    RCC_GetClocksFreq(&rcc_clocks);
    return ((UINT32)rcc_clocks.HCLK_Frequency/8);
}

/****************************************************
 *   Independent Watchdog Configure
 *   IWDGCLK = 40 kHz
 *   WDT Timeout = 5s
 *   CNT_CLK = 40 kHz / 256 = 156.25 Hz
 ****************************************************/ 
#define __IWGDCLK     (40000UL/(0x04<<IWDG_Prescaler_256))
void InitWatchDog(UINT32 ms)
{
    UINT16 reload;
    reload = ms* 40000UL/(0x04<<IWDG_Prescaler_256)/1000UL-1;
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);//clk = 40 kHz / 256 = 156.25 Hz
    IWDG_SetReload(reload);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/*
 ***********************************************************
 * Description:
 * 
 * Edit Log:
 *  2011.08.06--CCZY--Create
 ***********************************************************
 */
void InitGpio(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB, ENABLE);
    
    // PC.[0,1,2,3,] --> Output,Push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PC.[6~9] are connected with the interface connect of the device.
    // We don't know its function, so just configured it with output + push-pull.
    // It's default value is 0.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //PB1 -->LCD_BUSY,Input pull-down.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    //GPIO_Write(GPIOC,0x0000);
    
}

//**********************************************************
// Set the IMGSW1 ~ IMGSW4
// Only the low four bits are valid.
void SetSWx(UINT8 value)
{
    UINT16 tmp;
    tmp = GPIO_ReadInputData(GPIOC) & 0xFFF0;
    GPIO_Write(GPIOC,tmp | (value&0xF));
}

//**********************************************************
// Set the PC6 ~ PC
// Only the low four bits are valid.
void SetDbgPort(UINT8 value)
{
    UINT16 tmp;
    tmp = GPIO_ReadInputData(GPIOC) & 0xFC3F;
    tmp |= (value&0xf)<<6;
    GPIO_Write(GPIOC,tmp);
}

BOOL LcdReady(void)
{
    return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)?FALSE:TRUE;
}

//**********************************************************
//  Init the External Interrupt for the Button 1 & 2;
// 
void InitExtInterrupt(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
    
    //PA1 = Button 1
    //PA2 = Button 2
    // Configure Key Button GPIO Pin as input floating (Key Button EXTI Line)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Connect Key Button EXTI Line to Key Button GPIO Pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);

    EXTI_ClearITPendingBit(EXTI_Line1);
    EXTI_ClearITPendingBit(EXTI_Line2);
  
    /* Configure Key Button EXTI Line to generate an interrupt on falling edge */  
    EXTI_InitStructure.EXTI_Line = EXTI_Line1 | EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

//=================================================================
//  PCLK1 = 24MHz , CLK_TIM2 = 2*PCLK1 = 48MHz
//
void InitTIM2()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_DeInit(TIM2);    
    TIM_TimeBaseInitStruct.TIM_Period        = 1000; //1s
    TIM_TimeBaseInitStruct.TIM_Prescaler     = 48000-1; 
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode   = TIM_CounterMode_Down;
    
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2,ENABLE);
}

//**********************************************************
// Initialize the ADC port: SPI1
//  PA4 --> AD_Reset(GPIO)
//  PA5 --> AD_SCLK (SPI)
//  PA6 --> AD_MISO (SPI)
//  PA7 --> AD_MOSI (SPI)
//  PC4 --> AD_CS   (GPIO)
//  PC5 --> AD_RDY  (GPIO)
void InitSPI1(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO,ENABLE);
#if 1
    //Reset Pin, Output,Push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //SCLK & MOSI,Output,Push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //MISO,Input,Input,Pull-up.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //AD_CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
   
    //AD_RDY
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_CPHA =SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_CPOL =SPI_CPOL_High;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1,&SPI_InitStructure);
    SPI_Cmd(SPI1,ENABLE);
#else
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC,ENABLE);
    //Reset Pin, Output,Push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //AD_CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
}

/*
 ***************************************************
 *   Free Counter Function Group
 *   Counter Unit: 1ms
 ***************************************************
 */
void StartCounter(tagCounter *counter,INT32 ms)
{
    counter->t0 = OSTimeGet();
    counter->ms = ms;
}

BOOL CounterArrived(tagCounter *counter)
{
    if(counter->ms > 0)
    {
        counter->t1 = OSTimeGet();
        counter->ms -= (counter->t1 < counter->t0)?(counter->t1 + 0xFFFFFFFF - counter->t0):(counter->t1 - counter->t0);
        counter->t0 = counter->t1;
    }
    return (counter->ms > 0)?FALSE:TRUE;
}

//**********************************************************
//
void  WaitMs(UINT32 ms)
{
    OSTimeDlyHMSM(0,0,ms/1000,ms%1000);
    //UINT32 old;
    //old = gSysTickCounter+ms;
    //while(gSysTickCounter < old);
}

void  App_TimeTickHook(void)
{
    //IWDG_ReloadCounter();
    gSysTickCounter++;
}


//**********************************************************
// Button 1 Interrupt Service Routine
// Input Line 1, PA1; Rising Edge Trigger.
//
void EXTI1_IRQHandler(void)
{
    if ( EXTI_GetITStatus(EXTI_Line1) != RESET )
    {
        EXTI_ClearITPendingBit(EXTI_Line1);
        printf("Button 1 Released\n");
    }
}

//**********************************************************
// Button 1 Interrupt Service Routine
// Input Line 2, PA2; Rising Edge Trigger.
//
void EXTI2_IRQHandler(void)
{
    if ( EXTI_GetITStatus(EXTI_Line2) != RESET )
    {
        EXTI_ClearITPendingBit(EXTI_Line2);
        printf("Button 2 Released\n");
    }
}

void TIM2_IRQHandler(void)
{
    if(SET == TIM_GetITStatus(TIM2,TIM_FLAG_Update))
    {
        TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
    }
}
