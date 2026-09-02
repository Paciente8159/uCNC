/*
	Name: cnc_build.h
	Description: Compile time configurations for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 30/01/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_BUILD_H
#define CNC_BUILD_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CNC_MAJOR_MINOR_VERSION "1.17"
#define CNC_PATCH_VERSION ".0"
// CNC version for printing
#define CNC_VERSION CNC_MAJOR_MINOR_VERSION CNC_PATCH_VERSION
// CNC module version (numeric version of the printing version). May change as new features or functions delarations are modified
#define UCNC_MODULE_VERSION 11700

#ifdef __cplusplus
}
#endif

#endif
