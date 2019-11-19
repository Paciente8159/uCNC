#include "settings.h"
#include "mcu.h"

SETTINGS g_settings;

void settings_load()
{
	//dummy
	g_settings.step_mm[0] = 10;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting X: %u steps/mm\n"), g_settings.step_mm[0]);
	#endif
	g_settings.step_mm[1] = 10;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting Y: %u steps/mm\n"), g_settings.step_mm[1]);
	#endif
	g_settings.step_mm[2] = 10;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting Z: %u steps/mm\n"), g_settings.step_mm[2]);
	#endif
	
	g_settings.max_speed[0] = 50;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting X speed: %f mm/s\n"), g_settings.max_speed[0]);
	#endif
	g_settings.max_speed[1] = 50;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting Y speed: %f mm/s\n"), g_settings.max_speed[1]);
	#endif
	g_settings.max_speed[2] = 50;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting Z speed: %f mm/s\n"), g_settings.max_speed[2]);
	#endif
	
	g_settings.max_accel[0] = 2;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting X accel: %f mm/s2\n"), g_settings.max_accel[0]);
	#endif
	g_settings.max_accel[1] = 2;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting Y accel: %f mm/s2\n"), g_settings.max_accel[1]);
	#endif
	g_settings.max_accel[2] = 2;
	#ifdef __DEBUG__
		mcu_printfp(PSTR("Setting Z accel: %f mm/s2\n"), g_settings.max_accel[2]);
	#endif
	
	g_settings.max_x = 1000.0f;
	g_settings.max_y = 1000.0f;
	g_settings.max_z = 1000.0f;
	
	g_settings.arc_tolerance = 0.05f;
	
}

void settings_reset()
{
}

void settings_save()
{	
}
