/*-
 * Copyright (c) 2012-2013 Jan Breuer,
 *
 * All Rights Reserved
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   scpi-def.c
 * @date   Thu Nov 15 10:58:45 UTC 2012
 * 
 * @brief  SCPI Command Defination
 * 
 */
#include <stdio.h>
#include <string.h>
#include "datatype.h"
#include "parser.h"
#include "sysdb.h"
#include "utils.h"
#include "error.h"
#include "scpi.h"
#include "fifo.h"

#define SCPI_INPUT_BUFFER_LENGTH 100

static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
static scpi_reg_val_t scpi_regs[SCPI_REG_COUNT];

//======================================================================
//                                                                     =
// Interface Function Decleration                                      =
//                                                                     =
//======================================================================

static int32_t SCPI_Error(scpi_t * context, int_fast16_t err) {
     //char buf[8];
     //UINT8 len=longToStr(err,buf,8);
     //putnch(buf,len);
    printf("**ERROR: %d, \"%s\"\r\n",(INT32) err, SCPI_ErrorTranslate(err));
    return err;
}

//==================================
// Return The length have been send.
static int32_t SCPI_Write(scpi_t * context, const char * data, int32_t len) {
    //while(len--) putch(*data++);
    INT32 i=0;
    while(i<len)
    {
        printf("%c",*data++);
        i++;
    }
    return i;
}

static scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    if (SCPI_CTRL_SRQ == ctrl) {
        printf("**SRQ: 0x%X (%d)\r\n", val, val);
    } else {
        printf("**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }
    return SCPI_RES_OK;
}

static scpi_result_t SCPI_Flush(scpi_t * context) {
    return SCPI_RES_OK;
}

static scpi_result_t SCPI_Reset(scpi_t * context) {
    printf("**Reset\r\n");
    return SCPI_RES_OK;
}

static scpi_result_t SCPI_Test(scpi_t * context) {
    return SCPI_RES_OK;
}

//======================================================================
//                                                                     =
// Command Decleration                                                 =
//                                                                     =
//======================================================================


static scpi_result_t DMM_MatrixSave(scpi_t * context)
{
    SaveSysDb();
    return SCPI_RES_OK;
}

//===============================================================
// Configure system database
// normal_matrix[arg1][arg2][arg3]=arg4
// arg1 & arg2 is interger
// arg4 is float32.
static scpi_result_t CFG_MatrixQ(scpi_t *context)
{
    scpi_number_t param;
    UINT8 i,j,k;
    
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        i = param.value;
    }else{
        return SCPI_RES_ERR;
    }

    // read second paraeter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        j = param.value;
    }else{
        return SCPI_RES_ERR;
    }

    // read third paraeter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        k = param.value;
    }else{
        return SCPI_RES_ERR;
    }
    SCPI_ResultDouble(context,GetMartix(i,j,k));
    return SCPI_RES_OK;
}

