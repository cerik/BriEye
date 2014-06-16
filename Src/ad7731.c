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
#include "stm32f10x_spi.h"
#include "datatype.h"
#include "platform.h"
#include "ad7731.h"


UINT32 AdReadData(UINT8 addr,UINT8 length)
{
    UINT8 i,cmd;
    UINT32 data=0;

    cmd = 0x10 | (addr&0x7);
    
    AdCS(true);
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );//Wait SPI is Not bussy.
    while(SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);//Clear Rx Buffer
    SPI_I2S_SendData(SPI1,cmd);
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );//Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);//Clear Rx Buffer
    for(i=0;i<length;i++)
    {
        SPI_I2S_SendData(SPI1,0xFF);//Send Dummy to get Data
        while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );//Wait SPI is Not bussy.
        while(RESET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) );//Wait Rx Buffer Is not empty
        data = (data<<8) | SPI_I2S_ReceiveData(SPI1);
    }
    AdCS(false);

    return data;
}

void AdWriteData(UINT8 addr,UINT32 data,UINT8 length)
{
    INT8   i;
    UINT8  cmd;

    cmd = addr&0x7;
    
    AdCS(true);
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );//Wait SPI is Not bussy.
    while(SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);//Clear Rx Buffer
    SPI_I2S_SendData(SPI1,cmd);
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );//Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);//Clear Rx Buffer
    for(i=length-1;i>0;i++)
    {
        SPI_I2S_SendData(SPI1,(data>>(8*i))&0xFF );//Send Dummy to get Data
        while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );//Wait SPI is Not bussy.
        while(RESET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) );//Wait Rx Buffer Is not empty
        data = (data<<8) | SPI_I2S_ReceiveData(SPI1);
    }
    AdCS(false);

}

void AdTest(void)
{
#if 0
    UINT16 data;
    printf("SPI:%X,%X,%X\n",SPI1->CR1,SPI1->CR2,SPI1->SR);

    //Send 0x10
    AdCS(true);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Clear Rx Buffer
    while(SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);
    SPI_I2S_SendData(SPI1,0x10);//SpiSendByte(0x10);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Clear Rx Buffer
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);
    //Send Dummy to get Data
    SPI_I2S_SendData(SPI1,0xFF);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Wait Rx Buffer Is not empty
    while(RESET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) );
    data = SPI_I2S_ReceiveData(SPI1);
    AdCS(false);
 
    printf("Status:0x%X\n",data);
    WaitMs(1);
    //=============================================================
    //Send 0x10
    AdCS(true);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Clear Rx Buffer
    while(SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);
    SPI_I2S_SendData(SPI1,0x12);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Clear Rx Buffer
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) ) data = SPI_I2S_ReceiveData(SPI1);

    //Read High Byte
    //Send Dummy to get Data
    SPI_I2S_SendData(SPI1,0xFF);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Wait Rx Buffer Is not empty
    while(RESET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) );
    data = SPI_I2S_ReceiveData(SPI1);

    //Read Low Byte
    //Send Dummy to get Data
    SPI_I2S_SendData(SPI1,0xFF);
    //Wait SPI is Not bussy.
    while( SET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) );
    //Wait Rx Buffer Is not empty
    while(RESET==SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) );
    data = (data<<8) | SPI_I2S_ReceiveData(SPI1);
    
    AdCS(false);
 
    printf("Mode:0x%X\n",data);
#else
    printf("Status: %X\n",AdReadData(0,1));
    printf("Mode  : %X\n",AdReadData(2,2));
    AdWriteData(2,0x175,2);
#endif
}

void AdCS(BOOL state)
{
    GPIO_WriteBit(GPIOC,GPIO_Pin_4,state?Bit_RESET:Bit_SET);
}

void AdReset(BOOL state)
{
    GPIO_WriteBit(GPIOA,GPIO_Pin_4,state?Bit_RESET:Bit_SET);
}
