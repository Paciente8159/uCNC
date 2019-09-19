#ifndef UTILS_H
#define UTILS_H

#define SETBIT(x,y) (x |= (y)) /* Set bit y in byte x*/
#define CLEARBIT(x,y) (x &= (~y)) /* Clear bit y in byte x*/
#define CHECKBIT(x,y) (x & (y)) /* Check bit y in byte x*/

#endif