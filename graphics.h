// NCurses graphics
// (c) 2020 by Stefan Roettger

#pragma once

#include <ncurses.h> // for NCurses

#include <math.h> // for math functions
#include <ctype.h> // for c character definitions
#include <string.h> // for c string definitions
#include <stdio.h> // for c std i/o functions
#include <stdlib.h> // for standard c functions
#include <stdbool.h> // for standard bool values
#include <unistd.h> // for posix c functions

// init ASCII GFX
// * setup the standard screen of NCurses
void init_gfx();

// exit ASCII GFX
// * close the standard screen of NCurses
void exit_gfx();

// set the drawing window
// * by default the standard screen is used
void set_drawing_window(WINDOW *w);

// draw a frame at the edges of the screen
void draw_frame();

// draw a sprite at barycenter position (x, y)
// * the sprite is made up from consecutive text lines separated by \n
// * if the sprite position is outside of the screen the text will wrap around
void draw_sprite(int y, int x,
                 const char *text);

// draw a screen area at top-left position (x, y) with size (sx, sy)
// * the data is made up from consecutive lines of characters
void draw_area(int y, int x,
               int sy, int sx,
               const int *data);

// draw a text string with grid font characters at top-left position (x, y)
// * the grid font needs to be initialized beforehand via init_grid_font()
void draw_grid_text(int y, int x,
                    const char *text);

// draw a line from position (x1, y1) to (x2, y2)
// * symmetrified Bresenham algorithm
// * "ch" is the character used to draw the line
void draw_line(int y1, int x1, int y2, int x2,
               int ch = -1);

// print the alternate ncurses charset in a table
void print_altcharset();

// check for repeating time period
// * "time" is the actual time
// * "period" is the repeating time period
// * return value is true for the first "percent" of each period
// * by default the return value is true for the first half of the period
bool is_repeating(float time, float period,
                  float percent = 50);

// random float number in the range [0,1[
float rnd();
