#include "src/cnc.h"

int main(void)
{
    //initializes all systems
    cnc_init();

    for (;;)
    {
        cnc_run();
    }
}
