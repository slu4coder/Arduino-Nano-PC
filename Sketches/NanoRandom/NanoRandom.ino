#include <os.h>   // copy the files 'os.h' and 'asm.s' into the folder '.../Arduino/libraries/os'

void setup()
{
  for(byte i=0; i<25; i++) os::cram[i] = i & 15;
}

void loop()
{    
  os::vram[random(25)][random(40)] = random(33, 127);
}
