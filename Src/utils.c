//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#include "datatype.h"

UINT8 char2hex(UINT8 ch) 
{ 
    if((ch>='0')&&(ch<='9')) 
        return ch-'0'; 
    else if((ch>='A')&&(ch<='F')) 
        return ch-'A' + 0x0A; 
    else if((ch>='a')&&(ch<='f')) 
        return ch - 'a' + 0x0A; 
    else 
        return ch; 
} 
