/**
  *  This file is part of the OpenCL-ConsLaws framework
  *  Copyright (C) 2014, 2015 Jens Kristoffer Reitan Markussen
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  */





/**
* C Wrapper
*/

#ifndef C_VISUALIZER_H
#define C_VISUALIZER_H

#ifdef __cplusplus
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstddef>

#else
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "stddef.h"
#endif

#include "c_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef PointerHandle Visualizer;
typedef PointerHandle Figure;

/**
* Initialize API
*/
int oclsVizInit(Framework framework, Visualizer* visualizer);

/**
 *
 */
int oclsVisPlot(Visualizer visualizer, Data data, Domain domain, Figure* figure);

/**
 *
 */
int oclsVisUpdate(Figure figure, Data data);

#ifdef __cplusplus
}
#endif

#endif
