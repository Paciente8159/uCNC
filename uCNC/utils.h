/*
	Name: utils.h - some usefull macro definitions
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Sep/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

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

#define MUL10(X) (((X<<2) + X)<<1)

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ && __SIZEOF_FLOAT__ == 4)
	#define fastsqrt(x) ({int32_t result = 0x1fbb4000 + (*(int32_t*)&x >> 1);*(float*)&result;})
#else
	#define fastsqrt(x) sqrtf(x)
#endif

#endif
