//=============================================================================
// File Description:
//  This chip is Bigendin
//  Lcd Operation Source File
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#include <stdio.h>
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "datatype.h"

static const UINT8 LCD_CMD_CLR[]= {0xFD,0x02,0x00,0x00,0xDD,0xCC,0xBB,0xAA};
static const UINT8 LCD_CMD_HSK[]= {0xFD,0x00,0x00,0x00,0xDD,0xCC,0xBB,0xAA};
static const UINT8 LCD_CMD_OPEN[]={0xFD,0x0A,0x00,0x00,0xDD,0xCC,0xBB,0xAA};

void LcdSendByte(UINT8 data)
{
    while (!(USART3->SR & USART_FLAG_TXE));
    //USART3->DR = (data & 0x1FF);
    USART_SendData(USART3,data);
}

UINT8 LcdReadByte(void)
{
    while (!(USART1->SR & USART_FLAG_RXNE));    
    return USART_ReceiveData(USART3);
}

BOOL LcdBusy(void)
{
    return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
}
void LcdCmd(const UINT8 *cmd,UINT8 len)
{
    UINT8 i;
    for(i=0;i<len;i++)
    {
        LcdSendByte(cmd[i]);
    }
}

void LcdClearScreen(void)
{
    LcdCmd(LCD_CMD_CLR,sizeof(LCD_CMD_CLR));
}

void LcdHandShake(void)
{
    LcdCmd(LCD_CMD_HSK,sizeof(LCD_CMD_HSK));
    printf("return Data:\n");
    printf("%02X %02X\n",LcdReadByte(),LcdReadByte());
}

void LcdOpenDisplay(void)
{
    LcdCmd(LCD_CMD_OPEN,sizeof(LCD_CMD_OPEN));
}
