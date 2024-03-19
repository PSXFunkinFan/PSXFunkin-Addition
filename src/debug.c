/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "debug.h"
#include "pad.h"
#include "gfx.h"
#include "font.h"

#define MAX_SELECT 32

static struct
{
	char* name[32];
	RECT_FIXED src[MAX_SELECT];
	RECT_FIXED og_src[MAX_SELECT];
	s32 increment[MAX_SELECT];
	u8 select;
	boolean enabled;
} debug;

void Debug_Load(void)
{
	for (int i = 0; i < MAX_SELECT; i++)
	{
		debug.enabled = false;
		debug.select = 0;
		debug.src[i].x = debug.src[i].y = debug.src[i].w = debug.src[i].h = 0;
		debug.og_src[i].x = debug.og_src[i].y = debug.og_src[i].w = debug.og_src[i].h = 0;
		debug.increment[i] = 0;;
	}
}

void Debug_MoveDST(RECT_FIXED* dst, const char* name, u8 select, fixed_t camera_x, fixed_t camera_y, s32 increment)
{
	dst->x += debug.src[select].x;
	dst->y += debug.src[select].y;
	dst->w += debug.src[select].w;
	dst->h += debug.src[select].h;
	
	debug.og_src[select].x = dst->x + camera_x;
	debug.og_src[select].y = dst->y + camera_y;
	debug.og_src[select].w = dst->w;
	debug.og_src[select].h = dst->h;
	
	debug.increment[select] = increment;
	
	sprintf(debug.name[select], name);
	
}

void Debug_Tick(void)
{
	if (pad_state.press & PAD_SELECT)
		debug.enabled ^= 1;
	
	if (!debug.enabled)
		return;
	
	font_vcr.draw(&font_vcr,
		"Welcome to Addition Debug Mode",
		0,
		8,
		FontAlign_Left
	);
	
	char rect_text[80];
	
	sprintf(rect_text, "X: %d, Y: %d W: %d H: %d", debug.og_src[debug.select].x, debug.og_src[debug.select].y, debug.og_src[debug.select].w, debug.og_src[debug.select].h);
	
	font_vcr.draw(&font_vcr,
		rect_text,
		SCREEN_WIDTH2 + 32,
		8,
		FontAlign_Left
	);
	
	if (pad_state.press & PAD_L1)
	{
		if (debug.select == 0)
			debug.select = MAX_SELECT;
		else
			debug.select--;
	}
	
	if (pad_state.press & PAD_R1)
	{
		if (debug.select == MAX_SELECT)
			debug.select = 0;
		else
			debug.select++;
	}
	
	if (pad_state.held & PAD_LEFT)
		debug.src[debug.select].x -= debug.increment[debug.select];
	
	if (pad_state.held & PAD_DOWN)
		debug.src[debug.select].y += debug.increment[debug.select];
	
	if (pad_state.held & PAD_UP)
		debug.src[debug.select].y -= debug.increment[debug.select];
	
	if (pad_state.held & PAD_RIGHT)
		debug.src[debug.select].x += debug.increment[debug.select];
}
