//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "stm32f10x.h"

#define HIGH 0
#define LOW  1

/*
 * 与FPGA通信协议相关I/O口的定义
 */
#define PIN_ADDR     GPIO_Pin_0
#define PIN_READ     GPIO_Pin_1
#define PIN_WRITE    GPIO_Pin_5
#define PIN_EMP      GPIO_Pin_6
#define PIN_FUL      GPIO_Pin_7


#define MAX_CMD_LEN 30


/*
 * STM 向 FPGA发送的命令 
 *
 */
#define CMD_CFG_PARAM  0x1 /*[15:12]=b0001*/
#define CMD_START      0x2 /*[15:12]=b0010*/
#define CMD_RGB_ACC    0x3 /*[15:12]=b0011*/
#define CMD_RGB_RAW    0x4 /*[15:12]=b0100*/
#define CMD_TMP        0x5 /*[15:12]=b0101*/

/*
 * STM向PC&AMT&GMT发送数据前的第一个数据(字节)
 *
 */
#define CMD_PC_GET_PARAM    0x10 
#define CMD_PC_RGB          0x30 
#define CMD_GMT_AMT_RGB     0xE0

#define ERRORCOMMAND 255 //错误命令索引
#define ERRORARGLEN  100 //错误参数长度

#if 0

/*
 * Analyse command received from PC
 * and send relative data to PC
 *
 */
void uart_cmd_proc(void);
void usb_cmd_proc(void);
#endif
UINT8 CmdIndex(UINT8* cmd,UINT8 len);
UINT8 CmdArgLen(UINT8 index);
UINT8 CmdExecute(UINT8 cmd,void *pMsg);



/*
 ******************************************************************************************
 ******************************************************************************************
 *
 *   Control  door ...
 *
 ******************************************************************************************
 ******************************************************************************************
 */
#define SMALL 1
#define BIG   0
#define door_small_hole(void) do{GPIO_WriteBit(GPIOA,GPIO_Pin_14,Bit_SET);}while(0)  
#define door_big_hole(void)   do{GPIO_WriteBit(GPIOA,GPIO_Pin_14,Bit_RESET);}while(0)
#define door_hole_state(void) GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_14)

struct tagCommand{
    u8 id;
    u16 argLen;
    u8 *name;
    u8 (*func_pre)(void*);//命令处理函数
    u8 (*func_post)(void*);
};

extern struct tagCommand g_cmd[];
                               

#endif
