#include <os.h>   // copy the files 'os.h' and 'os.S' into a folder '/Arduino/libraries/os'

int x=0, y=5;     // cursor position

void setup()
{
  os::text(F("READY."), 0, 4);
  os::vram[y][x] = os::vram[y][x] + 128; // invert char at cursor position
}

void loop()
{   
  byte a = os::getkey();
  if (a != 0) // any key pressed?
  {    
    os::vram[y][x] = os::vram[y][x] - 128; // un-invert char at old cursor position
    switch(a)
    { 
      case 17: if (y>0) y--; break;
      case 18: if (y<24) y++; break;      
      case 19: if (x>0) x--; break;
      case 20: if (x<39) x++; break;      
      case 10: x=0; if (y<24) y++; else os::scroll(); break;
      case 8: x--; if (x<0) x=0; os::vram[y][x] = 32; break;
      default: os::vram[y][x++] = a; if (x>39) { x=0; if (y<24) y++; else os::scroll(); } break;
    }
    os::vram[y][x] = os::vram[y][x] + 128; // invert char at new cursor position
  }
}