static scpi_result_t CFG_Matrix(scpi_t *context)
{
    scpi_number_t param;
    FLOAT32 value;
    UINT8 i,j,k;
    
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        i = param.value;
    }else{
        return SCPI_RES_ERR;
    }

    // read second paraeter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        j = param.value;
    }else{
        return SCPI_RES_ERR;
    }

    // read third paraeter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        k = param.value;
    }else{
        return SCPI_RES_ERR;
    }

    // read fourth paraeter if present
    if (SCPI_ParamNumber(context, &param, FALSE)) {
        value = param.value;
    }else{
        return SCPI_RES_ERR;
    }
    SetMartix(i,j,k,value);
    printf("[%d][%d][%d]=%f\r\n",i,j,k,value);
    return SCPI_RES_OK;
}

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    {  "*CLS",SCPI_CoreCls},
    {  "*ESE",SCPI_CoreEse},
    { "*ESE?",SCPI_CoreEseQ},
    { "*ESR?",SCPI_CoreEsrQ},
    { "*IDN?",SCPI_CoreIdnQ},
    {  "*OPC",SCPI_CoreOpc},
    { "*OPC?",SCPI_CoreOpcQ},
    {  "*RST",SCPI_CoreRst},
    {  "*SRE",SCPI_CoreSre},
    { "*SRE?",SCPI_CoreSreQ},
    { "*STB?",SCPI_CoreStbQ},
    { "*TST?",SCPI_CoreTstQ},
    {  "*WAI",SCPI_CoreWai},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {      "SYSTem:ERRor?",SCPI_SystemErrorNextQ},
    { "SYSTem:ERRor:NEXT?",SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:COUNt?",SCPI_SystemErrorCountQ},
    {    "SYSTem:VERSion?",SCPI_SystemVersionQ},

    {       "STATus:QUEStionable?",SCPI_StatusQuestionableEventQ},
    { "STATus:QUEStionable:EVENt?",SCPI_StatusQuestionableEventQ},
    { "STATus:QUEStionable:ENABle",SCPI_StatusQuestionableEnable},
    {"STATus:QUEStionable:ENABle?",SCPI_StatusQuestionableEnableQ},
    {              "STATus:PRESet",SCPI_StatusPreset,},

    /* CONF */
    {         "CONFigure:MATrix",CFG_Matrix},
    {        "CONFigure:MATrix?",CFG_MatrixQ},
    {          "CONFigure:COUnt",0},
    {         "CONFigure:COUnt?",0},
    {      "CONFigure:FREQuency",0},
    {     "CONFigure:FREQuency?",0},
    {        "CONFigure:PRODuct",0},
    {       "CONFigure:PRODuct?",0},
    {           "CONFigure:DEte",0},
    {          "CONFigure:DEte?",0},
    {           "CONFigure:SAVe",DMM_MatrixSave},
    {                "SYStem:ID",0},
    {               "SYStem:ID?",0},
    {          "SYStem:VERsion?",0},

    /* MEAS */
  //{"MEASure:VOLTage:DC?",MEAS_MeasureVoltageDcQ,},
  //{       "MEASure:RAw?",MEAS_Raw,},
  //{       "MEASure:RGB?",MEAS_MeasureRgbReq,},
  //{       "MEASure:REG?",MEAS_MeasureRegReq,},
  //{     "MEASure:FLIck?",MEAS_FlickReq,},
  //{"MEASure:VOLTage:DC:RATio?",SCPI_StubQ,},
  //{      "MEASure:VOLTage:AC?",SCPI_StubQ,},
  //{      "MEASure:CURRent:DC?",SCPI_StubQ,},
  //{      "MEASure:CURRent:AC?",SCPI_StubQ,},
  //{      "MEASure:RESistance?",SCPI_StubQ,},
  //{     "MEASure:FRESistance?",SCPI_StubQ,},
  //{       "MEASure:FREQuency?",SCPI_StubQ,},
  //{          "MEASure:PERiod?",SCPI_StubQ,},
    SCPI_CMD_LIST_END
};

static scpi_interface_t scpi_interface = {
    SCPI_Error,
    SCPI_Write,
    SCPI_Control,
    SCPI_Flush,
    SCPI_Reset,
    SCPI_Test
};

static scpi_t scpi_context = {
    scpi_commands,               //cmdlist
    {//buffer
        SCPI_INPUT_BUFFER_LENGTH,//length
        0,                       //position
        scpi_input_buffer        //*data
    },
    {0,0,0},                     //paramlist
    &scpi_interface,             //interface
    0,                           //output_count
    0,                           //input_count
    FALSE,                       //cmd_error
    0,                           //error_queue
    scpi_regs,                   //registers
    scpi_units_def,              //units
    scpi_special_numbers_def,    //special_numbers
    0                            //user_context
};

int ScpiInput(const char * data, INT32 len)
{
    return SCPI_Input(&scpi_context, data, len); 
}

void InitSCPI(void)
{
    SCPI_Init(&scpi_context);
}
