//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#include <stdio.h>
#include <stdio.h>
#include "misc.h"
#include "ucos_ii.h"
#include "datatype.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define RX_BUFFER_SIZE 20
#define TX_BUFFER_SIZE 20

struct {
    UINT8    RxBuffer[RX_BUFFER_SIZE];        //Rx Buffer
    UINT8    TxBuffer[TX_BUFFER_SIZE];        //Tx Buffer
    UINT8    RxBufferOutPtr;                  //Rx Buffer Out Pointer
    UINT8    RxBufferInPtr;                   //Rx Buffer In Pointer
    UINT8    RxBufferCnt;                     //Rx Buffer Counter
    UINT8    TxBufferOutPtr;                  //Tx Buffer Out Pointer
    UINT8    TxBufferInPtr;                   //Tx Buffer In  Pointer
    OS_EVENT *RxEventPtr;                     //Rx Event Pointer
    OS_EVENT *TxEventPtr;                     //Tx Event Pointer
}tagUartCtrl;

/*
 **********************************************************
 * Local Varibale & Macro Definition
 **********************************************************
 */
struct __FILE {
  int handle;                 // Add whatever you need here 
};
FILE __stdout;
FILE __stdin;


/*
 *---------------------------------------------------------
 * Description:
 *   Get a char from UART1
 * Return:
 *   one char UART1 received 
 * 
 * Note:
 *   RXNE will be auto cleared by software read to the UART_DR register.
 *
 * Edit Log:
 *    2011.08.06--CCZY--Create
 *---------------------------------------------------------
 */
UINT8 getch(BOOL block)
{
    while (!(USART1->SR & USART_FLAG_RXNE) && block);
    return (UINT8)(USART1->DR & 0x1FF);
}

/*
 *---------------------------------------------------------
 * Description:
 *
 * Edit Log:
 *    2011.08.06--CCZY--Create
 *---------------------------------------------------------
 */
void putch(UINT8 ch)
{
    while (!(USART1->SR & USART_FLAG_TXE));
    USART1->DR = (ch & 0x1FF);
}

int fputc(int ch, FILE *f) {
    putch(ch);
    return ch;
}
int fgetc(FILE *f) {
    return getch(true);
}
void _ttywrch(int ch) {
    putch(ch);
}
int ferror(FILE *f) {
    return EOF;
}
void _sys_exit(int return_code) {
label:  goto label;
}

//=============================================================================
//   UART1 Configure
//   Config:
//      115200 bps;
//      8 data bit;1 stop bit;No parity;No flow ctrl;
//=============================================================================
void InitUart(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 

    tagUartCtrl.RxEventPtr=OSSemCreate(0);
    tagUartCtrl.TxEventPtr=OSSemCreate(TX_BUFFER_SIZE-1);    //create the semaphore with the cnt value of TX_BUFFER_SIZE -1
    if(tagUartCtrl.RxEventPtr==(void *)0)  return;
    tagUartCtrl.RxBufferInPtr=0;
    tagUartCtrl.RxBufferOutPtr=0;
    tagUartCtrl.TxBufferInPtr=0;
    tagUartCtrl.TxBufferOutPtr=0;
    tagUartCtrl.RxBufferCnt=0;
    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // USART1_TX -> PA9 
    // USART1_RX -> PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure); 
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    USART_ClearFlag(USART1,USART_FLAG_TXE);
    USART_Cmd(USART1, ENABLE);
}

UINT8 SerialGetChar(void)
{
    UINT8 ErrMsg;
    UINT8 rxdata;

    OSSemPend(tagUartCtrl.RxEventPtr,0,&ErrMsg);
    rxdata=tagUartCtrl.RxBuffer[tagUartCtrl.RxBufferOutPtr];

    tagUartCtrl.RxBufferOutPtr = (tagUartCtrl.RxBufferOutPtr+1)%RX_BUFFER_SIZE;
    if(tagUartCtrl.RxBufferCnt > 0)
    {
        tagUartCtrl.RxBufferCnt--;
    }
    return (rxdata);
}

UINT8 SerialPutChar(UINT8 dat)
{
    UINT8 err;

    OSSemPend(tagUartCtrl.TxEventPtr,0,&err);
    if(err!=OS_NO_ERR)
    {
        return err;
    }
    tagUartCtrl.TxBuffer[tagUartCtrl.TxBufferInPtr]=dat;
    tagUartCtrl.TxBufferInPtr = (tagUartCtrl.TxBufferInPtr+1)%TX_BUFFER_SIZE;
    USART_SendData(USART1,dat);
    
    return OS_NO_ERR;
}

void SerialFlush(void)
{
    tagUartCtrl.RxBufferInPtr=0;
    tagUartCtrl.RxBufferOutPtr=0;
    tagUartCtrl.TxBufferInPtr=0;
    tagUartCtrl.TxBufferOutPtr=0;
    tagUartCtrl.RxBufferCnt=0;
}

UINT16 SerialPutData(const void *const pMsg,UINT16 size)
{
    UINT8 *p = (UINT8*)pMsg;
    for(;size>0;size++)
    {
        SerialPutChar(*p++);
    }
    return OS_NO_ERR;
}

//=============================================================================
// Description:
//   Uart1 Interrupt Handler
// Notes:
//  EXTIO6 -- PB6
//  EXTIO7 -- PB7  
//
// Edit Log:
//  2014.04.20--Cerik,Modified
//=============================================================================
void USART1_IRQHandler(void)
{
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
    {//RX Interrupt
        USART_ClearFlag(USART1,USART_FLAG_RXNE);
        if(tagUartCtrl.RxBufferCnt < RX_BUFFER_SIZE)
        {
            tagUartCtrl.RxBuffer[tagUartCtrl.RxBufferInPtr]=USART_ReceiveData(USART1);
            tagUartCtrl.RxBufferInPtr = (tagUartCtrl.RxBufferInPtr+1)%RX_BUFFER_SIZE;
            tagUartCtrl.RxBufferCnt++;
            OSSemPost(tagUartCtrl.RxEventPtr); //POST TO A SEMAPHORE
        }
    }
    if(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==SET)
    {//Tx Complete
        USART_ClearFlag(USART1,USART_FLAG_TXE);
        if(tagUartCtrl.TxBufferOutPtr != tagUartCtrl.TxBufferInPtr)
        {
            USART_SendData(USART1,tagUartCtrl.TxBuffer[tagUartCtrl.TxBufferOutPtr]);
            tagUartCtrl.TxBufferOutPtr = (tagUartCtrl.TxBufferOutPtr+1)%TX_BUFFER_SIZE;
            OSSemPost(tagUartCtrl.TxEventPtr);
        }
    }
}

