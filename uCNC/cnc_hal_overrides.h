//boardmap override dummy file

#ifndef CNC_HAL_OVERRIDES_H
#define CNC_HAL_OVERRIDES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ENABLE_IO_MODULES
#define ENABLE_MAIN_LOOP_MODULES
#define LOAD_MODULES_OVERRIDE() ({LOAD_MODULE(grblhal_keypad);})

#ifdef __cplusplus
}
#endif

#endif