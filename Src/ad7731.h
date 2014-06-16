#ifndef __AD7731_H
#define __AD7731_H

#include "datatype.h"

#if 0
#define WEN 0x00

#define SigleW      0x00
#define SigleR      0x10
#define ContinueR   0x20
#define SContinueR  0x30

#define CR 0x00
#define SR 0x00
#define DR 0x01
#define MR 0x02
#define FR 0x03
#define NR 0x04
#define OR 0x05
#define GR 0x06
#define TR 0x07
#endif
void WriteToReg(UINT32 data, UINT8 num);
UINT32 ReadFromReg(UINT8 bytenumber);
UINT8 AD7731_Init(void);
UINT8 AD7731_SwitchChnl(UINT8 chnl);
UINT8 AD7731_ZeroCal(UINT8 chnl);
void AdTest(void);
void AdCS(BOOL state);
void AdReset(BOOL state);




#endif
