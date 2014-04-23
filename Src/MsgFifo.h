
/**
 * @file   scpi_fifo.h
 * @date   Thu Nov 15 10:58:45 UTC 2012
 * 
 * @brief  basic FIFO implementation
 * 
 * 
 */

#ifndef SCPI_FIFO_H
#define SCPI_FIFO_H

#include "datatype.h"

#ifdef  __cplusplus
extern "C" {
#endif

    
#define MSG_FIFO_SIZE 512

    struct _fifo_t {
        UINT16 wr;
        UINT16 rd;
        UINT16 size;
        UINT8  data[MSG_FIFO_SIZE];
    };
    typedef struct _fifo_t msg_fifo_t;

    void MsgFifoInit(msg_fifo_t * fifo);
    void MsgFifoClear(msg_fifo_t * fifo);
    BOOL MsgFifoAdd(msg_fifo_t * fifo, UINT8 value);
    BOOL MsgFifoRemove(msg_fifo_t * fifo, UINT8 * value);
    BOOL MsgFifoCount(msg_fifo_t * fifo, INT16 * value);
    void *MsgFifiBuf(msg_fifo_t *fifo);

#ifdef  __cplusplus
}
#endif


#endif  /* SCPI_FIFO_H */
