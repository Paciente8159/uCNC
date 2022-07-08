#ifdef ARDUINO_ARCH_LPC176X
#include <time.h>

extern "C"
{
    void lpc176x_delay_us(uint32_t delay)
    {
        LPC176x::delay_us(delay);
    }
}
#endif