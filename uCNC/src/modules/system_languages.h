/*
	Name: system_languages.h
	Description: System menus for displays for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-04-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SYSTEM_LANGUAGES_H
#define SYSTEM_LANGUAGES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define LANGUAGE_EN 1

#ifndef SYSTEM_LANGUAGE
#define SYSTEM_LANGUAGE LANGUAGE_EN
#endif

#if (SYSTEM_LANGUAGE == LANGUAGE_EN)
#include "language/language_en.h"
#endif

#ifdef __cplusplus
}
#endif

#endif