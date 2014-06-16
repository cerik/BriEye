//====================================================================
// File Name    : usb_tmc.c
// Author       : Cerik
// Version      : V6.1
// Date         : 11/15/2011
// Description  : Header file TMC protocol layer.
//====================================================================

#ifndef _USB_TMC_H_
#define _USB_TMC_H_

#include "usb_desc.h"

#if defined ( __CC_ARM )
#pragma anon_unions
#endif

#define TMC_HEADER_SIZE    sizeof(tagTmcBulkMsgHeader) //Must 12 Byte
#define USB_BUF_SIZE       1024

#define EP_MAXPKGSIZE      64 //Must be aligned with 4 byte
#define MAXHEADERDATASIZE  (EP_MAXPKGSIZE - TMC_HEADER_SIZE)

void tmcResetRxState(void);
UINT8 tmcGetBulkOutTag(void);
UINT8 tmcGetBulkInTag(void);

/*
 **********************************************************
 * Description:
 *   To check whether the data receiving of TMC Layer 
 *    has been finished .
 * Return:
 *   TRUE --- finished
 *   FALSE -- unfinished
 **********************************************************
 */
BOOL tmcIsRxFinished(void);
void tmcRxFinishedClear(void);

/*
 **********************************************************
 * Description:
 *   send user data from TMC layer.
 *   
 * Parameters:
 *   buffer --- user data will be sended.
 *   datLen --- data length stored in buffer.
 * 
 * Return:
 *   transmited count;
 **********************************************************
 */
UINT32 tmcSendData(const void * const pBuffer,const UINT32 bufLen);

/*
 **********************************************************
 * Parameters:
 *   buffer --- place the user data will be stored;
 *   bufSize -- maxsize of buffer;
 *
 * Return:
 *   copyed data size;
 **********************************************************
 */
UINT32 tmcGetData( void *buffer, UINT32 bufSize);
UINT32 tmcHostReqSize(void);
UINT32 tmcRxDataSize(void);
UINT32 tmcTxDataSize(void);
void  *tmcTxDataBufPtr(void);
void  *tmcRxDataBufPtr(void);

#endif
