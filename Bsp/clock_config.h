
/*
 ***********************************************************
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
 ***********************************************************
 */

//OTGFSPRE:1;PLLMUL:A;PPRE1:4;SW:2:PLL as system clock
//HSI=8MHz
//PLLCLK = 48MHz=HSI/2 * 12 =>> SYSCLK = 48MHz
#define __RCC_CFGR_VAL      0x682C0A
#define __HSI               8000000UL  //8MHz

//PLLSRC  = HSI/2
//PLLMUL  = PLLSRC * X --> PLLCLK
//SW        = PLLCLK
//SYSCLK = SW
#define CFGR_PLLMUL_MASK   (0xF << 18)
#define CFGR_SW_MASK       (0x3 << 0 )
#define CFGR_PLLXTPRE_MASK (0x1 << 17)
#define CFGR_PLLSRC_MASK   (0x1 << 16)
#define CFGR_HPRE_MASK     (0xF << 4 )
#define CFGR_PRE1_MASK     (0x7 << 8 )
#define CFGR_PRE2_MASK     (0x7 << 11)

#define __PLLMULL (((__RCC_CFGR_VAL & CFGR_PLLMUL_MASK) >> 18) + 2)  //=10+2=12 

#if   ((__RCC_CFGR_VAL & CFGR_SW_MASK) == 0x00) 
  #define __SYSCLK   __HSI                         // HSI is used as system clock
#elif ((__RCC_CFGR_VAL & CFGR_SW_MASK) == 0x01)
  #define __SYSCLK   __HSE                         // HSE is used as system clock
#elif ((__RCC_CFGR_VAL & CFGR_SW_MASK) == 0x02)    // PLL  is used as system clock
  #if (__RCC_CFGR_VAL & CFGR_PLLSRC_MASK)          // HSE is PLL clock source
    #if (__RCC_CFGR_VAL & CFGR_PLLXTPRE_MASK)      // HSE/2 is used
      #define __SYSCLK  ((__HSE >> 1) * __PLLMULL) // SYSCLK = HSE/2 * pllmull
    #else                                          // HSE is used
      #define __SYSCLK  ((__HSE >> 0) * __PLLMULL) // SYSCLK = HSE   * pllmul
    #endif
  #else                                            // HSI/2 is PLL clock source
    #define __SYSCLK  ((__HSI >> 1) * __PLLMULL)   // SYSCLK = HSI/2 * pllmul
  #endif
#else
   #error "ask for help"
#endif
   
/*----------------------------------------------------------------------------
 * Define  HCLK
 *
 * HCLK = SYSCLK = 48MHz
 *----------------------------------------------------------------------------
 */
#define __HCLKPRESC  ((__RCC_CFGR_VAL & CFGR_HPRE_MASK) >> 4)
#if (__HCLKPRESC & 0x08)
  #define __HCLK        (__SYSCLK >> ((__HCLKPRESC & 0x07)+1))
#else
  #define __HCLK        (__SYSCLK)
#endif

/*----------------------------------------------------------------------------
 * Define  PCLK1
 *
 * PCLK1 = HCLK/2 = 48MHz /2 = 24MHz
 *----------------------------------------------------------------------------
 */
#define __PCLK1PRESC  ((__RCC_CFGR_VAL & CFGR_PRE1_MASK) >> 8)//0x4
#if (__PCLK1PRESC & 0x04)
  #define __PCLK1       (__HCLK >> ((__PCLK1PRESC & 0x03)+1))
#else
  #define __PCLK1       (__HCLK)
#endif

/*----------------------------------------------------------------------------
 * Define  PCLK2
 *
 * PCLK2 = HCLK/4 = 48MHz/4=12MHz
 *----------------------------------------------------------------------------
 */
#define __PCLK2PRESC  ((__RCC_CFGR_VAL & CFGR_PRE2_MASK) >> 11)
#if (__PCLK2PRESC & 0x04)
  #define __PCLK2       (__HCLK >> ((__PCLK2PRESC & 0x03)+1))
#else
  #define __PCLK2       (__HCLK)
#endif

