//=============================================================================
// File Description:
//
//=============================================================================
// Log:
//=============================================================================/*
#ifndef _PARAM_H_
#define _PARAM_H_

#include "flash.h"

//Firmware Version
#define VERSION       660

//Init Scale Rator, it's value is 7 for this version.
//Program will change this rator automatically for the sample value;
#define START_SCALE (UINT8)0x07 

/*
 ******************************************************************************************
 ******************************************************************************************
 *
 *   parameters define
 *
 ******************************************************************************************
 ******************************************************************************************
 */
struct tagBrieyeArgs{
    FLOAT32 user_dat;      /*User defined date */
    UINT32  version;       /*Firmware Version+Calibration Data*/
    UINT16  sample_freq;
    UINT16  sample_number;
    UINT16  sample_remove;
    FLOAT32 rgb_matrix[8][4][3];
};

struct tagLinearArgs{
    FLOAT32 dark_matrix[8][4][3];
};

struct tagDarkCalibArgs{
    FLOAT32 offset[3];
    FLOAT32 ad[3];  
};

/*
 ******************************************************************************************
 ******************************************************************************************
 *
 *   
 *
 ******************************************************************************************
 ******************************************************************************************
 */
#define BRIEYE_ARGS_SIZE    sizeof(struct tagBrieyeArgs) /* total size: 808 */
#define LINEAR_ARGS_SIZE    sizeof(struct tagLinearArgs) /* total size: 768 */

#define LINEAR_ARGS_ADDR    (PARAM_FLASH_ADDR)
#define BRIEYE_ARGS_ADDR    (PARAM_FLASH_ADDR + FLASH_PAGE_SIZE) 

/*
 ******************************************************************************************
 ******************************************************************************************
 *
 ******************************************************************************************
 ******************************************************************************************
 */
extern  struct tagBrieyeArgs gt_ccd;  
extern  struct tagLinearArgs gt_linearg;
extern  struct tagDarkCalibArgs gt_darkcalib;

/*
 ******************************************************************************************
 ******************************************************************************************
 *
 *   
 *
 ******************************************************************************************
 ******************************************************************************************
 */
void   load_default_param(void);
UINT16 read_brieye_args(void *pMsg);
UINT16 read_dark_args(void *pMsg);
UINT16 write_brieye_args(const void *const pMsg);
UINT16 write_dark_args(const void *const pMsg);

UINT8  load_brieye_db(void);
UINT8  load_dark_db(void);
UINT8  save_brieye_db(void);
UINT8  save_dark_db(void);

#endif /* _PARAM_H_ */
