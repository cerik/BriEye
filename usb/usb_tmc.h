//====================================================================
// File Name    : usb_tmc.c
// Author       : Cerik
// Version      : V6.1
// Date         : 11/15/2011
// Description  : Header file TMC protocol layer.
//====================================================================

#ifndef __USB_REPORT_H
#define __USB_REPORT_H

#include "usb_type.h"
#include "usb_desc.h"
#include "usb_type.h"

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif


typedef enum {
    DEV_DEP_MSG_OUT=1,
    REQUEST_DEV_DEP_MSG_IN=2,
    VENDOR_SPECIFIC_OUT=126,
    REQQUEST_VENDOR_SPECIFIC_IN=127,
    VENDOR_SPECIFIC_IN=127
}TMC_MSG_ID;

//====================================================================
//  big endian
//====================================================================
typedef struct {  
    UINT8  bID;
    UINT8  bTag;
    UINT8  bTagInverse;
    UINT8  reserved;
    UINT32 transferSize;//not include header size;just all data of this transfer(this transfer maybe divided into many transactions.) 
    union {
        //for Bulk Out Package
        struct{
            UINT8  transferAttr;
            UINT8  reserved[3];
        }dev_dep_msg_out;

        struct{
            UINT8  reserved[4];
        }vendor_spec_out;

        //for Bulk In Package
        struct{
            UINT8  transferAttr;
            UINT8  reserved[3];
        }dev_dep_msg_in;
        
        struct{
            UINT8  transferAttr;
            UINT8  termChar;
            UINT8  reserved[2];
        }req_dev_dep_msg_in;

        struct{
            UINT8  reserved[4];
        }req_vendor_spec_in;

        struct{
            UINT8  reserved[4];
        }vendor_spec_in;
    };
}tagTmcBulkMsgHeader;

#define TMC_HEADER_SIZE    sizeof(tagTmcBulkMsgHeader) //12 Byte
#define USB_BUF_SIZE       1024

#define EP_MAXPKGSIZE      64 //Must be aligned with 4 byte
#define MAXHEADERDATASIZE  (EP_MAXPKGSIZE - TMC_HEADER_SIZE)

typedef enum{
    TERR_NONE=0,
    TERR_MSGID=1,
    TERR_TAG=2,
    TERR_TAGINV=3,
    TERR_LACKDAT=4,
    TERR_EMPTYPKG=5,
    TERR_MSGOVERFLOW=6
}tagTmcError;

typedef struct 
{
    tagTmcBulkMsgHeader lastTmcBulkOutHeader;
    tagTmcBulkMsgHeader lastTmcBulkInHeader;
    tagTmcError         tmcLastError;

    UINT32              rxDatCount;    // received byte count of one transfer,not include header info.
    UINT32              txDatCount;    // already xmited byte of bulk-in response,not include header info

    BOOL                rxFinished;    // wheaher the bulk-out msg have been received completelly.
    BOOL                txFinished;    // wheaher the respone msg have been send completely. 
}tagTmcLayerInfo;


void tmcResetRxState(void);
tagTmcError tmcLastError(void);
tagTmcBulkMsgHeader* tmcGetBulkOutHeader(void);
tagTmcBulkMsgHeader* tmcGetBulkInHeader(void);

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
