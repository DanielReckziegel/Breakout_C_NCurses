#include <ncurses.h>
#include <math.h> // for math functions
#include <ctype.h> // for c character definitions
#include <string.h> // for c string definitions
#include <stdlib.h> // for standard c functions
#include <stdbool.h> // for standard bool values
#include <unistd.h> // for posix c functions
#include "graphics.h"
#include "gridfont.h"
#include "sound.h"
#include "scrollarea.h"

// verwendete Charakters aus gridfont
static int max_x = COLS, max_y = LINES; // screen size
const char text[] = "Breakout";
const char help[] = "Press q to quit!";
const char start[] = "Press Space to start!";
const char paddle[] = "****************";
const char gameOverChar[] = "Game Over";
const char youWonChar[] = "You Won!";

// ein Brick 
static const char brickDummy[] = 
  "##########\n"
  "##########\n"
  "##########\n";

// Startbildschirm 
static const char breakout[] =
  ("######                                                  \n"
   "#     # #####  ######   ##   #    #  ####  #    # ##### \n"
   "#     # #    # #       #  #  #   #  #    # #    #   #   \n"
   "######  #    # #####  #    # ####   #    # #    #   #   \n"
   "#     # #####  #      ###### #  #   #    # #    #   #   \n"
   "#     # #   #  #      #    # #   #  #    # #    #   #   \n"
   "######  #    # ###### #    # #    #  ####   ####    #   \n");

// Datenstruktur für ein Brick
struct Brick
{
  int id;
  int coordY;
  int coordX;
  bool dead;
};


Brick bricks[100];
int brickCounter = 0;
int scr;
//int brickSizeY = get_grid_char_lines();
//int brickSizeX = get_grid_char_cols();
bool b = false;
int lives = 3;
int score = 0;

//Prototypes
void fillBrickStruct(int index, int y, int x, bool b);
void drawFrame();
void drawStartFont(int scr);
void gameLoop();
void ballAnimation(float t, float dt);
void draw();
void drawRowOfBrickets();
void createAllBrickets();
void drawPaddle();
void paddleControl(int crtl);
void scrolling();
void scanNeededBricks();
void printNeededBrickets();
void drawBrick(int x, int y, const char *text);
void printScore();
void printLives();
void steuerung();
void gameOver();
void youWon();
void fail();
void bounce();
void bounceWall();
void brickSound();
void refreshSound();
void intro();
void scrollingSound();

//Main Methode
int main()
{
  // initialize ncurses screen
  initscr(); // initialize the screen to contain a single window
  curs_set(FALSE); // disable text cursor
  noecho(); // do not echo keyboard input
  getmaxyx(stdscr, max_y, max_x); // & not required
  clear();

  //draw Start Screen
  drawStartFont(scr);
  drawFrame();
  scanNeededBricks();
  refresh();
  sound_init();
  intro();

  // Wartet auf eine Eingabe während des Gameloops
  while (int ch = getch())
    switch (ch) 
    {
      case 'q': 
        endwin();
        return(0);
        // Beim drücken der Leertaste werden der Score und die Leben zurückgesetzt und mit einer Hilfe einer For schleife alle Bricks wieder "lebendig" gemacht, indem der Brick Bool auf false gesetzt wird. Danach wird der GameLoop wieder gestartet
      case ' ': 
        //sound_exit();
        //sound_init();
        clear();
        drawFrame();
        scrolling();
        lives = 3;
        score = 0;
        for (int i = 0; i < brickCounter; i++)
        {
          if (bricks[i].dead == true)
          {
            bricks[i].dead = false;;
          }
        }
        gameLoop();
    }
  sound_exit();
  return(0);
}

// gameloop
void gameLoop() 
{
  int fps = 20;
  float dt = 1.0/fps;
  float us = 1000 * 1000 * dt;
  float t = 0;
  timeout(0);
  getmaxyx(stdscr, max_y, max_x);
  static int ctrl = 0;
  init_pair(4, COLOR_WHITE, COLOR_BLACK);

  // Gameloop ist so lange aktiv bis der Spieler weniger als 0 Leben hat
  while (lives >=0)
  {
    clear();
    getmaxyx(stdscr, max_y, max_x);
    draw();
    getch();
    paddleControl(ctrl);
    ballAnimation(t, dt);
    color_set(4, NULL);
    printScore();
    printLives();
    steuerung();
    refresh();
    usleep(us);
    t += dt;

    // Wenn der Score gleich der Anzahl der Bricks welche auf den Bildschirm passen ist, dann hat man das Spiel gewonnen, da dann kein Brick mehr auf dem Bildschirm zu sehen ist. 
    if (score == brickCounter) 
    {
      lives = -1;
    }

    // Kontrolle der Paddles. Beim drücken wird der lives Wert auf -1 gesetzt. Somit wird der GameLoop verlassen und das Spiel abgebrochen
    int ch = getch();
    if (ch != EOF)
    {
      switch (ch) {
        case 'a':
          ctrl -= 8;
          break;
        case 'd':
          ctrl += 8;
          break;
        case 'q':
          lives  = -1;
      }
    } 
  }

  // Hier wird geprüft ob der Spieler - nachdem der Gameloop verlassen wurde - gewonnen oder verloren hat.
  if (score == brickCounter) 
  {
    clear();
    drawFrame();
    youWon();
  } else 
  {
    clear();
    drawFrame();
    gameOver();
  }
}

