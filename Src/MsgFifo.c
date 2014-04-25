
#include "MsgFifo.h"

void MsgFifoInit(msg_fifo_t * fifo) {
    fifo->wr = 0;
    fifo->rd = 0;
    fifo->size = MSG_FIFO_SIZE;
}

void MsgFifoClear(msg_fifo_t * fifo) {
    fifo->wr = 0;
    fifo->rd = 0;
}

BOOL MsgFifoAdd(msg_fifo_t * fifo, UINT8 value) {
    /* FIFO full? */
    if (fifo->wr == ((fifo->rd + fifo->size) % (fifo->size + 1))) {
        MsgFifoRemove(fifo, NULL);
    }

    fifo->data[fifo->wr] = value;
    fifo->wr = (fifo->wr + 1) % (fifo->size + 1);

    return TRUE;
}

BOOL MsgFifoRemove(msg_fifo_t * fifo, UINT8 * value) {
    /* FIFO empty? */
    if (fifo->wr == fifo->rd) {
        return FALSE;
    }

    if(value) {
        *value = fifo->data[fifo->rd];
    }

    fifo->rd = (fifo->rd + 1) % (fifo->size + 1);

    return TRUE;
}

BOOL MsgFifoCount(msg_fifo_t * fifo, INT16 * value) {
    *value = fifo->wr - fifo->rd;
    if (*value < 0) {
        *value += (fifo->size + 1);
    }
    return TRUE;
}

void *MsgFifoPtr(msg_fifo_t *fifo)
{
    return fifo->data;
}

