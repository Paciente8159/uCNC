#include "config.h"
#include "machinedefs.h"
#include "settings.h"

SETTINGS g_settings;

void settings_convert_min_to_s()
{
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		g_settings.max_speed[i] /= 60.0f;
		g_settings.max_accel[i] /= 60.0f;
	}
}

void settings_load()
{
	//dummy
	g_settings.step_mm[0] = 200;
	g_settings.step_mm[1] = 200;
	g_settings.step_mm[2] = 200;
	g_settings.max_speed[0] = 300;
	g_settings.max_speed[1] = 50;
	g_settings.max_speed[2] = 50;
	
	g_settings.max_accel[0] = 100;
	g_settings.max_accel[1] = 50;
	g_settings.max_accel[2] = 50;
	g_settings.max_x = 1000.0f;
	g_settings.max_y = 1000.0f;
	g_settings.max_z = 1000.0f;
	
	settings_convert_min_to_s();
	
}

void settings_reset()
{
}

void settings_save()
{	
}
