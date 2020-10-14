// NCurses side-scrolling area
// (c) 2020 by Stefan Roettger

#pragma once

#include "graphics.h"

// set the drawing window
void set_drawing_window(WINDOW *w);

// set the size of the scrollable area
void set_area_size(int sx, int sy);

// get the width of the scrollable area
int get_area_width();

// get the height of the scrollable area
int get_area_height();

// set the size of the displayed window
void set_window_size(int sx, int sy);

// get the width of the displayed window
int get_window_width();

// get the height of the displayed window
int get_window_height();

// set the offset of the displayed window
void set_window_offset(int ox, int oy);

// clear the scrollable area
void clear_area(int ch = ' ');

// set the border of the scrollable area
void set_area_border(int ch);

// set the border of the displayed window
void set_window_border(int ch);

// get the cell character at position (x, y)
int get_cell(int x, int y);

// set the cell at position (x, y) to character ch
void set_cell(int x, int y, int ch);

// get the cell area at top-left position (x, y) with size (sx, sy)
int *get_cell_area(int x, int y,
                   int sx, int sy);

// fill a cell area at top-left position (x, y) with size (sx, sy)
void fill_cell_area(int x, int y,
                    int sx, int sy,
                    int ch);

// render a cell area at top-left position (x, y) with size (sx, sy)
void render_cell_area(int x, int y,
                      int sx, int sy,
                      const int *data);

// render a text area at top-left position (x, y)
void render_text_area(int x, int y,
                      const char *text);

// render a grid font character at top-left position (x, y)
void render_grid_char(int x, int y, int ch);

// render a text string with grid font characters at top-left position (x, y)
void render_grid_text(int x, int y,
                      const char *text);

// render a line from position (x1, y1) to (x2, y2)
void render_line(int x1, int y1, int x2, int y2,
                 int ch = -1);

// flood-fill a cell area starting at position (x, y)
void flood_fill(int x, int y, int ch);

// flood-fill everything but a cell area starting at position (x, y)
void inverse_flood_fill(int x, int y, int ch);

// enable a sprite overlay
// * "num" is the number of the sprite
// * "sx" and "sy" is the cell area size of the sprite
// * "window" determines if the sprite position is area or window relative
void enable_sprite(int num,
                   int sx, int sy,
                   bool window = false);

// clear a sprite
// * "num" is the number of the sprite
// * "ch" is the characted to be set
// * by default renders a sprite fully transparent
void clear_sprite(int num, int ch = -1);

// set the sprite data
void set_sprite_data(int num, int sx, int sy, const int *data);

// fill a sprite cell area
// * "num" is the number of the sprite
// * "x" and "y" is the top-left position (x, y) of the sub-area to be cleared
// * "sx" and "sy" is the size of the sub-area to be cleared
// * "ch" is the character to be copied into the sub-area
void fill_sprite_area(int num, int x, int y, int sx, int sy, int ch);

// set a sprite cell area
// * "num" is the number of the sprite
// * "x" and "y" is the top-left position (x, y) of the sub-area to be set
// * "sx" and "sy" is the size of the sub-area to be set
// * "data" is the data to be copied into the sub-area
// * data values of -1 are not copied having a transparency effect
void set_sprite_area(int num, int x, int y, int sx, int sy, const int *data);

// print a sprite text area
// * "num" is the number of the sprite
// * "x" and "y" is the top-left position (x, y) of the text to be printed
// * "text" is the string to be printed
void print_sprite_text(int num,
                       int x, int y,
                       const char *text);

// print a sprite grid font character
// * "num" is the number of the sprite
// * "x" and "y" is the top-left position (x, y) of the character to be printed
// * "ch" is the character to be printed
void print_sprite_grid_char(int num,
                            int x, int y,
                            int ch);

// print a sprite text string with grid font characters
// * "num" is the number of the sprite
// * "x" and "y" is the top-left position (x, y) of the text to be printed
// * "text" is the string to be printed
void print_sprite_grid_text(int num,
                            int x, int y,
                            const char *text);

// set the sprite position
// * "num" is the number of the sprite
// * "x" and "y" is the top-left position (x, y) of the sprite
void set_sprite_position(int num, int x, int y);

// disable a sprite
// * "num" is the number of the sprite
void disable_sprite(int num);

// redraw the displayed window at scrollable top-left position (x, y)
void redraw_window(int x, int y);

// scroll the displayed window to top-left position (x, y)
// * "deltax" and "deltay" is the position delta that triggers scrolling
// * "stop" determines if scrolling should stop at the edges of the scrollable area
void scroll_window(int x, int y, int deltax = 0, int deltay = 0, bool stop = true);

// release allocated memory
void release_area();