//draw Methode welche immer den Rahmen und die Bricks zeichnet
void draw()
{
  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_GREEN);
  init_pair(3, COLOR_WHITE, COLOR_WHITE);
  start_color();
  color_set(1, NULL);
  drawFrame();
  color_set(2, NULL);
  createAllBrickets();
}

// Score, Live und Steuerungsanzeige anzeige oben im Bildschirm
void printScore()
{
  mvprintw(1, 5,"Score: %d", score);
}

void printLives()
{
  mvprintw(1, 20,"Lives: %d", lives);
}

void steuerung()
{
  mvprintw(1, max_x/2, "Steuerung: nach links: 'a'. nach rechts 'd'. Spiel verlassen: 'q'");
}

// Bildschirmanimation wenn man das Spiel verloren hat
void gameOver()
{
  int tx = max_x/2;
  int ty = max_y/2;
  init_grid_font();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  attrset(A_BLINK);
  color_set(1, NULL);
  draw_grid_text(ty-(max_y/3), tx - strlen(gameOverChar)/2 * get_grid_char_cols(), gameOverChar);
  attrset(A_NORMAL);
  color_set(1, NULL);
  draw_grid_text(max_y-(max_y/2), tx - strlen(start)/2 * get_grid_char_cols(), start);
  draw_grid_text(max_y-(max_y/3), tx - strlen(help)/2 * get_grid_char_cols(), help);
}

// Bildschirmanimation wenn man das Spiel gewonnen hat. 
void youWon()
{
  int tx = max_x/2;
  int ty = max_y/2;
  init_grid_font();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  attrset(A_BLINK);
  color_set(1, NULL);
  draw_grid_text(ty-(max_y/3), tx - strlen(youWonChar)/2 * get_grid_char_cols(), youWonChar);
  attrset(A_NORMAL);
  color_set(1, NULL);
  draw_grid_text(max_y-(max_y/2), tx - strlen(start)/2 * get_grid_char_cols(), start);
  draw_grid_text(max_y-(max_y/3), tx - strlen(help)/2 * get_grid_char_cols(), help);
}


// Methode welche die benötigten Bricks anhand der Bildschirmgröße errechnet und daraufhin die Koordinaten der Bricks errechnet und das Struct Brick mit den Daten füllt. 
void scanNeededBricks()
{
  getmaxyx(stdscr, max_y, max_x);
  int charSizeX = 9;
  int charSizeY = 2;
  int startx = charSizeX +3;
  int starty = 3*charSizeY;

  int y = 0;
  for (int j = 0; j < max_y/2; j++)
  {
    for (int i = 0; i < max_x-(1.5*charSizeX); i++) 
    {
      fillBrickStruct(brickCounter, starty+y, startx, b); 
      i = i + (1.5*charSizeX);
      startx = startx + (1.5*charSizeX);
      brickCounter += 1;
      b = false;
    }
    startx = charSizeX +3;
    y = y + (2*charSizeY);
    j = j + (2*charSizeY);
  }
}

// Struct zum Befüllen der Bricks mit den benötigten Daten (koordinaten und bool werte)
void fillBrickStruct(int index, int y, int x, bool b)
{
  bricks[index].coordY = y;
  bricks[index].coordX = x;
  bricks[index].dead = b;
}


//Methode welche alle Brickets Zeichnet
void createAllBrickets()
{
  getmaxyx(stdscr, max_y, max_x);
  int charSizeX = 9;
  int charSizeY = 2;
  int startx = charSizeX;
  int y = 0;
  for (int j = 0; j < max_y/2; j++)
  {
    for (int i = 0; i < max_x-(1*charSizeX); i++) 
    {
      for (int k = 0; k < brickCounter; k++)
      {
        // Es werden nur die Bricks gezeichnet, welche bei ihrem Boolean Wert dead ein false stehen haben.
        // Anfangs haben alle Bricks einen false wert.
        if (bricks[k].dead == false)
        {
          draw_sprite(bricks[k].coordY, bricks[k].coordX, brickDummy);
        }
      }
      i = i + (1.5*charSizeX);
      startx = startx + (1.5*charSizeX);
    }
    startx = charSizeX;
    y = y + (2*charSizeY);
    j = j + (2*charSizeY);
  }
}


