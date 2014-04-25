//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================
#include "sysdb.h"

//Please reference the Scatter file;
const struct tagSysDb gSysDbBlk __attribute__((section("SYSDB_SECTION"),used));

//=============================================================================
// Description:
//      Send System Database to host.
//      All parameters are stored in the buffer 'pMsg' with the type UINT8
// Return:
//      parameter size with byte unit.
UINT16 ReadSysDb(void *const pMsg)
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
                tmp = (UINT8*)&gSysDbBlk.normal_matrix[i][j][k];
                for(m=0;m<sizeof(gSysDbBlk.normal_matrix[0][0][0]);m++) *p++ = tmp[m];
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
    struct tagSysDb SysDb;
    
    for(i=0;i<8;i++)
    {
        for(j=0;j<4;j++)
        {
            for(k=0;k<3;k++)
            {
                tmp=(UINT8*)&SysDb.normal_matrix[i][j][k];
                for(m=sizeof(SysDb.normal_matrix[0][0][0]);m>0;m--) *tmp++ = *p++;
            }
        }                                   
    }
    SaveSysDb(&SysDb);
    return p - (UINT8*)pMsg;
}

//=============================================================================
// Description:
//      Load System Database from flash into ram
// Return:
//      0 --- success loaded.
UINT8 LoadSysDb(struct tagSysDb *const pSysDb)
{
    VUINT32 src = (UINT32)&gSysDbBlk;//SYS_DB_ADDR; Please reference the Scatter file;
    UINT32 i,j,*dst = (UINT32*)pSysDb;

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
UINT8 SaveSysDb(const struct tagSysDb * const pSysDb)
{
    VUINT32 dst = (UINT32)&gSysDbBlk;//SYS_DB_ADDR; Please reference the Scatter file;
    const UINT32* src = (UINT32* )(&pSysDb);
    FLASH_Status FLASHStatus;

    FLASHStatus=Flash_Write_Word(dst, src, SYS_DB_SIZE/4,true);
    return (FLASHStatus == FLASH_COMPLETE)?0:1;
}
