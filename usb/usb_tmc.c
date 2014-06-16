/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_endp.c
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Endpoint routines
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usb_conf.h"
#include "platform.h"
#include "ucos_ii.h"
#include "usb_lib.h"
#include "usb_tmc.h"
#include "dbg.h"

/* Private define ------------------------------------------------------------*/
extern OS_EVENT *gUsbCmdMailbox;

/* Private variables ---------------------------------------------------------*/


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


/*
 ***********************************************************************
 *flag the transfer status: 
 *   IDLE --- the first pkg should contains tmcHeader;    -> RUN
 *   RUN ---- pkg just data;                              -> DONE
 *   ERROR -- transfer has meet some error                -> IDLE
 *   DONE---- all data has been received of this transfer;-> IDLE
 ***********************************************************************
 */
typedef enum{
    TMC_IDLE=0,
    TMC_RUNNING=1,
}tagTmcStatus;

static tagTmcStatus  gRxState=TMC_IDLE;       // TMC bulk-out receive state

static tagTmcLayerInfo gTmcLayerInfo={
    {0},      //lastTmcBulkOutHeader
    {0},      //lastTmcBulkInHeader
    TERR_NONE,//tmcLastError
    0,        //rxCount
    0,        //txCount
    FALSE,    //rxFinished
    TRUE,     //txFinished
};

//===================================================================================
// TMC transfer layer buffer;
//  used to package the user data into several tmc pkg
//===================================================================================
static UINT8 tmcLayerTxBuffer[EP_MAXPKGSIZE]={0};
static UINT8 tmcLayerRxBuffer[USB_BUF_SIZE]={0};

//===================================================================================
// File Name    : protocol.c
// Author       : CCZY & LYS
// Version      : V6.1
// Date         : 11/15/2011
// Description  : Source file TMC protocol layer.
//===================================================================================
static void tmcMsgAnalize(void)
{
    if(TRUE == gTmcLayerInfo.rxFinished) //all data have been received.
    {
        switch(gTmcLayerInfo.lastTmcBulkOutHeader.bID)
        {
        case DEV_DEP_MSG_OUT: //This is the host Write operation,not read operation
            gTmcLayerInfo.txDatCount   = 0;
            OSMboxPost(gUsbCmdMailbox,(void*)0x01);//Host Write
            DEBUG_MSG(TMC_DEBUG,"\n");
            break;
        case REQUEST_DEV_DEP_MSG_IN: //This is the host read operation,not write operation.
            OSMboxPost(gUsbCmdMailbox,(void*)0x02);//Host Read
            memcpy((UINT8*)&gTmcLayerInfo.lastTmcBulkInHeader,
                   (const void *)&gTmcLayerInfo.lastTmcBulkOutHeader,TMC_HEADER_SIZE);
            DEBUG_MSG(TMC_DEBUG,"txDatCount:%d\n",gTmcLayerInfo.txDatCount);
            break;
        case VENDOR_SPECIFIC_OUT:
            break;
        default:
            gTmcLayerInfo.tmcLastError = TERR_MSGID;
            break;
        } 
    }
}

