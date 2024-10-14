// boardmap override dummy file

#ifndef CNC_HAL_OVERRIDES_H
#define CNC_HAL_OVERRIDES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define LOAD_MODULES_OVERRIDE() ({LOAD_MODULE(o_codes);})

#ifdef __cplusplus
}
#endif

#endif