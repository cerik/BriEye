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
#include "debug.h"

/* Private define ------------------------------------------------------------*/
extern OS_EVENT *gUsbCmdMailbox;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

tagTmcLayerInfo gTmcLayerInfo={
    {0},      //lastTmcBulkOutHeader
    {0},      //lastTmcBulkInHeader
    TMC_IDLE, //rxState
    TERR_NONE,//tmcLastError
    0,        //rxCount
    0,        //txCount
    true,     //rxFinished
    false,    //txFinished
};

/*
 *********************************************************
 *  TMC transfer layer buffer;
 *  used to package the user data into several tmc pkg
 *********************************************************
 */
static UINT8 tmcLayerTxBuffer[EP_MAXPKGSIZE]={0};
static UINT8 tmcLayerRxBuffer[USB_BUF_SIZE]={0};

/*
 ***************************************************************
 * Bulk-Out Endpoint : Host -> device
 *
 * process received package of BULK-OUT endpoint from host.
 * ENDPOINT1;
 ***************************************************************
 */
void EP1_OUT_Callback(void)
{
    tagTmcBulkOutHeader *tmcBulkOutHeaderPtr=0;

    switch(gTmcLayerInfo.rxState)
    {
    case TMC_IDLE:
        //New transfer, clear rxFinished flag.
        gTmcLayerInfo.rxFinished = false;

        //Init the data count of the first transaction(not include header info), and save the newest pkg header.
        gTmcLayerInfo.rxDatCount = (PMAToUserBufferCopy(tmcLayerRxBuffer,ENDP1_RXADDR,GetEPRxCount(ENDP1))-TMC_HEADER_SIZE);
        tmcBulkOutHeaderPtr      = (tagTmcBulkOutHeader*)tmcLayerRxBuffer;

        //Package Error Check
        if(tmcBulkOutHeaderPtr->bTag == gTmcLayerInfo.lastTmcBulkOutHeader.bTag)
        {
            gTmcLayerInfo.tmcLastError = TERR_TAG;
            goto TMC_ERROR;
        }
        else if(tmcBulkOutHeaderPtr->bTag & tmcBulkOutHeaderPtr->bTagInverse)
        {
            gTmcLayerInfo.tmcLastError = TERR_TAGINV;
            goto TMC_ERROR;
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
            gTmcLayerInfo.rxFinished = true;
            DEBUG_MSG(TMC_DEBUG,"Finished.\n--------\n");
        }
        else
        {
            gTmcLayerInfo.rxState = TMC_RUN;
            DEBUG_MSG(TMC_DEBUG,"\n");
        }
        break;
    case TMC_RUN:
        //now, the package is the continue one of this transfer;
        if( (TMC_HEADER_SIZE + gTmcLayerInfo.rxDatCount + GetEPRxCount(ENDP1)) >= USB_BUF_SIZE )
        {//Overflow
            DEBUG_MSG(TMC_DEBUG,"\n");
            gTmcLayerInfo.tmcLastError = TERR_MSGOVERFLOW;
            gTmcLayerInfo.rxState      = TMC_IDLE;
        }
        else
        {
            DEBUG_MSG(TMC_DEBUG,"\n");
            //The continue transaction are all data package,not contain header info, so just add the length to rxDatCount.
            gTmcLayerInfo.rxDatCount += PMAToUserBufferCopy(tmcLayerRxBuffer + TMC_HEADER_SIZE + gTmcLayerInfo.rxDatCount,ENDP1_RXADDR,GetEPRxCount(ENDP1));
            if( gTmcLayerInfo.lastTmcBulkOutHeader.transferSize <= gTmcLayerInfo.rxDatCount )
            {//OK,data have been received finished.
                gTmcLayerInfo.rxFinished   = true;
                gTmcLayerInfo.rxState      = TMC_IDLE;
                gTmcLayerInfo.tmcLastError = TERR_NONE;
            }
        }
        break;
    default:
        break;
    }
    if(tmcRxFinished())
    {
        tmcMsgAnalize();
    }
TMC_ERROR:
    SetEPRxStatus(ENDP1, EP_RX_VALID);
}

/*
 ***************************************************************
 * Bulk-IN Endpoint
 *
 * PC的IN分组回调函数
 * 在成功发给PC分组后回调该函数
 ***************************************************************
 */
void EP2_IN_Callback(void)
{
    gTmcLayerInfo.txFinished = true;
}

/*
 ***************************************************************
 * RESET Callback 
 * This function will be called after usb connected successfully
 *  with host.
 * So, we need reset tmc state machine here.
 ***************************************************************
 */
void RESET_Callback(void)
{
    gTmcLayerInfo.txFinished = true;
}

/*
 ***************************************************************
 * Send data from user layer;
 * User data will be divided into several part if 
 * its length is bigger then the maxpacksize;
 * And , only the first contains the tmcHeader information.
 *
 * bufLen:
 *    [0 ~ 1024] byte
 ***************************************************************
 */
