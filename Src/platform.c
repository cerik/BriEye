//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "clock_config.h"
#include "datatype.h"
#include "ucos_ii.h"
#include "misc.h"

#pragma import(__use_no_semihosting_swi)

/*
 **********************************************************
 * Global Varibale & Macro & Function Declaration
 **********************************************************
 */
#define SYSCLK  48000000    //48M
#define HCLK    48000000    //48M
#define APB1    24000000    //24M 

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

//=============================================================================
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
 ****************************************************/
#define __IWDG_PERIOD      5000000        //unit:us ;here is 5000ms  = 5s          
#define __IWGDCLK          (40000UL/(0x04<<__IWDG_PR))
#define __IWDG_RLR         (__IWDG_PERIOD*__IWGDCLK/1000000UL-1)

void InitWatchDog(void)
{
    IWDG_ReloadCounter();// enable write to PR, RLR
    IWDG_SetPrescaler(IWDG_Prescaler_256);//clk = 40 kHz / 256 = 156.25 Hz
    IWDG_SetReload(__IWDG_RLR);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/*
 *---------------------------------------------------------
 * Description:
 *   Configure the communication port with FPGA, include
 *   PB.[0,1,5,6,7,8,9,10,11,12,13,14,15]
 *   PA.[0,1,2,3,4,5,6,7]
 *   PD.[0,1]
 * 
 * Edit Log:
 *  2011.08.06--CCZY--Create
 *---------------------------------------------------------
 */
void InitGpio(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure;

    jtag_remap();
    
    //PA14
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //PD0,PD1
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    //set data bus as 0x0
    GPIO_Write(GPIOB,0xFFFF);
    GPIO_Write(GPIOA,0xFFFF);

    GPIO_WriteBit(GPIOD,GPIO_Pin_0,Bit_SET);
    GPIO_WriteBit(GPIOD,GPIO_Pin_1,Bit_SET);
}

//=============================================================================
//  Timer4 : Free Running Timer,CNT++ every 1ms.
// Note:
//   PCLK1 = base clock=24MHz
void InitCounterTimer(void)
{
    TIM_TimeBaseInitTypeDef  TIM4_TimeBaseStructure;

    NVIC_InitTypeDef NVIC_InitStructure;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);

    TIM_DeInit(TIM4);
    /* Time Base configuration */
    TIM4_TimeBaseStructure.TIM_Prescaler = __ARR(__TIMXCLK, __TIM4_PERIOD);
    TIM4_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM4_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM4_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM4_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
    TIM_TimeBaseInit(TIM4,&TIM4_TimeBaseStructure);       

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM4, ENABLE);
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
    return (counter->ms > 0)?false:true;
}

//=============================================================================
//
void  WaitMs(UINT32 ms)
{
    OSTimeDlyHMSM(0,0,ms/1000,ms%1000);
}

void DataBusMode(GPIOMODE mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12 | GPIO_Pin_11| GPIO_Pin_10| GPIO_Pin_9| GPIO_Pin_8;
    if(GPIO_IN_MODE == mode) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3 | GPIO_Pin_2| GPIO_Pin_1| GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void TIM4_IRQHandler (void) 
{ 
    // check interrupt source
    if( TIM_GetITStatus(TIM4 , TIM_IT_Update) != RESET ) 
    {
        TIM_ClearITPendingBit(TIM4 , TIM_FLAG_Update);
        gSysTickCounter++;
#ifdef TIMER_DEBUG
        printf("2:0x%.2x\n\r",TIM2->SR);
#endif
    }
}

/*
 *---------------------------------------------------------
 * Description:
 *   External Interrupt Handler
 *
 * Notes:
 *  EXTIO6 -- PB6
 *  EXTIO7 -- PB7  
 *
 * Edit Log:
 *  2011.08.06--CCZY--Create
 *---------------------------------------------------------
 */
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1<<6)) // EXTI6 interrupt pending?
    {
        //Interrupt Event Code
        EXTI->PR |= (1<<6);  // clear pending interrupt
    }
    else if(EXTI->PR & (1<<7)) //EXTI7 interrupt pending?
    {
        EXTI->PR |= (1<<7);
    }
}

void  App_TimeTickHook(void)
{
    IWDG_ReloadCounter();
}

