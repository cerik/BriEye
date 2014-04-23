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
#include <string.h>
#include <stdio.h>
#include "platform.h"
#include "datatype.h"
#include "parser.h"
#include "utils.h"
#include "error.h"
#include "scpi.h"
#include "fifo.h"
#include "database.h"


#define SCPI_INPUT_BUFFER_LENGTH 100

static fifo_t fifo;
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

static int32_t SCPI_Write(scpi_t * context, const char * data, int32_t len) {
    while(len--) putch(*data++);
    return len;
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

static scpi_result_t DMM_MeasureVoltageDcQ(scpi_t * context) {
    scpi_number_t param1, param2;
    char bf[15];
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param1, FALSE)) {
        SCPI_NumberToStr(context, &param1, bf, 15);
        SCPI_ResultString(context,bf);
    }

    // read second paraeter if present
    if (SCPI_ParamNumber(context, &param2, FALSE)) {
        SCPI_NumberToStr(context, &param2, bf, 15);
        SCPI_ResultString(context,bf);
    }
    return SCPI_RES_OK;
}

//===================================================================
// Configure Command
//===================================================================
static scpi_result_t CFG_Frequency(scpi_t * context) {
    scpi_number_t param1;
    //char bf[15];
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param1, FALSE)) {
        gt_ccd.sample_freq = param1.value;
        //SCPI_NumberToStr(context, &param1, bf, 15);
        //SCPI_ResultString(context,bf);
        return SCPI_RES_OK;
    }
    else
    {
        return SCPI_RES_ERR;
    }
}
static scpi_result_t CFG_FrequencyQ(scpi_t * context) {
    char bf[15];
    scpi_number_t param1;
    param1.value = gt_ccd.sample_freq ;
    param1.type = SCPI_NUM_NUMBER;
    SCPI_NumberToStr(context, &param1, bf, 15);
    SCPI_ResultString(context,bf);
    return SCPI_RES_OK;
}

static scpi_result_t CFG_Count(scpi_t * context) {
    scpi_number_t param1;
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param1, FALSE)) {
        gt_ccd.sample_number = param1.value;
        return SCPI_RES_OK;
    }
    else
    {
        return SCPI_RES_ERR;
    }
}

static scpi_result_t CFG_CountQ(scpi_t * context) {
    char bf[15];
    scpi_number_t param1;
    param1.value = gt_ccd.sample_number ;
    param1.type = SCPI_NUM_NUMBER;
    SCPI_NumberToStr(context, &param1, bf, 15);
    SCPI_ResultString(context,bf);
    return SCPI_RES_OK;
}

static scpi_result_t CFG_Remove(scpi_t * context) {
    scpi_number_t param1;
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param1, FALSE)) {
        gt_ccd.sample_remove = param1.value;
        return SCPI_RES_OK;
    }
    else
    {
        return SCPI_RES_ERR;
    }
}

static scpi_result_t CFG_RemoveQ(scpi_t * context) {
    char bf[15];
    scpi_number_t param1;
    param1.value = gt_ccd.sample_remove ;
    param1.type = SCPI_NUM_NUMBER;
    SCPI_NumberToStr(context, &param1, bf, 15);
    SCPI_ResultString(context,bf);
    return SCPI_RES_OK;
}

//===================================================================
// Device Measure Command
//===================================================================
static scpi_result_t DMM_CalibDbX(scpi_t * context,INT8 which)
{
    scpi_number_t param1, param2;
    char bf[15];
    // read first parameter if present
    if (SCPI_ParamNumber(context, &param1, FALSE)) {
//        ABC[which][0]=param1.value;
        SCPI_NumberToStr(context, &param1, bf, 15);
        SCPI_ResultString(context,bf);
    }

    // read second paraeter if present
    if (SCPI_ParamNumber(context, &param2, FALSE)) {
//        ABC[which][1]=param2.value;
        SCPI_NumberToStr(context, &param2, bf, 15);
        SCPI_ResultString(context,bf);
    }
    return SCPI_RES_OK;
}

static scpi_result_t DMM_CalibDbA(scpi_t * context)
{
    return DMM_CalibDbX(context,0);
}
static scpi_result_t DMM_CalibDbB(scpi_t * context)
{
    return DMM_CalibDbX(context,1);
}
static scpi_result_t DMM_CalibDbC(scpi_t * context)
{
    return DMM_CalibDbX(context,2);
}

static scpi_result_t DMM_CalibDbXQ(scpi_t * context,INT8 which)
{ 
    char bf[15];
    // read first parameter if present
    SCPI_ResultString(context,bf);

    // read second paraeter if present
    SCPI_ResultString(context,bf);
    return SCPI_RES_OK;
}
static scpi_result_t DMM_CalibDbAQ(scpi_t * context)
{
    return DMM_CalibDbXQ(context,0);
}
static scpi_result_t DMM_CalibDbBQ(scpi_t * context)
{
    return DMM_CalibDbXQ(context,1);
}
static scpi_result_t DMM_CalibDbCQ(scpi_t * context)
{
    return DMM_CalibDbXQ(context,2);
}

static scpi_result_t DMM_MeasRawVolt(scpi_t * context)
{
    //StartConversion(TRUE);
    //while(!IsCovsCompleted());
//    SCPI_ResultDouble(context,gAdState.PvCalc[0]);
//    SCPI_ResultDouble(context,gAdState.PvCalc[1]);
    return SCPI_RES_OK;
}

