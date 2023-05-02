/*
	Name: system_menu_languages.h
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

#ifndef SYSTEM_MENU_LANGUAGES_H
#define SYSTEM_MENU_LANGUAGES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MENU_EN 1

#ifndef SYSTEM_MENU_LANGUAGE
#define SYSTEM_MENU_LANGUAGE MENU_EN
#endif

#if (SYSTEM_MENU_LANGUAGE == MENU_LANGUAGE_EN)

#ifdef __cplusplus
}
#endif

#endif