//Startbildschirm
void drawStartFont(int scr)
{
  int tx = max_x/2;
  init_grid_font();
  void intro();

  // Blinkender Breakout Schriftzug
  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  attrset(A_BLINK);
  color_set(1, NULL);
  draw_sprite(max_y/3, tx - strlen(text)/2, breakout);
  attrset(A_NORMAL);

  //print start & help text
  color_set(1, NULL);
  draw_grid_text(max_y-(max_y/2), tx - strlen(start)/2 * get_grid_char_cols()+scr, start);
  draw_grid_text(max_y-(max_y/3), tx - strlen(help)/2 * get_grid_char_cols()+scr, help);
}


//Rahmen Zeichnen
void drawFrame() 
{
  char rand[] = "$";
  int i;
  int j;

  color_set(1, NULL);
  //links oben nach links unten
  for (i = 0; i < max_y-get_grid_char_lines()+4; i++) 
  {
    init_grid_font();
    draw_grid_text(0+i, 0, rand);
  }

  // links oben nach rechts oben
  for (j = 0; j < max_x-get_grid_char_cols()+1; j++) 
  {
    init_grid_font();
    draw_grid_text(0, j-(get_grid_char_cols()), rand);
  }

  // rechts oben nach rechts unten
  for (i = 0; i < max_y-get_grid_char_lines()+4; i++) 
  {
    init_grid_font();
    draw_grid_text(0+i, max_x-(get_grid_char_cols()), rand);
  }

  // links unten nach rechts unten
  for (j = 0; j < max_x-get_grid_char_cols(); j++) 
  {
    init_grid_font();
    draw_grid_text(max_y-(get_grid_char_lines()), 0+j, rand);
  }
}

void ballAnimation(float t, float dt)
{
  //Größe eines Bricks 
  int charSizeX = 9;
  int charSizeY = 2;

  getmaxyx(stdscr, max_y, max_x);
  static int bx = max_x/2;
  static int by = max_y/2 + charSizeY;
  static int dbx = 1;
  static int dby = 1;
  start_color();
  color_set(3, NULL);
  mvaddch(by, bx, '*');
  bx += dbx;
  by += dby;

  //collision Rahmen X-Achse
  if (bx >= max_x-get_grid_char_cols()-2) 
  {
    dbx = -dbx;
    bounceWall();
  }
  else if (bx <= get_grid_char_cols()) 
  {
    dbx = -dbx;
    bounceWall();
  }

  //collision Rahmen Y-Achse
  if (by >= max_y) 
  {
    lives -= 1;
    fail();
    bx = max_x/2;
    by = max_y/2;
  }
  else if (by <= get_grid_char_lines()+1) 
  {
    dby = -dby;
    bounceWall();
  }

  // Bricks Collisionserkennung
  // Die Collisionserkennung funktioniert leider nicht so wie ich es mir vorgestellt habe. Ich habe versucht die verschiedenen Koordinaten der generierten Bricks aus den Structs zu holen und den cursor mit den koordinaten abgeglichen. Daraufhin wird der boolean des structs auf true gesetzt. Es werden immer nur alle Bricks gezeichnet, deren Zustand "false" ist. Die Bricks werden zwar jedes mal erkannt, (zu erkennen am steigenenden Score bei jeder berührung) jedoch verschwinden die bricks leider nicht. 
  int mask = A_CHARTEXT | A_ALTCHARSET;
  int col = mvinch(by, bx) & mask;
  // unterseite des Bricks
  if (col =='#')
  {
    brickSound();
    for (int i = 0; i <= brickCounter; i++)
    {
      // Unterseite des Bricks
      // Wenn die Y-Koordiante des Balls gleich der Y-Koordinate + der länge (Lines) eines Bricks ist
      if (by == bricks[i].coordY+charSizeY)
      {
        // Wenn die X-Koordinate des Balls zwischen der X-Koordinate des Bricks und der X-Koordinate + der Größe des Bricks ist. Dann wird der boolean dead des Bricks auf true gesetzt. Dadurch wird dieser nichtmehr gezeichnet. 
        if (bricks[i].coordX <= bx && bx <= (bricks[i].coordX+charSizeX)) 
        {
          dby = -dby;
          bricks[i].dead = true;
          refresh();
          score += 1;
          clear();
        }
      }
      // oberseite des Bricks
      // Wenn die Y-Koordiante des Balls gleich der Y-Koordinate eines Bricks ist
      else if (by == bricks[i].coordY)
      {
        // Wenn die X-Koordinate des Balls zwischen der X-Koordinate des Bricks und der X-Koordinate + der Größe des Bricks ist. Dann wird der boolean dead des Bricks auf true gesetzt. Dadurch wird dieser nichtmehr gezeichnet. 
        if (bricks[i].coordX <= bx && bx <= bricks[i].coordX+charSizeX) 
        {
          dby = -dby;
          bricks[i].dead = true;
          refresh();
          score += 1;
          clear();
        }
      }
      // rechter Rand des Bricks
      // Wenn die X-Koordiante des Balls gleich der X-Koordinate + der Breite (Cols) eines Bricks ist
      else if (bx == (bricks[i].coordX+charSizeX))
      {
        // Wenn die Y-Koordinate des Balls zwischen der Y-Koordinate des Bricks und der Y-Koordinate + der Länge (Lines) des Bricks ist. Dann wird der boolean dead des Bricks auf true gesetzt. Dadurch wird dieser nichtmehr gezeichnet. 
        if (bricks[i].coordY <= by && by <= (bricks[i].coordY+charSizeY)) 
        {
          dbx = -dbx;
          bricks[i].dead = true;
          refresh();
          score += 1;
          clear();
        }
      }
      // linker Rand des Bricks
      // Wenn die X-Koordiante des Balls gleich der X-Koordinate eines Bricks ist
      else if (bx == bricks[i].coordX)
      {
        // Wenn die Y-Koordinate des Balls zwischen der Y-Koordinate des Bricks und der Y-Koordinate + der Länge (Lines) des Bricks ist. Dann wird der boolean dead des Bricks auf true gesetzt. Dadurch wird dieser nichtmehr gezeichnet. 
        if (bricks[i].coordY <= by && by <= (bricks[i].coordY+charSizeY)) 
        {
          dbx = -dbx;
          bricks[i].dead = true;
          refresh();
          score += 1;
          clear();
        }
      }
    }
  }
  //Paddle collision
  if (col =='*')
  {
    dby = -dby;
    bounce();
  }
}

