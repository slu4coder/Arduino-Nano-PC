// ------------------------------------------------------------------------
// Displays the 'Mandelbrot' set by projecting the area (-2.5..1) * (-1..1)
// onto 32 x 22 pixels using a maximum of 15 iterations and 32-bit integer
// math in 9-bit fixed-point format  -    written by C. Herting (slu4) 2024
// ------------------------------------------------------------------------
#include <os.h> // copy the files 'os.h' and 'os.S' into the folder '.../Arduino/libraries/os'

void setup() { os::fill(); } // clear the screen

void loop()
{      
  long cb = -506; // -1
  for (byte y=0; y<22; y++)
  {
    long ca = -1288; // -2.5
    for (byte x=0; x<32; x++)
    {
      long za = ca; // init start condition
      long zb = cb;
      byte n;
      
      for (n=15; n>0; n--) // iterate f(z) = z^2 + c
      {
        long za_sq = (za * za) >> 9;
        long zb_sq = (zb * zb) >> 9;
        if (za_sq + zb_sq >= (4 << 9)) break; // diverging if z^2 >= 4
        zb = (((za * zb) >> 9) << 1) + cb; // zb = za*zb + cb
        za = za_sq - zb_sq + ca; // za = za^2 - zb^2 + ca
      }
      os::vram[y][x] = ' ' + n; // plot current pixel
      ca += 56;
    }
    cb += 46;
  }
}
