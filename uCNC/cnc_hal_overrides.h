// boardmap override dummy file

#ifndef CNC_HAL_OVERRIDES_H
#define CNC_HAL_OVERRIDES_H

#ifdef __cplusplus
extern "C"
{
#endif

	#define ENABLE_MAIN_LOOP_MODULES
	#define ENABLE_IO_MODULES
	#define ENABLE_PARSER_MODULES
	#define ENABLE_MOTION_CONTROL_MODULES
#define LOAD_MODULES_OVERRIDE() ({LOAD_MODULE(web_pendant);LOAD_MODULE(sd_card_v2);})

#ifdef __cplusplus
}
#endif

#endif