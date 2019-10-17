#ifndef UTILS_H
#define UTILS_H

#define SETBIT(x,y) (x |= (1<<y)) /* Set bit y in byte x*/
#define CLEARBIT(x,y) (x &= ~(1<<y)) /* Clear bit y in byte x*/
#define CHECKBIT(x,y) (x & (1<<y)) /* Check bit y in byte x*/

#define SETFLAG(x,y) (x |= (y)) /* Set bit y in byte x*/
#define CLEARFLAG(x,y) (x &= (~y)) /* Clear bit y in byte x*/
#define CHECKFLAG(x,y) (x & (y)) /* Check bit y in byte x*/

#define MAX(a,b) (a>b) ? a : b
#define MIN(a,b) (a<b) ? a : b
#define ABS(a) (a>0) ? a : -a

/*
//circular buffer useful macros
#define TYPEDEF_BUFFER(TYPE,SIZE) typedef struct {\
	uint8_t size;\
	uint8_t start;\
	uint8_t end;\
    TYPE buffer[SIZE];\
}TYPE##_BUFFER

#define BUFFER_INIT(BUF) BUF.start = 0; BUF.end = 0; memset(&BUF.buffer, 0, sizeof(BUF.buffer))

#define BUFFER_NEXT_INDEX(BUF, INDEX) ((++INDEX != BUF.size) ? INDEX : 0)
#define BUFFER_PREV_INDEX(BUF, INDEX) ((INDEX != 0) ? (INDEX - 1) : (BUF.size - 1))

#define IS_BUFFER_EMPTY(BUF) (BUF.start == BUF.end)
#define IS_BUFFER_FULL(BUF) (BUF.end + 1 == BUF.start || (BUF.end + 1 == BUF.size && BUF.start == 0))

#define BUFFER_READ_PTR(BUF) &BUF.buffer[BUF.start]
#define BUFFER_WRITE_PTR(BUF) &BUF.buffer[BUF.end]
#define BUFFER_PTR(BUF, INDEX) &BUF.buffer[INDEX]
#define BUFFER_WRITE_INDEX(BUF) BUF.end
#define BUFFER_READ_INDEX(BUF) BUF.start

#define BUFFER_WRITE_PTR_INC(BUF) if(++BUF.end == BUF.size) BUF.end = 0
#define BUFFER_READ_PTR_INC(BUF) if(++BUF.start == BUF.size) BUF.start = 0
*/
//used to in circular buffers
//struct must have pointers to self named prev and next

/*#define CIRC_BUFFER_INIT(NAME,SIZE) {\
	for(uint8_t i = 1; i < SIZE; i++)\
	{\
		NAME[i].prev = &NAME[i-1];\
		NAME[i-1].next = &NAME[i];\
	}\
	NAME[0].prev = &NAME[SIZE - 1];\
	NAME[SIZE - 1].next = &NAME[0];\
}*/

#endif
