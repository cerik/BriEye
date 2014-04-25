//=============================================================================
// File Description:
//  This chip is Bigendin
//=============================================================================
// Create by Cerik
//=============================================================================
// Log:
//=============================================================================
#include "platform.h" 
#include "ucos_ii.h"
#include "uartdrv.h"
#include "MsgFifo.h"
#include "sysdb.h" 
#include "dbg.h"
#include "cpu.h"
#include "crc.h"
#include "cmd.h"

//========================================================================
//        External Function definition
//
extern void USB_Init(void);
extern void USB_Connect (BOOL con); 

//========================================================================
//       Local Global variable defination
static OS_STK App_TaskStk[4][APP_TASK_STK_SIZE];

//========================================================================
//       Global variable defination
tatDevStatus gDevStatus;
msg_fifo_t   gMsgFifo;
OS_EVENT    *gFlickTskMailbox; 
OS_EVENT    *gUsbCmdMailbox;

//========================================================================
//       Local Function definition
//
static void App_UartCmdTask(void* p_arg);
static void App_UsbCmdTask(void* p_arg);
static void App_FlickTask(void *p_arg);


//========================================================================
//   MIAN ENTRY
int main(void)
{
    UINT8 os_err,err_code=0;

    InitGpio(); 
    InitUart();
    
    init_crcccitt_tab();
    InitCounterTimer();

    USB_Init();
    USB_Connect(TRUE);
    InitWatchDog();
    CPU_IntDis();         /* Disable all ints until we are ready to accept them.  */
    OSInit();             /* Initialize "uC/OS-II, The Real-Time Kernel".         */
    OS_CPU_SysTickInit(); /* Initialize the SysTick.   */
    
    gFlickTskMailbox = OSMboxCreate((void *) 0); 
    gUsbCmdMailbox   = OSMboxCreate((void *) 0);
    
    os_err = OSTaskCreate(App_UartCmdTask,0,&App_TaskStk[1][APP_TASK_STK_SIZE - 1],(INT8U) APP_TASK_UART_PRIO);
    if(OS_ERR_NONE != os_err)
    {
        err_code = 1;
        goto ERROR;
    }

    os_err = OSTaskCreate(App_UsbCmdTask,0,&App_TaskStk[2][APP_TASK_STK_SIZE - 1],(INT8U) APP_TASK_USB_PRIO);
    if(OS_ERR_NONE != os_err)
    {
        err_code = 2;
        goto ERROR;
    }

    os_err = OSTaskCreate(App_FlickTask,0,&App_TaskStk[3][APP_TASK_STK_SIZE - 1],(INT8U) APP_TASK_FLICK_PRIO);
    if(OS_ERR_NONE != os_err)
    {
        err_code = 3;
        goto ERROR;
    }
    InitUartPart2();
    
    OSStart();
ERROR:
    printf("error:%d,%d\r\n",os_err,err_code);
    while(1);
}

/*
*********************************************************************************************************
*                                          App_UartCmdTask()
*
* Description :Responsible for the uart message receive.
*
* Argument : p_arg       Argument passed to 'App_UartCmdTask()' by 'OSTaskCreate()'.
*
* Return   : none.
*
* Caller   : This is a task.
*
* Note     : none.
*********************************************************************************************************
*/
static void App_UartCmdTask(void* p_arg)
{
    while(1)
    {
        DEBUG_MSG(1,"%d\r\n",p_arg);
        printf("%c\r\n",SerialGetChar());
        //OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

/*
*********************************************************************************************************
*                                          App_UsbCmdTask()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument : p_arg       Argument passed to 'App_UsbCmdTask()' by 'OSTaskCreate()'.
*
* Return   : none.
*
* Caller   : This is a task.
*
* Note     : none.
*********************************************************************************************************
*/
static void App_UsbCmdTask(void* p_arg)
{
    //UINT8 err;
    
    while (TRUE)
    {
        OSTimeDlyHMSM(0, 0, 3, 0);
        DEBUG_MSG(1,"%d\r\n",1);
        //OSMboxPend(gFlickTskMailbox, 0, &err);
    }
}

/*
*********************************************************************************************************
*                                          App_FlickTask()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument : p_arg       Argument passed to 'App_FlickTask()' by 'OSTaskCreate()'.
*
* Return   : none.
*
* Caller   : This is a task.
*
* Note     : none.
*********************************************************************************************************
*/
static void App_FlickTask(void *p_arg)
{
    //UINT8  err;

    while(1)
    {
        //OSMboxPend(gFlickTskMailbox, 0, &err);
        DEBUG_MSG(CMD_DEBUG," \r\n");
        OSTimeDlyHMSM(0, 0, 5, 0);
    }
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/OS-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument : ptcb   is a pointer to the task control block of the task being created.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void App_TaskCreateHook(OS_TCB* ptcb)
{
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument : none.
*
* Note     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument : none.
*********************************************************************************************************
*/
void App_TaskStatHook(void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument : none.
*
* Note     : 1 Interrupts are disabled during this call.
*
*            2  It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument : ptcb    is a pointer to the TCB of the task being created.
*
* Note     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB* ptcb)
{
   (void) ptcb;
}
#endif

#endif
