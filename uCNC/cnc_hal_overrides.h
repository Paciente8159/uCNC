// boardmap override dummy file

#ifndef CNC_HAL_OVERRIDES_H
#define CNC_HAL_OVERRIDES_H

#ifdef __cplusplus
extern "C"
{
#endif

#undef TOOL_COUNT
#undef TOOL1
#undef TOOL2
#undef TOOL3
#undef TOOL4
#undef TOOL5
#undef TOOL6
#undef TOOL7
#undef TOOL8
#undef TOOL9
#undef TOOL10

#define ENABLE_RT_SYNC_MOTIONS
#define ENABLE_MAIN_LOOP_MODULES
#define ENABLE_IO_MODULES
#define ENABLE_PARSER_MODULES
#define ENABLE_MOTION_CONTROL_MODULES
#define ENABLE_LASER_PWM
#define ENABLE_LASER_PPI
#define ENABLE_PLASMA_THC
#define ENABLE_EMBROIDERY
#define ENABLE_ATC_HOOKS
#define ENABLE_ITP_FEED_TASK

#define TOOL_COUNT 10
#define TOOL1 embroidery_stepper
#define TOOL2 laser_ppi
#define LASER_PPI PWM0
#define TOOL3 laser_pwm
#define TOOL4 pen_servo
#define TOOL5 plasma_thc
#define TOOL6 spindle_besc
#define TOOL7 spindle_pwm
#define TOOL8 spindle_relay
#define TOOL9 vfd_modbus
#define TOOL10 vfd_pwm

#define LOAD_MODULES_OVERRIDE() ({LOAD_MODULE(atc);})

#ifdef __cplusplus
}
#endif

#endif
