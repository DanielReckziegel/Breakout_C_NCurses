// NCurses graphics
// (c) 2020 by Stefan Roettger

#include "graphics.h"
#include "gridfont.h"

#include <time.h>

static WINDOW *W = NULL; // the drawing window

// init ASCII GFX
void init_gfx()
{
   // initialize ncurses screen
   // * the screen origin (0, 0) is at the top left corner
   initscr(); // initialize the screen to contain a single window
   curs_set(FALSE); // disable text cursor
   noecho(); // do not echo keyboard input
}

// exit ASCII GFX
void exit_gfx()
{
   // restore normal terminal behaviour before exiting
   endwin();
}

// set the window
void set_window(WINDOW *w)
{
   W = w;
}

// draw a frame at the edges of the screen
void draw_frame()
{
   WINDOW *w = W?W:stdscr;

   wborder(w, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
           ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
}

// draw a sprite at barycenter position (x, y)
void draw_sprite(int y, int x,
                 const char *text)
{
   WINDOW *w = W?W:stdscr;

   // calculate sprite size
   int cols = 0;
   int rows = 1;
   const char *c = text;
   while (*c != '\0')
   {
      if (*c++ != '\n')
         cols++;
      else
         rows++;
   }

   // offset from barycenter by subtracting half sprite size
   cols /= rows;
   x -= cols/2;
   y -= rows/2;

   // print consecutive sprite lines
   wmove(w, y, x);
   while (*text != '\0')
   {
      if (*text != '\n')
         waddch(w, *text);
      else
         wmove(w, ++y, x);

      text++;
   }
}

// draw a screen area at top-left position (x, y) with size (sx, sy)
void draw_area(int y, int x,
               int sy, int sx,
               const int *data)
{
   WINDOW *w = W?W:stdscr;

   for (int j=0; j<sy; j++)
   {
      wmove(w, y + j, x);
      for (int i=0; i<sx; i++)
      {
         int ch = data[i+j*sx];
         if (ch >= 0) waddch(w, ch);
      }
   }
}

// draw a text string with grid font characters at top-left position (x, y)
void draw_grid_text(int y, int x,
                    const char *text)
{
   int start = x;

   int sy = get_grid_char_lines();
   int sx = get_grid_char_cols();

   while (*text != '\0')
   {
      if (*text != '\n')
      {
         int *data = get_grid_char_data(*text);
         draw_area(y, x, sy, sx, data);
         x += sx;
      }
      else
      {
         x = start;
         y += sy;
      }

      text++;
   }
}

/*

 Evolution from a naive line drawing implementation, which draws a line
 from position (x1, y1) to (x2, y2), to the Bresenham algorithm:

 Step 1) Naive implementation (horizontal case)

 float dx = x2 - x1;
 float dy = y2 - y1;
 float delta = dy / dx;
 int x = x1;
 float y = y1;
 while (x++ <= x2)
 {
    mvaddch((int)(y+0.5), x, '*');
    y = y + delta;
 }

 Step 2) Differential implementation (float version)

 float dx = x2 - x1;
 float dy = y2 - y1;
 float delta = dy / dx;
 int x = x1, y = y1;
 float err = -0.5;
 while (x++ <= x2)
 {
    mvaddch(y, x, '*');
    err += delta;
    if (err > 1)
    {
       err -= 1;
       y++;
    }
 }

 Step 3) Differential implementation (float version /wo division)

 int dx = x2 - x1;
 int dy = y2 - y1;
 int x = x1, y = y1;
 float err = -0.5*dx;
 while (x++ <= x2)
 {
    mvaddch(y, x, '*');
    err += dy;
    if (err > dx)
    {
       err -= dx;
       y++;
    }
 }

 Step 4) Differential implementation (int version)

 int dx = x2 - x1;
 int dy = y2 - y1;
 int x = x1, y = y1;
 int err = -dx;
 while (x++ <= x2)
 {
    mvaddch(y, x, '*');
    err += 2*dy;
    if (err > dx)
    {
       err -= 2*dx;
       y++;
    }
 }

 Step 5) Differential implementation (negated version)

 int dx = x2 - x1;
 int dy = y2 - y1;
 int x = x1, y = y1;
 int err = dx;
 while (x++ <= x2)
 {
    mvaddch(y, x, '*');
    err -= dy<<1;
    if (err < 0)
    {
       err += dx<<1;
       y++;
    }
 }

 Step 6) Universal implementation (Bresenham algorithm)

 int dx = x2 - x1;
 int dy = y2 - y1;
 if (abs(dx) > abs(dy))
 {
    // x is fast direction
    ... horizontal case
 }
 else
 {
    // y is fast direction
    ... symmetrical vertical case
 }

*/

// draw a line from position (x1, y1) to (x2, y2)
void draw_line(int y1, int x1, int y2, int x2,
               int ch)
{
   WINDOW *w = W?W:stdscr;

   int dx, dy;
   int ix, iy;

   // determine x increment
   if (x2 >= x1)
   {
      dx = x2-x1;
      ix = 1;
   }
   else
   {
      dx = x1-x2;
      ix = -1;
   }

   // determine y increment
   if (y2 >= y1)
   {
      dy = y2-y1;
      iy = 1;
   }
   else
   {
      dy = y1-y2;
      iy = -1;
   }

   int pdx, pdy;
   int ddx, ddy;
   int slow, fast;

   // determine fast direction
   if (dx > dy)
   {
      // x is fast direction
      pdx = ix; pdy = 0;
      ddx = ix; ddy = iy;
      slow = dy; fast = dx;
   }
   else
   {
      // y is fast direction
      pdx = 0; pdy = iy;
      ddx = ix; ddy = iy;
      slow = dx; fast = dy;
   }

   // determine line character
   int c = '*';
   if (dx == 0) c = ACS_VLINE;
   if (dy == 0) c = ACS_HLINE;
   if (ch >= 0) c = ch;

   // loop along fast direction
   int x = x1, y = y1;
   int err = fast;
   int i = 0;
   while (TRUE)
   {
      int b = mvwinch(w, y, x) & A_CHARTEXT;

      if (b == ' ')
         mvwaddch(w, y, x, c);

      if (i++ == fast)
         break;

      err -= slow<<1;

      if (err < 0)
      {
         err += fast<<1;
         x += ddx;
         y += ddy;
      }
      else
      {
         x += pdx;
         y += pdy;
      }
   }
}

// print the alternate ncurses charset in a table
void print_altcharset()
{
   WINDOW *w = W?W:stdscr;

   attron(A_UNDERLINE);
   wprintw(w, "NCURSES ALTCHARSET CHARACTERS\n");
   attroff(A_UNDERLINE);

   int counter, counter2=0;
   for (counter=32; counter<256; counter++)
   {
      // output one altcharset character
      wprintw(w, "%3d = ", counter);
      waddch(w, counter | A_ALTCHARSET);

      // output table dividers
      if (counter2 < 7)
      {
         waddch(w, ' ');
         waddch(w, ACS_VLINE);
         waddch(w, ' ');
      }

      // wrap table columns
      if (++counter2 > 7)
      {
         waddch(w, '\n');
         counter2 = 0;
      }

      // skip unprintable characters
      if (counter == 127)
         counter = 159;
   }
}

// check for repeating time period
bool is_repeating(float time, float period,
                  float percent)
{
   float w = time/period-(int)(time/period);
   return(w<percent/100);
}

// random float number in the range [0,1[
float rnd()
{
   static bool init = false;

   if (!init)
   {
      srand((int)time(NULL));
      init = true;
   }

   return(rand()/((float)RAND_MAX+1));
}