UINT32 tmcSendDat(const void * const pBuffer,const UINT32 bufLen)
{
    UINT32 index,xmitLen,cpyIndex;
    UINT8  err_code;
    tagCounter counter;
    
    const UINT8 * const pSrc=(UINT8*)pBuffer;

    err_code = 0;
    if( 0 == bufLen )
    {
        err_code = 1;
        goto END;
    }
    
    //First package
    //Wait previous transmit finished.
    StartCounter(&counter,50);//50ms
    while(false == gTmcLayerInfo.txFinished)
    {
        if(CounterArrived(&counter))
        {
            err_code = 2;
            goto TIMEOUT;
        }
    }
    gTmcLayerInfo.lastTmcBulkInHeader.transferAttr = 1;  //last byte is the last byte of this transfer
    gTmcLayerInfo.lastTmcBulkInHeader.reserved1[0] = 0;
    gTmcLayerInfo.lastTmcBulkInHeader.reserved1[1] = 0;
    gTmcLayerInfo.lastTmcBulkInHeader.reserved1[2] = 0;

    //copy the bulk-in header into tx buffer.
    memcpy(tmcLayerTxBuffer,(const void *)&gTmcLayerInfo.lastTmcBulkInHeader,TMC_HEADER_SIZE);

    xmitLen =gTmcLayerInfo.lastTmcBulkOutHeader.transferSize;//host expected byte
    if(xmitLen > bufLen) xmitLen = bufLen;//this will let the host wait timeout.

    //copy user data into tx buffer.
    for(index=0,cpyIndex=0;(index < xmitLen) && (index < MAXHEADERDATASIZE);index++,cpyIndex++)
        tmcLayerTxBuffer[TMC_HEADER_SIZE + cpyIndex] = pSrc[cpyIndex]; 

    gTmcLayerInfo.txFinished = false;
    UserToPMABufferCopy(tmcLayerTxBuffer,ENDP2,ENDP2_TXADDR,TMC_HEADER_SIZE + index);
    gTmcLayerInfo.txDatCount = cpyIndex;
    SetEPTxValid(ENDP2);

    //Send the remained data.
    while(gTmcLayerInfo.txDatCount<xmitLen)
    {
        StartCounter(&counter,50);//50ms
        while(false == gTmcLayerInfo.txFinished)
        {
            if(CounterArrived(&counter))
            {
                err_code = 3;
                goto TIMEOUT;
            }
        }
        for(index=0;index<EP_MAXPKGSIZE && cpyIndex<xmitLen;index++,cpyIndex++) tmcLayerTxBuffer[index] = pSrc[cpyIndex];
        gTmcLayerInfo.txFinished = false;
        gTmcLayerInfo.txDatCount += UserToPMABufferCopy(tmcLayerTxBuffer,ENDP2,ENDP2_TXADDR,index);
        SetEPTxValid(ENDP2);
    }
    goto END;

TIMEOUT:
    DEBUG_MSG(TMC_DEBUG,"Timeout ID=%d\n",err_code);
END:
    DEBUG_MSG(TMC_DEBUG,"txCount:%d\n====\n",gTmcLayerInfo.txDatCount);
    return gTmcLayerInfo.txDatCount;
}

BOOL tmcRxFinished(void)
{
    return gTmcLayerInfo.rxFinished;
}
void tmcRxFinishedClear(void)
{
    gTmcLayerInfo.rxFinished=false;
    gTmcLayerInfo.rxDatCount=0;
}

tagTmcError tmcLastError(void)
{
    return  gTmcLayerInfo.tmcLastError;
}
UINT32 tmcRxDataSize(void)
{
    return  gTmcLayerInfo.lastTmcBulkOutHeader.transferSize;
}
void *tmcRxDatPtr(void)
{
    return (void *)(tmcLayerRxBuffer+TMC_HEADER_SIZE);
}
void *tmcGetLastBulkOutHeader(void)
{
    return  (void*)&gTmcLayerInfo.lastTmcBulkOutHeader;
}
void *tmcGetTxBufPtr(void)
{
    return (void*)tmcLayerRxBuffer;
}

/*
 **********************************************************
 * Description:
 *   Get user data from TMC layer.
 *   
 * Parameters:
 *   buffer ----- where the user data will be stored.
 * 
 * Return:
 *   received count
 **********************************************************
 */
UINT32 tmcGetDat( void *buffer, UINT32 bufSize)
{   
    UINT8 *pSrc;
    UINT32 datSize=0;
    
    __disable_irq();//disable all interrupt 

    if( false == tmcRxFinished())  goto ERROR;

    datSize = tmcRxDataSize();
    if(bufSize < datSize) goto ERROR;
    
    pSrc = (UINT8*)tmcRxDatPtr();
    memcpy(buffer,pSrc,datSize);

ERROR:
    tmcRxFinishedClear();
    __enable_irq();//NVIC_RESETPRIMASK();//resume interrupt
    return datSize;
}

/*
 ***************************************************** 
 * File Name    : protocol.c
 * Author       : CCZY & LYS
 * Version      : V6.1
 * Date         : 11/15/2011
 * Description  : Source file TMC protocol layer.
 *******************************************************
 */
void tmcMsgAnalize(void)
{
    if(true == gTmcLayerInfo.rxFinished) //all data have been received.
    {
        switch(gTmcLayerInfo.lastTmcBulkOutHeader.msgId)
        {
        case DEV_DEP_MSG_OUT: //This is the host Write operation,not read operation
            gTmcLayerInfo.txDatCount   = 0;
            //OSMboxPost(gUsbCmdMailbox,(void*)0x01);//Host Write
            DEBUG_MSG(TMC_DEBUG,"\n");
            break;
        case REQUEST_DEV_DEP_MSG_IN: //This is the host read operation,not write operation.
            //OSMboxPost(gUsbCmdMailbox,(void*)0x02);//Host Read
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

UINT32 tmcHostReqSize(void)
{
    return gTmcLayerInfo.lastTmcBulkInHeader.transferSize;
}
UINT32 tmcTxDatCount(void)
{
    return gTmcLayerInfo.txDatCount;
}

