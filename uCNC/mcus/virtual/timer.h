/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2010, 2011, 2012 Teunis van Beelen
*
* teuniz@gmail.com
*
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
***************************************************************************
*
* This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*
***************************************************************************
*/
#include <stdio.h>

#ifdef __linux__

#include <sys/time.h>
#include <signal.h>

#else

/* this code only works on win2000, XP, Vista, 7 and up */
/* win95, win98 and ME are not supported                */
/* WINVER must have value 0x500 or higher               */
#ifndef WINVER
#define WINVER 0x500
#endif

#if WINVER < 0x500
#error "WINVER is < 0x500, cannot compile for old windows versions"
#endif

#include <windows.h>

#endif

#ifndef timer_INCLUDED
#define timer_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int start_timer(int, void (*)(void));

void stop_timer(void);

void startCycleCounter(void);
unsigned long stopCycleCounter(void);
unsigned long getCPUFreq(void);
unsigned long getTickCounter(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

