/*
    Name: pid.h
    Description: PID controller for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 07/03/2021

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef PID_H
#define PID_H

#ifdef __cplusplus
extern "C"
{
#endif

    void pid_init(void);

#ifdef __cplusplus
}
#endif

#endif