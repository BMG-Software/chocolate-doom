//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2020 B.M.Gooding
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	OpenGL version of the i_video.h file
//

#ifndef __I_GLVIDEO__
#define __I_GLVIDEO__

#include "doomtype.h" // Presume I'll be needing this??

#define SCREENWIDTH 320
#define SCREENHEIGHT 200

#define SCREENHEIGHT_4_3 240

// Here are the functions I think I'll need for now (pending further investigation)

void I_GLGetEvent();

/*
Called by D_DoomMain
Sets up all the OpenGL video stuff we need
*/
void I_GLInitGraphics();

/*
What thou createth, thou must destroyeth
*/
void I_GLShutdownGraphics();

/*
TODO: Check manual for function of this
*/
void I_GLUpdateNoBlit();

/*
Get everything we've done onto the screen
*/
void I_GLFinishUpdate();


void I_GLStartFrame();

void I_GLStartTic();



// Shall remove these as I find I don't need them
extern char *video_driver;
extern boolean screenvisible;

extern int vanilla_keyboard_mapping;
extern boolean screensaver_mode;
extern int usegamma;
// extern pixel_t *I_VideoBuffer;

extern int screen_width;
extern int screen_height;
extern int fullscreen;
extern int aspect_ratio_correct;
extern int integer_scaling;
extern int vga_porch_flash;
//extern int force_software_renderer;

extern char *window_position;
void I_GLGetWindowPosition(int *x, int *y, int w, int h);

// Joystic/gamepad hysteresis
extern unsigned int joywait;



#endif // __I_VIDEO__