static scpi_result_t DMM_MeasTemperature(scpi_t * context)
{
    FLOAT32 T[3]={0,0,0};

    //while(!IsCovsCompleted());
    
    SCPI_ResultDouble(context,T[0]);
    SCPI_ResultDouble(context,T[1]);
    SCPI_ResultDouble(context,T[2]);
    return SCPI_RES_OK;
}

static scpi_result_t DMM_CalibDbSave(scpi_t * context)
{
    //SaveDbToFlash();
    return SCPI_RES_OK;
}

bool IsCmdlineTerminate(const char * cmd, INT32 len)
{
    return (NULL==strnpbrk(cmd, len, "\r\n"))?FALSE:TRUE;
}

static const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    {  "*CLS",SCPI_CoreCls,},
    {  "*ESE",SCPI_CoreEse,},
    { "*ESE?",SCPI_CoreEseQ,},
    { "*ESR?",SCPI_CoreEsrQ,},
    { "*IDN?",SCPI_CoreIdnQ,},
    {  "*OPC",SCPI_CoreOpc,},
    { "*OPC?",SCPI_CoreOpcQ,},
    {  "*RST",SCPI_CoreRst,},
    {  "*SRE",SCPI_CoreSre,},
    { "*SRE?",SCPI_CoreSreQ,},
    { "*STB?",SCPI_CoreStbQ,},
    { "*TST?",SCPI_CoreTstQ,},
    {  "*WAI",SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {      "SYSTem:ERRor?",SCPI_SystemErrorNextQ,},
    { "SYSTem:ERRor:NEXT?",SCPI_SystemErrorNextQ,},
    {"SYSTem:ERRor:COUNt?",SCPI_SystemErrorCountQ,},
    {    "SYSTem:VERSion?",SCPI_SystemVersionQ,},

    {       "STATus:QUEStionable?",SCPI_StatusQuestionableEventQ,},
    { "STATus:QUEStionable:EVENt?",SCPI_StatusQuestionableEventQ,},
    { "STATus:QUEStionable:ENABle",SCPI_StatusQuestionableEnable,},
    {"STATus:QUEStionable:ENABle?",SCPI_StatusQuestionableEnableQ,},
    {              "STATus:PRESet",SCPI_StatusPreset,},

    {        "CONFigure:FREQuency",CFG_Frequency,},
    {       "CONFigure:FREQuency?",CFG_FrequencyQ,},
    {            "CONFigure:COUnt",CFG_Count,},
    {           "CONFigure:COUnt?",CFG_CountQ,},
    {           "CONFigure:REMOve",CFG_Remove,},
    {          "CONFigure:REMove?",CFG_RemoveQ,},
    //{        "CONFigure:RGBMATrix",CFG_RGBMatrix,},
    //{       "CONFigure:RGBMATrix?",CFG_RGBMatrixQ,},
    //{       "CONFigure:DARKMATrix",CFG_DarkMatrix,},
    //{      "CONFigure:DARKMATrix?",CFG_DarkMatrixQ,},
    //{           "CONFigure:OFFSet",CFG_DarkOffset,},
    //{          "CONFigure:OFFSet?",CFG_DarkOffsetQ,},
    //{              "CONFigure:ADc",CFG_Adc,},
    //{             "CONFigure:ADc?",CFG_AdcQ,},
    //{             "CONFigure:DOOr",CFG_Door,},
    //{             "CONFigure:SAVe",CFG_DBSave,},
    
    /* DMM */
    {"MEASure:VOLTage:DC?",DMM_MeasureVoltageDcQ,},
  //{       "MEASure:RAw?",DMM_Raw,},
  //{       "MEASure:RGB?",DMM_MeasureRgbReq,},
  //{       "MEASure:REG?",DMM_MeasureRegReq,},
  //{     "MEASure:FLIck?",DMM_FlickReq,},
  //{ "MEASure:FLIck:MAX?",DMM_FlickReq,},
  //{ "MEASure:FLIck:MIN?",DMM_FlickReq,},
    
  //{"MEASure:VOLTage:DC:RATio?",SCPI_StubQ,},
  //{      "MEASure:VOLTage:AC?",SCPI_StubQ,},
  //{      "MEASure:CURRent:DC?",SCPI_StubQ,},
  //{      "MEASure:CURRent:AC?",SCPI_StubQ,},
  //{      "MEASure:RESistance?",SCPI_StubQ,},
  //{     "MEASure:FRESistance?",SCPI_StubQ,},
  //{       "MEASure:FREQuency?",SCPI_StubQ,},
  //{          "MEASure:PERiod?",SCPI_StubQ,},
    {"MEASure:RAWVolt?",DMM_MeasRawVolt,},
    {"MEASure:TEMPerature?",DMM_MeasTemperature,},

    {"CALIBrate:DB:A",DMM_CalibDbA,},
    {"CALIBrate:DB:B",DMM_CalibDbB,},
    {"CALIBrate:DB:C",DMM_CalibDbC,},
    {"CALIBrate:DB:A?",DMM_CalibDbAQ,},
    {"CALIBrate:DB:B?",DMM_CalibDbBQ,},
    {"CALIBrate:DB:C?",DMM_CalibDbCQ,},
    {"CALIBrate:DB:SAVE",DMM_CalibDbSave,},
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
    fifo_init(&fifo);
    SCPI_Init(&scpi_context);
}

