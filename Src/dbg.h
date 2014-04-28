#ifndef _DBG_H_
#define _DBG_H_

#include <stdio.h>


#define CMD_DEBUG     1
#define APP_DEBUG     1
#define DBG_UART      0
#define USB_DEBUG     0
#define TMC_DEBUG     1
#define TMC_CMD_DEBUG 1
#define TMC_CH_READ   1
#define TMC_ERROR_LOG 1

#define DEBUG_MSG(debug,format,args...) \
  do{ if((debug)&& 1) printf("%s:%d ->"format,__func__,__LINE__,##args); }while(0)

#endif
