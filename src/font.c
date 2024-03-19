/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "font.h"

#include "timer.h"
#include "stage.h"

#include <string.h>

FontData font_bold, font_arial, font_vcr;
static Gfx_Tex font_tex1;

void Font_DrawTex(struct FontData* this, RECT* src, s32 x, s32 y, u8 r, u8 g, u8 b)
{
	if (this->zoom == NULL)
	{
		Gfx_BlitTexCol(this->tex, src, x, y, r, g, b);
	}
	else
	{
		RECT_FIXED dst = {x << FIXED_SHIFT, y << FIXED_SHIFT, src->w << FIXED_SHIFT, src->h << FIXED_SHIFT};
		Stage_DrawTexCol(this->tex, src, &dst, *this->zoom, r, g, b);
	}
}

//Font_Bold
u16 Font_Bold_GetWidth(struct FontData *this, const char *text)
{
	(void)this;
	return strlen(text) * 13;
}

void Font_Bold_DrawCol(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align, u8 r, u8 g, u8 b)
{
	//Offset position based off alignment
	switch (align)
	{
		case FontAlign_Left:
			break;
		case FontAlign_Center:
			x -= Font_Bold_GetWidth(this, text) / 2;
			break;
		case FontAlign_Right:
			x -= Font_Bold_GetWidth(this, text);
			break;
	}
	
	//Get animation offsets
	u8 v1 = (animf_count >> 1) & 1;
	
	//Draw string character by character
	u8 c;
	while ((c = *text++) != '\0')
	{
		u8 v0 = 0;
		
		if (v1)
				v0 ^= 1 * 14;
			
		//Draw character
		if ((c -= 'A') <= 'z' - 'A') //Lower-case will show inverted colours
		{
			RECT src = {((c % 8) * 28) + v0, (c / 8) * 16, 14, 16};
			Font_DrawTex(this, &src, x, y, r, g, b);
		}
		
		x += 13;
	}
}

//Font_Arial
#include "font_arialmap.h"

u16 Font_Arial_GetWidth(struct FontData *this, const char *text)
{
	(void)this;
	
	//Draw string width character by character
	s32 width = 0;
	
	u8 c;
	while ((c = *text++) != '\0')
	{
		//Shift and validate character
		if ((c -= 0x20) >= 0x60)
			continue;
		
		//Add width
		width += font_arialmap[c].gw;
	}
	
	return width;
}

void Font_Arial_DrawCol(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align, u8 r, u8 g, u8 b)
{
	//Offset position based off alignment
	switch (align)
	{
		case FontAlign_Left:
			break;
		case FontAlign_Center:
			x -= Font_Arial_GetWidth(this, text) / 2;
			break;
		case FontAlign_Right:
			x -= Font_Arial_GetWidth(this, text);
			break;
	}
	
	//Draw string character by character
	u8 c;
	while ((c = *text++) != '\0')
	{
		//Shift and validate character
		if ((c -= 0x20) >= 0x60)
			continue;
		
		//Draw character
		RECT src = {font_arialmap[c].ix, 129 + font_arialmap[c].iy, font_arialmap[c].iw, font_arialmap[c].ih};
		Font_DrawTex(this, &src, x, y, r, g, b);
		
		//Increment X
		x += font_arialmap[c].gw;
	}
}

//Font_VCR
#include "font_vcrmap.h"

u16 Font_VCR_GetWidth(struct FontData *this, const char *text)
{
	(void)this;
	return strlen(text) * 7;
}

void Font_VCR_DrawCol(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align, u8 r, u8 g, u8 b)
{
	//Offset position based off alignment
	switch (align)
	{
		case FontAlign_Left:
			break;
		case FontAlign_Center:
			x -= Font_VCR_GetWidth(this, text) >> 1;
			break;
		case FontAlign_Right:
			x -= Font_VCR_GetWidth(this, text);
			break;
	}
	
	//Draw string character by character
	u8 c;
	s16 oldx = x;
	
	while ((c = *text++) != '\0')
	{
		if (c == '\n')
		{
			x = oldx;
			y += 7;
		}

		//Draw character
		//Shift and validate character
		if ((c -= 0x20) >= 0x60)
			continue;

		RECT src = {font_vcrmap[c].charX, 196 + font_vcrmap[c].charY, font_vcrmap[c].charW, font_vcrmap[c].charL};
		
		Font_DrawTex(this, &src, x, y, r, g, b);

		//Increment X
		x += (font_vcrmap[c].charW - 1);
	}
}

//Common font functions
void Font_Draw(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align)
{
	this->draw_col(this, text, x, y, align, 0x80, 0x80, 0x80);
}

//Font functions
void Font_LoadTextures(void)
{
	Gfx_LoadTex(&font_tex1, IO_Read("\\FONT\\FONTS1.TIM;1"), GFX_LOADTEX_FREE);
	
}
void FontData_Load(FontData *this, Font font, fixed_t* zoom)
{
	//Load the given font
	switch (font)
	{
		case Font_Bold:
			//Load texture and set functions
			this->tex = &font_tex1;
			this->get_width = Font_Bold_GetWidth;
			this->draw_col = Font_Bold_DrawCol;
			break;
		case Font_Arial:
			//Load texture and set functions
			this->tex = &font_tex1;
			this->get_width = Font_Arial_GetWidth;
			this->draw_col = Font_Arial_DrawCol;
			break;
		case Font_VCR:
			//Load texture and set functions
			this->tex = &font_tex1;
			this->get_width = Font_VCR_GetWidth;
			this->draw_col = Font_VCR_DrawCol;
			break;
	}
	
	this->zoom = zoom;
	this->draw = Font_Draw;
}