#include <os.h>

char state, x, y, waiting;
int score, high = 0;
char shape[10];       // holding the current shape (with offset)
const char minos[7][10] PROGMEM = { { 0, 0, 1, 0, 0, -1, 1, -1,   0, 1 },     // Quadrat (with SRS offset)
                                    { -1, -1, 0, -1, 0, 0, 1, 0,  0, 0 },     // Z
                                    { -1, 0, 0, 0, 0, -1, 1, -1,  0, 0 },     // neg. Z
                                    { -1, 0, 0, 0, 1, 0, 2, 0,    0, 1 },     // I
                                    { -1, 0, 0, 0, 1, 0, 0, -1,   0, 0 },     // Podest
                                    { -1, -1, -1, 0, 0, 0, 1, 0,  0, 0 },     // L
                                    { -1, 0, 0, 0, 1, 0, 1, -1,   0, 0 }  };  // neg. L

void drawintro()
{
  os::fill();
  os::text(F("A R D U I N O   B L O C K S"), 6, 10, CYAN);
  os::text(F("Press  SPACE"), 13, 23, YELLOW);
}

void drawfield()
{
  os::fill();
  for (byte i=0; i<5; i++) os::text(F("<!..........!>"), 12, i, CYAN);
  for (byte i=5; i<20; i++) os::text(F("<!..........!>"), 12, i, WHITE);
  os::text(F("<!==========!>"), 12, 20, CYAN);
  os::text(F("  VVVVVVVVVV  "), 12, 21, MAGENTA);
  os::text(F("SCORE 0"), 0, 0, GREEN);
  os::text(F("HIGH"), 27, 0);    
  os::text(String(high, DEC), 32, 0);
  os::text(F("CONTROLS"), 0, 3, CYAN);
  os::text(F("A - Left"), 0, 5, GRAY);
  os::text(F("D - Right"), 0, 6, GRAY);
  os::text(F("W - Rotate"), 0, 7, GRAY);
  os::text(F("S - Drop"), 0, 8, GRAY);
}

boolean testShape(char x, char y)
{
  boolean isok = true;
  for (char i=0; i<8; i+=2)
  {
    if (x + shape[i] < 14+0 || x + shape[i] > 14+9) { isok = false; break; }         // linker / rechter Rand
    if (y + shape[i+1] < 0 || y + shape[i+1] > 19) { isok = false; break; }        // oberer / unterer Rand
    if (os::vram[y + shape[i+1]][x + shape[i]] != '.') { isok = false; break; }   // Spielfeld
  }
  return isok;
}

void newShape()
{
  byte m = random(7);
  for (char i=0; i<10; i++) shape[i] = pgm_read_byte_near(&minos[m][i]);
  x = 14+4; y = 1;
  if (testShape(x, y)) drawShape('#');
  else
  {
    os::text(F("GAME  OVER"), 14, 10, RED);
    os::text(F("Press  SPACE"), 13, 23, YELLOW);
    state = 2;
  }
}

void rotShape()
{
  char rotshape[10];
  for(char i=0; i<10; i+=2)
  {
    rotshape[i] = shape[i+1];
    rotshape[i+1] = -shape[i];
  }
  for (char i=0; i<10; i++) shape[i] = rotshape[i];
  x += shape[8]; y+= shape[9];  // add rotated offset correction
}

void drawShape(char c)
{
  for (char i=0; i<8; i+=2) os::vram[y + shape[i+1]][x + shape[i]] = c;        // Shape ins Feld schreiben  
}

void updateField()
{
  char anzrows = 0;                                                             // count the number of rows cleared
  for (char y=0; y<20; y++)                                                    // gehe alle Reihen durch
  {
    boolean rowfull = true;
    for(char x=14; x<14+10; x++) if (os::vram[y][x] == '.') rowfull = false;            // ist diese Reihe voll?
    if (rowfull)
    {
      anzrows++;
      for(char i=y; i>0; i--) for(char x=14; x<14+10; x++) os::vram[i][x] = os::vram[i-1][x];   // kopiere ab dieser Reihe alles nach unten
      for(char x=14; x<14+10; x++) os::vram[0][x] = '.';                                      // und lösche die obere Zeile
    }
  }
  switch (anzrows)
  {
    case 1: score += 40; break;
    case 2: score += 100; break;
    case 3: score += 200; break;
    case 4: score += 500; break;
  }
}

void setup()
{
  drawintro();
  state = 0;
}

void loop()
{ 
  int frame = os::frames;
  byte key = os::getkey();
  switch(state)
  {
    case 0:
      if (key == ' ')
      {
        randomSeed(os::frames); random();
        drawfield();
        newShape();
        waiting = 40; os::frames = 0; score = 0; state = 1;

      }
      break;
    case 1:
      if (key != 0)
      {
        if (key == 's') { waiting -= 40; score++; }
        else
        {
          drawShape('.');        // Shape ins Feld schreiben  
          switch (key)
          {
            case 'a':
              if (testShape(x-1, y)) x--;
              break;
            case 'd':
              if (testShape(x+1, y)) x++;
              break;
            case 'w':
              rotShape();                                               // drehe die Shape ccw
              if (testShape(x, y) == false)                             // prüfe, ob darunter KEIN Platz ist
              {
                if (testShape(x-1, y)) x--;                             // teste links
                else if (testShape(x+1, y)) x++;                        // teste rechts
                else if (testShape(x-2, y)) x-=2;                       // Sonderfall langer Balken
                else if (testShape(x+2, y)) x+=2;
                else { rotShape(); rotShape(); rotShape(); }            // geht nicht => Drehung rückgängig machen          
              }
              break;            
          }
          drawShape('#');        // Shape ins Feld schreiben  
        }
      }
      if (waiting-- < 0)                                          // FALLEN
      {
        waiting += (40 - (os::frames >> 9));
        drawShape('.');
        if (testShape(x, y+1)) { y++; drawShape('#'); }
        else
        {
          drawShape('#');                   // fix shape
          updateField();                    // clear full rows
          os::text(String(score, DEC), 6, 0);          
          if (score > high) { high = score; os::text(String(high, DEC), 32, 0); }
          newShape();                       // pick a new shape if possible
          drawShape('#');
        }
      }  
      break;
    case 2:
      if (key == ' ') { drawintro(); state = 0; }
      break;
  }
  while (frame == os::frames);
}