//===================================================================================
// Bulk-Out Endpoint : Host -> device
//
// process received package of BULK-OUT endpoint from host.
// ENDPOINT1;
//===================================================================================
void EP1_OUT_Callback(void)
{
    tagTmcBulkMsgHeader *tmcBulkOutHeaderPtr=0;

    switch(gRxState)
    {
    case TMC_IDLE:
        //New transfer, clear rxFinished flag.
        gTmcLayerInfo.rxFinished = FALSE;

        //Init the data count of the first transaction(not include header info), and save the newest pkg header.
        gTmcLayerInfo.rxDatCount = PMAToUserBufferCopy(tmcLayerRxBuffer,ENDP1_RXADDR,GetEPRxCount(ENDP1)) - TMC_HEADER_SIZE;
        tmcBulkOutHeaderPtr      = (tagTmcBulkMsgHeader*)tmcLayerRxBuffer;

        //Package Error Check
        if(tmcBulkOutHeaderPtr->bTag == gTmcLayerInfo.lastTmcBulkOutHeader.bTag)
        {
            gTmcLayerInfo.tmcLastError = TERR_TAG;
            goto L_END;
        }
        else if(tmcBulkOutHeaderPtr->bTag & tmcBulkOutHeaderPtr->bTagInverse)
        {
            gTmcLayerInfo.tmcLastError = TERR_TAGINV;
            goto L_END;
        }
        else
        {
            gTmcLayerInfo.tmcLastError = TERR_NONE;
        }

        //Update the last tmc msg header to the newest one.
        memcpy((UINT8*)&gTmcLayerInfo.lastTmcBulkOutHeader,tmcLayerRxBuffer,TMC_HEADER_SIZE);
        if(gTmcLayerInfo.rxDatCount < MAXHEADERDATASIZE || 
           tmcBulkOutHeaderPtr->transferSize <= MAXHEADERDATASIZE)
        {//This mean the data transmited in this transcation are all received.
            gTmcLayerInfo.rxFinished = TRUE;
        }
        else
        {
            gRxState = TMC_RUNNING;
        }
        DEBUG_MSG(TMC_DEBUG,"%d\n--------\n",gTmcLayerInfo.rxFinished);
        break;
    case TMC_RUNNING:
        //now, the package is the continue one of this transfer;
        if( (TMC_HEADER_SIZE + gTmcLayerInfo.rxDatCount + GetEPRxCount(ENDP1)) >= USB_BUF_SIZE )
        {//Overflow
            gTmcLayerInfo.tmcLastError = TERR_MSGOVERFLOW;
            gRxState      = TMC_IDLE;
            goto L_END;
        }
        else
        {
            //The continue transaction are all data package,not contain header info, so just add the length to rxDatCount.
            gTmcLayerInfo.rxDatCount += PMAToUserBufferCopy(tmcLayerRxBuffer + TMC_HEADER_SIZE + gTmcLayerInfo.rxDatCount,ENDP1_RXADDR,GetEPRxCount(ENDP1));
            if( gTmcLayerInfo.lastTmcBulkOutHeader.transferSize <= gTmcLayerInfo.rxDatCount )
            {//OK,data have been received finished.
                gTmcLayerInfo.rxFinished   = TRUE;
                gTmcLayerInfo.tmcLastError = TERR_NONE;
                gRxState      = TMC_IDLE;
            }
        }
        break;
    default:
        break;
    }
    if(TRUE == gTmcLayerInfo.rxFinished)
    {
        tmcMsgAnalize();
    }
L_END:
    SetEPRxStatus(ENDP1, EP_RX_VALID);
    DEBUG_MSG(TMC_DEBUG,"err:%d\n",gTmcLayerInfo.tmcLastError);
}

//===================================================================================
// Bulk-IN Endpoint
//
// PC的IN分组回调函数
// 在成功发给PC分组后回调该函数
//===================================================================================
void EP2_IN_Callback(void)
{
    gTmcLayerInfo.txFinished = TRUE;
}

//===================================================================================
// RESET Callback 
// This function will be called after usb connected successfully
//  with host.
// So, we need reset tmc state machine here.
//===================================================================================
void RESET_Callback(void)
{
    gTmcLayerInfo.txFinished = TRUE;
}

