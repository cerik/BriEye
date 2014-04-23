//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
void InitUart1(void);
void SerialFlush(void);
UINT8 SerialGetChar(void);
UINT8 SerialPutChar(UINT8 dat);
UINT16 SerialPutData(void *pMsg,UINT16 size);
