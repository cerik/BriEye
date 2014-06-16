//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================
#ifndef _CMD_H_
#define _CMD_H_

#include "datatype.h"

struct tagCommand{
    UINT8 id;
    UINT8 argLen;
    UINT8 *name;
    UINT8 (*func_pre)(void*);
    UINT8 (*func_post)(void*);
};                          

#endif