//===================================================================================
// Send data from user layer;
// User data will be divided into several part if 
// its length is bigger then the maxpacksize;
// And , only the first contains the tmcHeader information.
//
// bufLen:
//    [0 ~ 1024] byte
//===================================================================================
UINT32 tmcSendData(const void * const pBuffer,const UINT32 bufLen)
{
    UINT32 index,xmitLen,cpyIndex;
    UINT8  err_code;
    tagCounter counter;
    
    const UINT8 * const pSrc=(UINT8*)pBuffer;

    err_code = 0;
    if( 0 == bufLen )
    {
        err_code = 1;
        goto L_END;
    }
    
    //First package
    //Wait previous transmit finished.
    StartCounter(&counter,50);//50ms
    while(FALSE == gTmcLayerInfo.txFinished)
    {
        if(CounterArrived(&counter))
        {
            err_code = 2;
            goto L_TIMEOUT;
        }
    }
    gTmcLayerInfo.lastTmcBulkInHeader.dev_dep_msg_in.transferAttr = 1;  //last byte is the last byte of this transfer
    gTmcLayerInfo.lastTmcBulkInHeader.dev_dep_msg_in.reserved[0] = 0;
    gTmcLayerInfo.lastTmcBulkInHeader.dev_dep_msg_in.reserved[1] = 0;
    gTmcLayerInfo.lastTmcBulkInHeader.dev_dep_msg_in.reserved[2] = 0;

    //copy the bulk-in header into tx buffer.
    memcpy(tmcLayerTxBuffer,(const void *)&gTmcLayerInfo.lastTmcBulkInHeader,TMC_HEADER_SIZE);

    xmitLen =gTmcLayerInfo.lastTmcBulkOutHeader.transferSize;//host expected byte
    if(xmitLen > bufLen) xmitLen = bufLen;//this will let the host wait timeout.

    //copy user data into tx buffer.
    for(index=0,cpyIndex=0;(index < xmitLen) && (index < MAXHEADERDATASIZE);index++,cpyIndex++)
        tmcLayerTxBuffer[TMC_HEADER_SIZE + cpyIndex] = pSrc[cpyIndex]; 

    gTmcLayerInfo.txFinished = FALSE;
    UserToPMABufferCopy(tmcLayerTxBuffer,ENDP2,ENDP2_TXADDR,TMC_HEADER_SIZE + index);
    gTmcLayerInfo.txDatCount = cpyIndex;
    SetEPTxValid(ENDP2);

    //Send the remainned data.
    while(gTmcLayerInfo.txDatCount<xmitLen)
    {
        StartCounter(&counter,50);//50ms
        while(FALSE == gTmcLayerInfo.txFinished)
        {
            if(CounterArrived(&counter))
            {
                err_code = 3;
                goto L_TIMEOUT;
            }
        }
        for(index=0;index<EP_MAXPKGSIZE && cpyIndex<xmitLen;index++,cpyIndex++) tmcLayerTxBuffer[index] = pSrc[cpyIndex];
        gTmcLayerInfo.txFinished = FALSE;
        gTmcLayerInfo.txDatCount += UserToPMABufferCopy(tmcLayerTxBuffer,ENDP2,ENDP2_TXADDR,index);
        SetEPTxValid(ENDP2);
    }
    goto L_END;

L_TIMEOUT:
    DEBUG_MSG(TMC_DEBUG,"Timeout ID=%d\n",err_code);
L_END:
    DEBUG_MSG(TMC_DEBUG,"txCount:%d\n====\n",gTmcLayerInfo.txDatCount);
    return gTmcLayerInfo.txDatCount;
}


//===================================================================================
// Description:  Get user data from TMC layer.
//   
// Parameters:   buffer ----- where the user data will be stored.
// 
// Return:  received count
//===================================================================================
UINT32 tmcGetData( void *buffer, UINT32 bufSize)
{   
    UINT8 *pSrc;
    UINT32 datSize=0;
    
    //__disable_irq();//disable all interrupt 

    if( FALSE == tmcIsRxFinished())  goto L_ERROR;

    datSize = tmcRxDataSize();
    if(bufSize < datSize) goto L_ERROR;
    
    pSrc = (UINT8*)tmcRxDataBufPtr();
    memcpy(buffer,pSrc,datSize);

L_ERROR:
    tmcRxFinishedClear();
    //__enable_irq();//NVIC_RESETPRIMASK();//resume interrupt
    return datSize;
}

UINT8 tmcGetBulkOutTag(void)
{
    return gTmcLayerInfo.lastTmcBulkOutHeader.bTag;
}

UINT8 tmcGetBulkInTag(void)
{
    return gTmcLayerInfo.lastTmcBulkInHeader.bTag;
}

void tmcResetRxState(void)
{
    gRxState = TMC_IDLE; 
}

BOOL tmcIsRxFinished(void)
{
    return gTmcLayerInfo.rxFinished;
}
void tmcRxFinishedClear(void)
{
    gTmcLayerInfo.rxFinished=FALSE;
    gTmcLayerInfo.rxDatCount=0;
}

UINT32 tmcRxDataSize(void)
{
    //return  gTmcLayerInfo.lastTmcBulkOutHeader.transferSize;
    return gTmcLayerInfo.rxDatCount;
}
UINT32 tmcTxDataSize(void)
{
    return gTmcLayerInfo.txDatCount;
}

UINT32 tmcHostReqSize(void)
{
    return gTmcLayerInfo.lastTmcBulkInHeader.transferSize;
}

//===================================================================================
// Description: return the data field pointer of the received message from host.
//===================================================================================
void *tmcRxDataBufPtr(void)
{
    return (void *)(tmcLayerRxBuffer + TMC_HEADER_SIZE);
}

//===================================================================================
// Description: return the data field pointer of the xmited message to host.
//===================================================================================
void *tmcTxDataBufPtr(void)
{
    return (void*)(tmcLayerRxBuffer + TMC_HEADER_SIZE);
}

tagTmcError tmcLastError(void)
{
    return  gTmcLayerInfo.tmcLastError;
}

