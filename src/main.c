/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "main.h"

#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"

#include "menu/menu.h"
#include "stage.h"
#include "debug.h"

//Game loop
GameLoop gameloop;

//Error handler
char error_msg[0x200];

void ErrorLock(void)
{
	while (1)
	{
		FntPrint("A fatal error has occured\n~c700%s\n", error_msg);
		Gfx_Flip();
	}
}

//Memory heap
//#define MEM_STAT //This will enable the Mem_GetStat function which returns information about available memory in the heap

#define MEM_IMPLEMENTATION
#include "mem.h"
#undef MEM_IMPLEMENTATION

#ifndef PSXF_STDMEM
static u8 malloc_heap[0x1B0000];
#endif

//Game default options
void DefaultOptions(void)
{
	stage.ghost = true;
}

//Entry point
int main(int argc, char **argv)
{
	//Remember arguments
	my_argc = argc;
	my_argv = argv;
	
	//Initialize system
	PSX_Init();
	
	Mem_Init((void*)malloc_heap, sizeof(malloc_heap));
	
	IO_Init();
	Audio_Init();
	Gfx_Init();
	Pad_Init();
	
	Timer_Init();
	
	//Start game
	DefaultOptions();
	Font_LoadTextures();
	
	gameloop = GameLoop_Menu;
	Menu_Load(MenuPage_Opening);
	Debug_Load();
	
	//Game loop
	while (PSX_Running())
	{
		//Prepare frame
		Timer_Tick();
		Audio_ProcessXA();
		Pad_Update();
		
		#ifdef MEM_STAT
			//Memory stats
			size_t mem_used, mem_size, mem_max;
			Mem_GetStat(&mem_used, &mem_size, &mem_max);
			#ifndef MEM_BAR
				FntPrint("mem: %08X/%08X (max %08X)\n", mem_used, mem_size, mem_max);
			#endif
		#endif
		
		Debug_Tick();
		
		//Tick and draw game
		switch (gameloop)
		{
			case GameLoop_Menu:
				Menu_Tick();
				break;
			case GameLoop_Stage:
				Stage_Tick();
				break;
		}
		
		//Flip gfx buffers
		Gfx_Flip();
	}
	
	//Deinitialize system
	Pad_Quit();
	Gfx_Quit();
	Audio_Quit();
	IO_Quit();
	
	PSX_Quit();
	return 0;
}
