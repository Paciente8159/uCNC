#include "encoder.h"

static int32_t g_encoders_pos[ENCODERS];

static uint8_t read_encoder_pulses(void)
{
    uint8_t value = 0;
#if ENCODERS > 0
    value |= ((mcu_get_input(ENC0_PULSE)) ? ENC0_MASK : 0);
#endif
#if ENCODERS > 1
    value |= ((mcu_get_input(ENC1_PULSE)) ? ENC1_MASK : 0);
#endif
#if ENCODERS > 2
    value |= ((mcu_get_input(ENC2_PULSE)) ? ENC2_MASK : 0);
#endif
#if ENCODERS > 3
    value |= ((mcu_get_input(ENC3_PULSE)) ? ENC3_MASK : 0);
#endif
#if ENCODERS > 4
    value |= ((mcu_get_input(ENC4_PULSE)) ? ENC4_MASK : 0);
#endif
#if ENCODERS > 5
    value |= ((mcu_get_input(ENC5_PULSE)) ? ENC5_MASK : 0);
#endif
#if ENCODERS > 6
    value |= ((mcu_get_input(ENC6_PULSE)) ? ENC6_MASK : 0);
#endif
#if ENCODERS > 7
    value |= ((mcu_get_input(ENC7_PULSE)) ? ENC7_MASK : 0);
#endif
    return value;
}

void encoders_isr(void)
{
    static uint8_t last_pulse = 0;
    uint8_t pulse = read_encoder_pulses();
    uint8_t dir = read_encoder_dirs();
    uint8_t diff = last_pulse ^ pulse;

//checks if pulse pin changed state and is logical 1
#if ENCODERS > 0
    if ((diff & ENC0_MASK & pulse))
    {
        g_encoders_pos[0] += (dir & ENC0_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 1
    if ((diff & ENC1_MASK & pulse))
    {
        g_encoders_pos[1] += (dir & ENC1_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 2
    if ((diff & ENC2_MASK & pulse))
    {
        g_encoders_pos[2] += (dir & ENC2_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 3
    if ((diff & ENC3_MASK & pulse))
    {
        g_encoders_pos[3] += (dir & ENC3_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 4
    if ((diff & ENC4_MASK & pulse))
    {
        g_encoders_pos[4] += (dir & ENC4_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 5
    if ((diff & ENC5_MASK & pulse))
    {
        g_encoders_pos[5] += (dir & ENC5_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 6
    if ((diff & ENC6_MASK & pulse))
    {
        g_encoders_pos[6] += (dir & ENC6_MASK) : -1 : 1;
    }
#endif
#if ENCODERS > 7
    if ((diff & ENC7_MASK & pulse))
    {
        g_encoders_pos[7] += (dir & ENC7_MASK) : -1 : 1;
    }
#endif
}