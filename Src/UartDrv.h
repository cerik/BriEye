//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================

#ifndef _UART_DRV_H
#define _UART_DRV_H

void InitUart(void);
void InitUartPart2(void);
void SerialFlush(void);
UINT8 SerialGetChar(void);
UINT8 SerialPutChar(UINT8 dat);
UINT16 SerialPutData(void *pMsg,UINT16 size);

#endif
