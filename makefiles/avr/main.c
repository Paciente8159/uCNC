#include "cnc.h"

void main(void) __attribute__((noreturn));
void main(void)
{
    //initializes all systems
    cnc_init();

    for (;;)
    {
        cnc_run();
    }
}
