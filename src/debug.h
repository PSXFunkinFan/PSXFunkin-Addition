/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_DEBUG_H
#define PSXF_GUARD_DEBUG_H

#include "psx.h"
#include "fixed.h"

void Debug_Load(void);
void Debug_MoveDST(RECT_FIXED* dst, const char* name, u8 select, fixed_t camera_x, fixed_t camera_y, s32 increment);
void Debug_Tick(void);

#endif