//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================
#include "sysdb.h"

struct tagSysDb gSysDb;

//=============================================================================
// Description : Initate all default parameter
void LoadDefaultDb(void)
{
    UINT16 i,j;

    for(i=0;i<3;i++)
    {
        for(j=0;j<3;j++)
        {
            if(i==j)
            {
                gSysDb.normal_matrix[7][i][j]=1;
            }
            else
            {
                gSysDb.normal_matrix[7][i][j]=0;
            }
        }  
    }
}

//=============================================================================
// Description:
//      Send System Database to host.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.
UINT16 ReadSysDb(void *pMsg)
{
    UINT16 i,j,k,m;
    UINT8 *tmp;
    UINT8 *p=(UINT8*)pMsg;
    
    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp = (UINT8*)&gSysDb.normal_matrix[i][j][k];
                for(m=0;m<sizeof(gSysDb.normal_matrix[0][0][0]);m++) *p++ = tmp[m];
            }
        }
    }
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Receive System Database  from host and update it to local database.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.
UINT16 WriteSysDb(const void *const pMsg)
{
    UINT16 i,j,k,m;
    UINT8 *tmp;
    UINT8 *p=(UINT8*)pMsg;
    
    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp=(UINT8*)&gSysDb.normal_matrix[i][j][k];
                for(m=sizeof(gSysDb.normal_matrix[0][0][0]);m>0;m--) *tmp++ = *p++;
            }
        }                                   
    }
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Load System Database from flash into ram
// Return:
//      0 --- success loaded.
UINT8 LoadSysDb(void)
{
    VUINT32 src = SYS_DB_ADDR;
    UINT32 i,j,*dst = (UINT32*)(&gSysDb);

    for(i=0,j=0;i<SYS_DB_SIZE;i+=4,j++)
    {
        dst[j] = Flash_Read_Word(src + i);
    }
    return 0;
}

//=============================================================================
// Description:
//      Save System Database  from flash into ram
// Return:
//      0 --- save success.
//      1 --- save failed.
UINT8 SaveSysDb(void)
{
    VUINT32 dst = SYS_DB_ADDR;
    const UINT32* src = (UINT32* )(&gSysDb);
    FLASH_Status FLASHStatus;

    FLASHStatus=Flash_Write_Word(dst, src, SYS_DB_SIZE/4,true);
    return (FLASHStatus == FLASH_COMPLETE)?0:1;
}