// Methode zur Erstellung des Paddles
void drawPaddle()
{
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  color_set(1, NULL);
  mvprintw(max_y-(2*get_grid_char_lines()), max_x/2, paddle);
  refresh();
  color_set(2, NULL);
}


// Methode zur Kontrolle des Paddles
void paddleControl(int ctrl)
{
  getmaxyx(stdscr, max_y, max_x);
  static float bx = max_x/2;
  static float by = max_y-(2*get_grid_char_lines());
  static int paddleY = by + 0.5;
  static int paddleX = bx + 0.5;
  // Der Wert von ctrl wird bei den Tastendrücken 'a' und 'd' vergößert oder verkleinert und dan die Methode übergeben
  mvprintw(paddleY, paddleX+ctrl, paddle);
}


// Animationsbildschirm nach dem Startbildschirm beim Starten des Spiels. Während dieses Scrolling Bildschirmes wird auch die Methode ScanNeededBricks ausgeführt um  anhand der Bildschirmgröße du zu benötigten Bricks zu errechnen.
void scrolling()
{
  scrollingSound();
  set_area_size(max_x, max_y);
  set_window_size(max_x-(2*get_grid_char_cols()), max_y-(2*get_grid_char_lines()));
  set_window_offset(get_grid_char_cols(), get_grid_char_lines());
  clear_area('-');
  createAllBrickets();

  for (scr=0; scr < max_x; scr++)
  {
    redraw_window(get_grid_char_cols()-scr, get_grid_char_lines());
    refresh();
    usleep(10*1000);
  }
  draw();
  getch();
}


void drawBricks(int y, int x, const char *text)
{
  move(y, x);
  while (*text != '\0')
  {
    if (*text == '\n')
    {
      move(++y, x);
    }
    else
    {
      addch(*text);
    }
    text++;
  }
}

// Sounds 
// Sound beim verlieren eines Lebens
void fail()
{
  sound_play("fail.wav");
}

//Sound beim Collidieren mit dem Paddle
void bounce()
{
  sound_play("bounce.wav");
}

// Sound beim Kollidieren mit der Wand bzw. dem Rahmen
void bounceWall()
{
  sound_play("boing.wav");
}

// Sound beim zerstören eines Bricks
void brickSound()
{
  sound_play("hit.wav");
}

//Sound beim Neustarten eines Spiels 
void refreshSound()
{
  sound_play("bing.wav");
}

// Intromusik
void intro()
{
  sound_play("Intro.wav");
}

// Sound beim Scrollingbildschirm
void scrollingSound()
{
  sound_play("scrolling.wav");
}


