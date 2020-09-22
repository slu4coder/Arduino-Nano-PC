#include <os.h>

int x=0, y=2;

void setup()
{
  os::fill();
  os::text(F("0123456789012345678901234567890123456789"), 0, 0);
  os::vram[y][x] = 127;
}

void loop()
{   
  byte a = os::getkey();
  if (a != 0)
  {    
    os::vram[y][x] = 32;
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
    os::vram[y][x] = 127;
  }
}
