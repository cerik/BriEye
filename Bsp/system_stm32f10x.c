/**
  ******************************************************************************
  * @file    system_stm32f30x.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    04-September-2012
  * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File.
  *          This file contains the system clock configuration for STM32F30x devices,
  *          and is customized for use with STM32F3-DISCOVERY Kit.
  *          The STM32F30x is configured to run at 72 MHz, following the three
  *          configuration below:
  *            - PLL_SOURCE_HSI                : HSI (~8MHz) used to clock the PLL, and
  *                                              the PLL is used as system clock source.
  *            - PLL_SOURCE_HSE                : HSE (8MHz) used to clock the PLL, and
  *                                              the PLL is used as system clock source.
  *            - PLL_SOURCE_HSE_BYPASS(default): HSE bypassed with an external clock
  *                                              (8MHz, coming from ST-Link) used to clock
  *                                              the PLL, and the PLL is used as system
  *                                              clock source.
  *
  *
  * 1.  This file provides two functions and one global variable to be called from
  *     user application:
  *      - SystemInit(): Setups the system clock (System clock source, PLL Multiplier
  *                      and Divider factors, AHB/APBx prescalers and Flash settings),
  *                      depending on the configuration selected (see above).
  *                      This function is called at startup just after reset and
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32f30x.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick
  *                                  timer or configure other parameters.
  *
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  * 2. After each device reset the HSI (8 MHz Range) is used as system clock source.
  *    Then SystemInit() function is called, in "startup_stm32f30x.s" file, to
  *    configure the system clock before to branch to main program.
  *
  * 3. If the system clock source selected by user fails to startup, the SystemInit()
  *    function will do nothing and HSI still used as system clock source. User can
  *    add some code to deal with this issue inside the SetSysClock() function.
  *
  * 4. The default value of HSE crystal is set to 8MHz, refer to "HSE_VALUE" defined
  *    in "stm32f30x.h" file. When HSE is used as system clock source, directly or
  *    through PLL, and you are using different crystal you have to adapt the HSE
  *    value to your own configuration.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
#include "stm32f10x.h"

#define VECT_TAB_OFFSET  0x0 /*!< Vector Table base offset field.
                              This value must be a multiple of 0x200. */
#define PLL_SOURCE_HSI   1 // HSI (~8MHz) used to clock the PLL, and the PLL is used as system clock source

uint32_t SystemCoreClock = 48000000;

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};



/**
  * @brief  Configures the System clock source, PLL Multiplier and Divider factors,
  *         AHB/APBx prescalers and Flash settings
  * @note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
/*
 *  System Clock Configuration
 *
 * HSI = 8MHz
 * SYSCLK=PLLCLK = 48MHz = 8MHz/2 * 12
 * HCLK=48MHz
 * PCLK2(APB2)=12MHz
 * PCLK1(APB1)=24MHz
 * USBCLK = 48MHz
 * FCLK = 48MHz
 * SysTickCLK=HCLK/8=8MHz

 */
static void SetSysClock(void)
{
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/

    /* Enable Prefetch Buffer and set Flash Latency */
    FLASH->ACR = 0x00000012;//(uint32_t)(FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2);
#if 1
    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

    /* PCLK = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2 | (uint32_t)RCC_CFGR_PPRE2_DIV4;

    /* PLL configuration */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLMULL12);

    /* USB Pre*/
    RCC->CFGR |= (uint32_t)RCC_CFGR_USBPRE;

    /* MCO */
    RCC->CFGR &= ~(uint32_t)RCC_CFGR_MCO;

    /* Select PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_SW_1);

    /* Enable PLL */
    RCC->CR = (uint32_t)(RCC_CR_PLLON | 0x78 | RCC_CR_HSIRDY | RCC_CR_HSION);
#else
    RCC->CFGR  = 0x00682D02;                     // set clock configuration register
    RCC->CR    = 0x0100007B;                     // set clock control register
#endif
    while((RCC->CR & RCC_CR_HSIRDY) == 0); // Wait for HSIRDY = 1 (HSI is ready)
    while((RCC->CR & RCC_CR_PLLRDY) == 0); //Wait till PLL is ready
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL);
}

/*----------------------------------------------------------------------------
 STM32 System Timer setup.
 initializes the SysTick register
 *----------------------------------------------------------------------------*/
#define __SYSTICK_CTRL_VAL  0x00000002
#define __SYSTICK_PERIOD    0x00000001
#define __HCLK              48000000
#define __SYSTICKCLK        (__HCLK/8)
/*----------------------------------------------------------------------------
 Define  SYSTICKCLK
 *----------------------------------------------------------------------------*/
void stm32_SysTickSetup (void) 
{
#if ((__SYSTICK_PERIOD*(__SYSTICKCLK/1000)-1) > 0xFFFFFF)       // reload value to large
   #error "Reload Value to large! Please use 'HCLK/8' as System Timer clock source or smaller period"
#else
  SysTick->LOAD  = __SYSTICK_PERIOD*(__SYSTICKCLK/1000)-1;      // set reload register
  SysTick->CTRL  = __SYSTICK_CTRL_VAL;                          // set clock source and Interrupt enable

  SysTick->VAL   = 0;        // clear  the counter
  SysTick->CTRL |= 1;        // enable the counter
#endif
}

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface, the PLL and update the
  *         SystemFrequency variable.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
#if 1
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset CFGR register */
  RCC->CFGR = 0;//&= 0xF87FC00C;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
  //RCC->CFGR &= (uint32_t)0xFF80FFFF;
  
  /* Disable all interrupts */
  RCC->CIR = 0x00000000;
#endif
  /* Configure the System clock source, PLL Multiplier and Divider factors,
     AHB/APBx prescalers and Flash settings ----------------------------------*/
  SetSysClock();
  //stm32_SysTickSetup();
  
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
