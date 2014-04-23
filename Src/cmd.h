//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "datatype.h"

/*
 * ��FPGAͨ��Э�����I/O�ڵĶ���
 */
#define PIN_ADDR     GPIO_Pin_0
#define PIN_READ     GPIO_Pin_1
#define PIN_WRITE    GPIO_Pin_5
#define PIN_EMP      GPIO_Pin_6
#define PIN_FUL      GPIO_Pin_7

#define ERRORCOMMAND 255 //������������

struct tagCommand{
    UINT8 id;
    UINT8 argLen;
    UINT8 *name;
    UINT8 (*func_pre)(void*);
    UINT8 (*func_post)(void*);
};                          

#endif
