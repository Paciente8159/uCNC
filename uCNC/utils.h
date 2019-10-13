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


//circular buffer useful macros
#define TYPEDEF_BUFFER(TYPE,NAME,SIZE) \
typedef struct {\
	uint8_t size;\
	uint8_t start;\
	uint8_t end;\
    TYPE buffer[SIZE];\
}NAME

#define BUFFER_NEXT_INDEX(BUF) (((BUF.end + 1) != BUF.size) ? (BUF.end + 1) : 0
#define BUFFER_PREV_INDEX(BUF) (((BUF.end - 1) != 0) ? (BUF.end - 1) : 0

#define IS_BUFFER_EMPTY(BUF) (BUF.start == BUF.end)
#define IS_BUFFER_FULL(BUF) (BUF.end + 1 == start | (BUF.end + 1 == size && start == 0))

#define GET_READ_BUFFER_PTR(BUF) &BUF[BUF.start]
#define GET_WRITE_BUFFER_PTR(BUF) &BUF[BUF.end]

//used to in circular buffers
//struct must have pointers to self named prev and next

#define CIRC_BUFFER_INIT(NAME,SIZE) {\
	for(uint8_t i = 1; i < SIZE; i++)\
	{\
		NAME[i].prev = &NAME[i-1];\
		NAME[i-1].next = &NAME[i];\
	}\
	NAME[0].prev = &NAME[SIZE - 1];\
	NAME[SIZE - 1].next = &NAME[0];\
}
#define IS_BUFFER_EMPTY(NAME) (NAME ## _wr == NAME ## _rd)
#define IS_BUFFER_FULL(NAME) (NAME ## _wr->next == NAME ## _rd)

#endif
