#include <os.h>   // copy the files 'os.h' and 'os.S' into the folder '.../Arduino/libraries/os'

#define MAXLINES 70
struct line { char x, y, h; };
line lines[MAXLINES];

void setup()
{
  os::fill();
  for (byte i=0; i<MAXLINES; i++)
  {
    lines[i].x = random(COLS);    // make a random line
    lines[i].y = random(-1,ROWS);
    lines[i].h = random(3, 15);
  }
}

void loop()
{     
  int f = os::frames;

  for (byte i=0; i<MAXLINES; i++)
  {
    lines[i].y++;
    if (lines[i].y >= 0 && lines[i].y < ROWS) os::vram[lines[i].y][lines[i].x] = random(33, 127);
    if (lines[i].y - lines[i].h >= 0)
    {
      if (lines[i].y - lines[i].h < ROWS) os::vram[lines[i].y - lines[i].h][lines[i].x] = 32;
      else
      {
        lines[i].x = random(COLS);    // make a random line
        lines[i].y = -1;
        lines[i].h = random(3, 15);          
      }
    }
  }
  
  os::wait(5);
}
