//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================

#ifndef _MSP_TYPES_H_
#define _MSP_TYPES_H_

typedef         unsigned char  UINT8;
typedef           signed char  INT8;
typedef        unsigned short  UINT16;
typedef          signed short  INT16;
typedef          unsigned int  UINT32;
typedef            signed int  INT32;
typedef                 float  FLOAT32;
typedef                double  FLOAT64;
typedef    unsigned long long  UINT64;
typedef     signed long  long  INT64;

typedef volatile   signed int  VINT32;
typedef volatile unsigned int  VUINT32;

typedef enum {FALSE=0,TRUE=1} BOOL;
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

#define NaN32 0x7FFFFFFF
#define NaN16 0x7FFF

#define NULLCHAR  '\0'
#define NULL      0

#define MAXUINT8  0xFF
#define MAXUINT16 0xFFFF
#define MAXUINT32 0xFFFFFFFF


typedef struct {
    UINT8 lastUartCmd;
    UINT8 lastUsbCmd;
    union {
        UINT32 word;
        UINT32 bflick:1;//continue flick flag,1 = execute flick continue and report data to host.
        UINT32 bwave :1;//flick wave fliag,1= execute flick wave and report data to host.
    }devStatus;
}tatDevStatus;

typedef enum{GPIO_IN_MODE=0,GPIO_OUT_MODE=1}GPIOMODE;

typedef struct{
    INT32 ms;
    UINT32 t0,t1;
}tagCounter;

#ifdef  DEBUG
/*******************************************************************************
* Macro Name     : assert_param
* Description    : The assert_param macro is used for function's parameters check.
*                  It is used only if the library is compiled in DEBUG mode. 
* Input          : - expr: If expr is false, it calls assert_failed function
*                    which reports the name of the source file and the source
*                    line number of the call that failed. 
*                    If expr is true, it returns no value.
* Return         : None
*******************************************************************************/ 
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((UINT8 *)__FILE__, __LINE__))
__inline  void assert_failed(UINT8* file, UINT32 line);
#else
  #define assert_param(expr) ((void)0)
#endif

#endif