/*----------------------------------------------------------------------------
  * Define  Systick
  *
  * __SYSTICKCLK = HCLK/8 = 6MHz
  *  T = 100ms
  *----------------------------------------------------------------------------
  */

/*----------------------------------------------------------------------------
 * Define  TIM1CLK
 * T = 2* PCLK2 = 2* 12 = 24MHz
 *----------------------------------------------------------------------------
 */
#if (__PCLK2PRESC & 0x04)
  #define __TIM1CLK     (2*__PCLK2)
#else
  #define __TIM1CLK     (__PCLK2)
#endif


/*----------------------------------------------------------------------------
 * Define  TIMXCLK
 * T = 2* PCLK2 = 2* 12 = 24MHz
 *----------------------------------------------------------------------------
 */
#if (__PCLK1PRESC & 0x04)
  #define __TIMXCLK    (2*__PCLK1)
#else
  #define __TIMXCLK    (__PCLK1)
#endif

/*----------------------------------------------------------------------------
 *General purpose I/O Configuration
 *----------------------------------------------------------------------------
 */
#define __GPIO_SETUP              1
#define __GPIO_USED               0x0F
#define __GPIOA_CRL               0x33333333
#define __GPIOA_CRH               0x03000000
#define __GPIOB_CRL               0x33300033
#define __GPIOB_CRH               0x33333333
#define __GPIOC_CRL               0x00000000
#define __GPIOC_CRH               0x00403333
#define __GPIOD_CRL               0x00000033
#define __GPIOD_CRH               0x00000000
#define __GPIOE_CRL               0x00000000
#define __GPIOE_CRH               0x00000000

/*
 * Uart configuration
 */
#define __USART1_BAUDRATE         115200       //  4
#define __USART1_DATABITS         0x00000000
#define __USART1_STOPBITS         0x00000000
#define __USART1_PARITY           0x00000000
#define __USART1_FLOWCTRL         0x00000000
#define __USART1_REMAP            0x00000000

#define __USART1_CR1              0x00000020   //Inerrupt
#define __USART1_CR2              0x00000000
#define __USART1_CR3              0x00000000

//Define  Baudrate setting (BRR) for USART1 
#define __DIV(__PCLK, __BAUD)       ((__PCLK*25)/(4*__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

/*----------------------------------------------------------------------------
 * Independent watchdog configure
 *----------------------------------------------------------------------------
 */
#define __IWDG_PR      0x6            //clk = 40 kHz / 256 = 156.25 Hz
#define __IWGDCLK     (40000UL/(0x04<<__IWDG_PR))
#define __IWDG_RLR    (__IWDG_PERIOD*__IWGDCLK/1000000UL-1)

/*----------------------------------------------------------------------------
 * Define Timer PSC and ARR settings
 *----------------------------------------------------------------------------
 */
#define __VAL(__TIMCLK, __PERIOD) ((__TIMCLK/1000000UL)*__PERIOD)
#define __PSC(__TIMCLK, __PERIOD)  (((__VAL(__TIMCLK, __PERIOD)+49999UL)/50000UL) - 1)
#define __ARR(__TIMCLK, __PERIOD) ((__VAL(__TIMCLK, __PERIOD)/(__PSC(__TIMCLK, __PERIOD)+1)) - 1)

#define __TIM4_PERIOD             1000                   //1ms 
//#define __TIM4_PSC                0x5DBF                         // 
#define __TIM4_ARR                0xFFFF                 // 
#define __TIM4_CR1                0x0008 //one plus mode    // 
#define __TIM4_CR2                0x0000                 // 
#define __TIM4_SMCR               0x0000                 // 
#define __TIM4_CCMR1              0x0000                 // 
#define __TIM4_CCMR2              0x0000                 // 
#define __TIM4_CCER               0x0000                 // 
#define __TIM4_CCR1               0x0000                 // 
#define __TIM4_CCR2               0x0000                 // 
#define __TIM4_CCR3               0x0000                 // 
#define __TIM4_CCR4               0x0000                 // 
#define __TIM4_DIER               0x0000                 // 
