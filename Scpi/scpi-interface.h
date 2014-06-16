
#ifndef _SCPI_INTERFACE_
#define _SCPI_INTERFACE_
#include "datatype.h"

void InitSCPI(void);
int ScpiInput(const char * data, INT32 len);

#endif
