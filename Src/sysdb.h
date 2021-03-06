//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================/*
#ifndef _SYSDB_H_
#define _SYSDB_H_


/*
 ******************************************************************************************
 ******************************************************************************************
 *
 *   
 *
 ******************************************************************************************
 ******************************************************************************************
 */
void    SetMartix(UINT8 i,UINT8 j,UINT8 k,FLOAT32 value);
FLOAT32 GetMartix(UINT8 i,UINT8 j,UINT8 k);

void   LoadDefaultDb(void);
UINT16 ReadSysDb(void *pMsg);
UINT16 WriteSysDb(const void *const pMsg);

UINT8  LoadSysDb(void);
UINT8  SaveSysDb(void);

#endif /* _PARAM_H_ */
