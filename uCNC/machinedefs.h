#ifndef MACHINEDEFS_H
#define MACHINEDEFS_H

#include "config.h"

#ifndef MACHINE_KINEMATICS
#error Undefined kinematics
#endif

//define kynematics
#if (MACHINE_KINEMATICS == MACHINE_CARTESIAN_XYZ)
	#define AXIS_COUNT 3
	#define AXIS_X 0
	#define AXIS_Y 1
	#define AXIS_Z 2
	#define STEPPER_COUNT 3
#else
#error Kinematics not implemented
#endif

#endif
