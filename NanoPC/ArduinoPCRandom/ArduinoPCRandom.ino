#include <os.h>   // copy the files 'os.h' and 'os.S' into the folder '.../Arduino/libraries/os'

void setup() {}

void loop()
{    
  os::vram[random(25)][random(40)] = random(30, 256);
}
