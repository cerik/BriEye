//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================
#include <string.h>
#include <stdlib.h> 
#include <stdio.h> 
#include "database.h"
#include "flash.h"
#include "crc.h"
#include "cmd.h"

struct tagBrieyeArgs gt_ccd;
struct tagLinearArgs gt_linearg;
struct tagDarkCalibArgs gt_darkcalib={{0.0,0.0,0.0},{0.0,0.0,0.0}};

//=============================================================================
// Default parameter, all of these will be
// writen into ccd at start of system.
//=============================================================================


//=============================================================================
// Description : Initate all default parameter
// Return :
void load_default_param(void)
{
    UINT16 i,j;
    gt_ccd.version  = VERSION;
    gt_ccd.user_dat = 7;
    gt_ccd.sample_freq   = 9;
    gt_ccd.sample_number = 300;
    gt_ccd.sample_remove = 1;

    for(i=0;i<3;i++)
    {
        gt_ccd.rgb_matrix[7][0][i] = 0;//gt_ccd.dark_current[i] = 0;
        gt_linearg.dark_matrix[7][0][i] = 0;
    }

    for(i=0;i<3;i++)
    {
        for(j=0;j<3;j++)
        {
            if(i==j)
            {
                gt_ccd.rgb_matrix[7][i][j]=1;
            }
            else
            {
                gt_ccd.rgb_matrix[7][i][j]=0;
            }
        }  
    }
}

//=============================================================================
// Description:
//      Send Brieye Arguements to host.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.
UINT16 read_brieye_args(void *pMsg)
{
    UINT16 i,j,k,m;
    UINT8 *tmp;
    UINT8 *p=(UINT8*)pMsg;

    tmp = (UINT8*)&gt_ccd.user_dat;
    for(i=0;i<sizeof(gt_ccd.user_dat);i++) *p++ = tmp[i];
    
    /*发送"版本"*/
    tmp = (UINT8*)&gt_ccd.version;
    for(i=0;i<sizeof(gt_ccd.version);i++) *p++ = tmp[i];
    
    /*发送"采样频率"*/
    tmp = (UINT8*)&gt_ccd.sample_freq;
    for(i=0;i<sizeof(gt_ccd.sample_freq);i++) *p++ = tmp[i];
    
    /*发送"采样数目"*/
    tmp=(UINT8*)&gt_ccd.sample_number;
    for(i=0;i<sizeof(gt_ccd.sample_number);i++) *p++ = tmp[i];
    
    /*发送"减头去尾数"*/
    tmp=(UINT8*)&gt_ccd.sample_remove;
    for(i=0;i<sizeof(gt_ccd.sample_remove);i++) *p++ = tmp[i];
    
    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp = (UINT8*)&gt_ccd.rgb_matrix[i][j][k];
                for(m=0;m<sizeof(gt_ccd.rgb_matrix[0][0][0]);m++) *p++ = tmp[m];
            }
        }
    }
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Receive Brieye Arguements from host and update it to local database.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.

UINT16 write_brieye_args(const void *const pMsg)
{
    UINT16 i,j,k,m;
    UINT8 *tmp;
    UINT8 *p=(UINT8*)pMsg;
    
    tmp=(UINT8*)&gt_ccd.user_dat;
    for(j=sizeof(gt_ccd.user_dat); j>0; j--) *tmp++ = *p++;
    
    tmp=(UINT8*)&gt_ccd.version;
    for(j=sizeof(gt_ccd.version);j>0;j--) *tmp++ = *p++;
    
    gt_ccd.version %= 1000000;
    gt_ccd.version += (VERSION)*1000000;
    
    tmp=(UINT8*)&gt_ccd.sample_freq;
    for(j=sizeof(gt_ccd.sample_freq);j>0;j--) *tmp++ = *p++;
    
    tmp=(UINT8*)&gt_ccd.sample_number;
    for(j=sizeof(gt_ccd.sample_number);j>0;j--) *tmp++ = *p++;
    
    /*接收"减头去尾数"*/
    tmp=(UINT8*)&gt_ccd.sample_remove;
    for(j=sizeof(gt_ccd.sample_remove);j>0;j--) *tmp++ = *p++;

    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp=(UINT8*)&gt_ccd.rgb_matrix[i][j][k];
                for(m=sizeof(gt_ccd.rgb_matrix[0][0][0]);m>0;m--) *tmp++ = *p++;
            }
        }                                   
    }
    
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Send Dark Arguements to host.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.
UINT16 read_dark_args(void *pMsg)
{
    INT16 i,j,k,m;
    UINT8 *tmp;
    UINT8 *p = pMsg;
      
    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp=(UINT8*)&gt_linearg.dark_matrix[i][j][k];
                for(m=0;m<sizeof(gt_linearg.dark_matrix[0][0][0]);m++) *p++ = tmp[m];
            }
        }
    }
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Receive Dark Arguements from host to local database.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.
UINT16 write_dark_args(const void *const pMsg)
{
    INT16 i,j,k,m;
    UINT8 *tmp;
    UINT8 *p = (UINT8*)pMsg;
    
    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp = (UINT8*)&gt_linearg.dark_matrix[i][j][k];
                for(m=sizeof(gt_linearg.dark_matrix[0][0][0]);m>0;m--) *p++ = tmp[m];
            }
        }                                   
    }
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Load brieye parameters from flash into ram
// Return:
//      0 --- success loaded.
UINT8 load_brieye_db(void)
{
    vu32 src = BRIEYE_ARGS_ADDR;
    UINT32* dst = (UINT32*)(&gt_ccd);
    UINT32  i,j,len,dat;

    len = BRIEYE_ARGS_SIZE;
    for(i=0,j=0;i<len;i+=4,j++)
    {
        dat = Flash_Read_Word(src + i);
        *(vu32*)(dst+j) = dat;
    }
    return 0;
}

//=============================================================================
// Description:
//      Load linear parameters from flash into ram
// Return:
//      0 --- success loaded.
UINT8 load_dark_db(void)
{
    volatile vu32 src = LINEAR_ARGS_ADDR;
    volatile UINT32* dst = (UINT32*)(&gt_linearg);
    UINT32  i,j,len,dat;

    len = LINEAR_ARGS_SIZE;
    for(i=0,j=0;i<len;i+=4,j++)
    {
        dat = Flash_Read_Word(src + i);
        *(vu32*)(dst+j) = dat;
    }
    return 0;
}

//=============================================================================
// Description:
//      Save brieye parameters from flash into ram
// Return:
//      0 --- save success.
//      1 --- save failed.
UINT8 save_brieye_db(void)
{
    vu32 dst = BRIEYE_ARGS_ADDR;
    const UINT32* src = (UINT32* )(&gt_ccd);
    volatile FLASH_Status FLASHStatus;

    FLASHStatus=Flash_Write_Word(dst, src, BRIEYE_ARGS_SIZE/4,true);
    
    if(FLASHStatus == FLASH_COMPLETE)
        return 0;
    else
        return 1;

}

//=============================================================================
// Description:
//      Save linear parameters from flash into ram
// Return:
//      0 --- save success.
//      1 --- save failed.
UINT8 save_dark_db(void)
{
    vu32 dst = LINEAR_ARGS_ADDR;
    const UINT32* src = (UINT32* )(&gt_linearg);
    FLASH_Status FLASHStatus;
   
    FLASHStatus=Flash_Write_Word(dst, src, LINEAR_ARGS_SIZE/4,true);
    
    if(FLASHStatus == FLASH_COMPLETE)
      return 0; /* ok */
    else
      return 1; /* error */
}

