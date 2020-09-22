// *****************************************************************************
// *****                                                                   *****
// ***** ARDUINO NANO PC System with 40x25 Character VGA and PS/2 Keyboard *****
// *****                                                                   *****
// *****        Written by Carsten Herting 18.09.2020 Version 2.5          *****
// *****                                                                   *****
// *****************************************************************************

// Copy this header library to 'Arduino/libraries/os/os.h'

// USAGE
// os::vram[0..24][0..39]                 video ram array (25 rows, 40 columns)
// os::frames                             frame counter (60Hz)
// os::text(String s, byte x, byte y)     prints a text string s to a given postion (x, y)
// os::fill(char c)                       fills the entire video RAM with c
// os::scroll()                           scrolls up the video RAM by one step
// byte os::getkey()                      returns ASCII code of pressed keys

// LICENSE: See end of file for license information.
// Set Arduino Nano FuseA from 0xFF to 0xBF to enable output of 16MHz CLKO on pin D8
// Use macro F("...") to define const Strings in flash memory when SRAM is short.
// Use PROGMEM to define const variables in flash memory when SRAM is short.
// Pin D0-7: parallel pixel output to 74165 (D6-7: also 74164 input from keyboard)
// Pin D8:   CLKO (16MHz)
// Pin D9:   MR 74164 keyboard register
// Pin D10:  /VSYNC VGA (timer1, every 1/60s)
// Pin D11:  74165 pixel shift register parallel load (timer2, 1MHz => 2MHz /PE)
// Pin D12:  /HSYNC VGA (inside ISR, every 32µs)

namespace os
{  
  #define SIZE  8									// keyboard input buffer size
  volatile byte reg[SIZE];        // ring buffer of register data
  volatile byte regout = 0;       // index of current output position in queue
  volatile byte regin = 0;        // index of current input position in queue
  
  volatile int frames = 0;        // counting the displayed frames (60Hz)
  volatile int vline = 0;         // current vertical position of pixel video output
  volatile byte vram[25][40];     // array of video ram data
  
  const byte charset[8][96] = {   // charset line data starting with character 32 (SPACE)
    0x00,0x18,0x66,0x66,0x18,0x62,0x3C,0x06,0x0C,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x18,0x3C,0x3C,0x06,0x7E,0x3C,0x7E,0x3C,0x3C,0x00,0x00,0x0E,0x00,0x70,0x3C,0x3C,0x18,0x7C,0x3C,0x78,0x7E,0x7E,0x3C,0x66,0x3C,0x1E,0x66,0x60,0x63,0x66,0x3C,0x7C,0x3C,0x7C,0x3C,0x7E,0x66,0x66,0x63,0x66,0x66,0x7E,0x3C,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x18,0x70,0x00,0xFF,
    0x00,0x18,0x66,0x66,0x3E,0x66,0x66,0x0C,0x18,0x18,0x66,0x18,0x00,0x00,0x00,0x03,0x66,0x18,0x66,0x66,0x0E,0x60,0x66,0x66,0x66,0x66,0x00,0x00,0x18,0x00,0x18,0x66,0x66,0x3C,0x66,0x66,0x6C,0x60,0x60,0x66,0x66,0x18,0x0C,0x6C,0x60,0x77,0x76,0x66,0x66,0x66,0x66,0x66,0x18,0x66,0x66,0x63,0x66,0x66,0x06,0x30,0x60,0x0C,0x18,0x00,0x66,0x00,0x60,0x00,0x06,0x00,0x0E,0x00,0x60,0x18,0x06,0x60,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x00,0xFF,
    0x00,0x18,0x66,0xFF,0x60,0x0C,0x3C,0x18,0x30,0x0C,0x3C,0x18,0x00,0x00,0x00,0x06,0x6E,0x38,0x06,0x06,0x1E,0x7C,0x60,0x0C,0x66,0x66,0x18,0x18,0x30,0x7E,0x0C,0x06,0x6E,0x66,0x66,0x60,0x66,0x60,0x60,0x60,0x66,0x18,0x0C,0x78,0x60,0x7F,0x7E,0x66,0x66,0x66,0x66,0x60,0x18,0x66,0x66,0x63,0x3C,0x66,0x0C,0x30,0x30,0x0C,0x3C,0x00,0x6E,0x3C,0x60,0x3C,0x06,0x3C,0x18,0x3E,0x60,0x00,0x00,0x60,0x18,0x66,0x7C,0x3C,0x7C,0x3E,0x7C,0x3E,0x7E,0x66,0x66,0x63,0x66,0x66,0x7E,0x18,0x18,0x18,0x00,0xFF,
    0x00,0x18,0x00,0x66,0x3C,0x18,0x38,0x00,0x30,0x0C,0xFF,0x7E,0x00,0x7E,0x00,0x0C,0x76,0x18,0x0C,0x1C,0x66,0x06,0x7C,0x18,0x3C,0x3E,0x00,0x00,0x60,0x00,0x06,0x0C,0x6E,0x7E,0x7C,0x60,0x66,0x78,0x78,0x6E,0x7E,0x18,0x0C,0x70,0x60,0x6B,0x7E,0x66,0x7C,0x66,0x7C,0x3C,0x18,0x66,0x66,0x6B,0x18,0x3C,0x18,0x30,0x18,0x0C,0x7E,0x00,0x6E,0x06,0x7C,0x60,0x3E,0x66,0x3E,0x66,0x7C,0x38,0x06,0x6C,0x18,0x7F,0x66,0x66,0x66,0x66,0x66,0x60,0x18,0x66,0x66,0x6B,0x3C,0x66,0x0C,0x70,0x18,0x0E,0x3B,0xFF,
    0x00,0x00,0x00,0xFF,0x06,0x30,0x67,0x00,0x30,0x0C,0x3C,0x18,0x00,0x00,0x00,0x18,0x66,0x18,0x30,0x06,0x7F,0x06,0x66,0x18,0x66,0x06,0x00,0x00,0x30,0x7E,0x0C,0x18,0x60,0x66,0x66,0x60,0x66,0x60,0x60,0x66,0x66,0x18,0x0C,0x78,0x60,0x63,0x6E,0x66,0x60,0x66,0x78,0x06,0x18,0x66,0x66,0x7F,0x3C,0x18,0x30,0x30,0x0C,0x0C,0x18,0x00,0x60,0x3E,0x66,0x60,0x66,0x7E,0x18,0x66,0x66,0x18,0x06,0x78,0x18,0x7F,0x66,0x66,0x66,0x66,0x60,0x3C,0x18,0x66,0x66,0x7F,0x18,0x66,0x18,0x18,0x18,0x18,0x6E,0xFF,
    0x00,0x00,0x00,0x66,0x7C,0x66,0x66,0x00,0x18,0x18,0x66,0x18,0x18,0x00,0x18,0x30,0x66,0x18,0x60,0x66,0x06,0x66,0x66,0x18,0x66,0x66,0x18,0x18,0x18,0x00,0x18,0x00,0x66,0x66,0x66,0x66,0x6C,0x60,0x60,0x66,0x66,0x18,0x6C,0x6C,0x60,0x63,0x66,0x66,0x60,0x3C,0x6C,0x66,0x18,0x66,0x3C,0x77,0x66,0x18,0x60,0x30,0x06,0x0C,0x18,0x00,0x66,0x66,0x66,0x60,0x66,0x60,0x18,0x3E,0x66,0x18,0x06,0x6C,0x18,0x6B,0x66,0x66,0x7C,0x3E,0x60,0x06,0x18,0x66,0x3C,0x3E,0x3C,0x3E,0x30,0x18,0x18,0x18,0x00,0xFF,
    0x00,0x18,0x00,0x66,0x18,0x46,0x3F,0x00,0x0C,0x30,0x00,0x00,0x18,0x00,0x18,0x60,0x3C,0x7E,0x7E,0x3C,0x06,0x3C,0x3C,0x18,0x3C,0x3C,0x00,0x18,0x0E,0x00,0x70,0x18,0x3C,0x66,0x7C,0x3C,0x78,0x7E,0x60,0x3C,0x66,0x3C,0x38,0x66,0x7E,0x63,0x66,0x3C,0x60,0x0E,0x66,0x3C,0x18,0x3C,0x18,0x63,0x66,0x18,0x7E,0x3C,0x03,0x3C,0x18,0x00,0x3C,0x3E,0x7C,0x3C,0x3E,0x3C,0x18,0x06,0x66,0x3C,0x06,0x66,0x3C,0x63,0x66,0x3C,0x60,0x06,0x60,0x7C,0x0E,0x3E,0x18,0x36,0x66,0x0C,0x7E,0x0E,0x18,0x70,0x00,0xFF,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x60,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x18,0x00,0x00,0xFF,
  };
  
  const byte LookupScanToASCII[3][128] PROGMEM = {  // lookup table (in: SHIFT/ALTGR state and PS2 scancode => out: desired ASCII code) change for your country's keyboard layout
    { 0,0,0,0,0,0,0,0,         0,0,0,0,0,9,94,0,         0,0,0,0,0,113,49,0,       0,0,121,115,97,119,50,0,    // w/o SHIFT or ALT(GR)
      0,99,120,100,101,52,51,0,0,32,118,102,116,114,53,0,0,110,98,104,103,122,54,0,0,0,109,106,117,55,56,0,
      0,44,107,105,111,48,57,0,0,46,45,108,0,112,0,0,    0,0,0,0,0,96,0,0,         0,0,10,43,0,35,0,0,
      0,60,0,0,0,0,8,0,        0,0,0,19,0,0,0,0,         0,0,18,0,20,17,27,0,      0,0,0,0,0,0,0,0  },
    { 0,0,0,0,0,0,0,0,         0,0,0,0,0,0,248,0,        0,0,0,0,0,81,33,0,        0,0,89,83,65,87,34,0,       // with SHIFT
      0,67,88,68,69,36,0,0,    0,0,86,70,84,82,37,0,     0,78,66,72,71,90,38,0,    0,0,77,74,85,47,40,0,
      0,59,75,73,79,61,41,0,   0,58,95,76,0,80,63,0,     0,0,0,0,0,0,0,0,          0,0,0,42,0,39,0,0,
      0,62,0,0,0,0,0,0,        0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0  },
    { 0,0,0,0,0,0,0,0,         0,0,0,0,0,0,0,0,          0,0,0,0,0,64,0,0,         0,0,0,0,0,0,0,0,            // with ALT(GR)
      0,0,0,0,0,0,0,0,         0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,          0,0,0,0,0,123,91,0,
      0,0,0,0,0,125,93,0,      0,0,0,0,0,0,92,0,         0,0,0,0,0,0,0,0,          0,0,0,126,0,0,0,0,
      0,124,0,0,0,0,0,0,       0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0  } };
  
  void init()
  {
    noInterrupts();               // disable interrupts before messing around with timer registers

    DDRC  = 0b00000000;
		PORTC = 0b00000000;
		DDRD  = 0b11111111;
    PORTD = 0b00000000;  
    DDRB  = 0b00011111;           // pin8: CLKO, pin 9: 74173 /OE, pin 10: VSYNC (timer1), pin 11: 74165 PE (timer2), pin 12: HSync "by hand" inside ISR, pin 13: 74173 MR
    PORTB = 0b00010000;           // HSYNC=1, /MR=0

    GTCCR = 0b10000011;           // set TSM, PSRSYNC und PSRASY to correlate all 3 timers
  
    // *****************************
    // ***** Timer0: VGA HSYNC *****
    // *****************************
    TCNT0  = 6;										// align VSYNC and HSYNC pulses
    TCCR0A = (1<<WGM01) | (0<<WGM00);       // mode 2: Clear Timer on Compare Match (CTC)
    TCCR0B = (0<<WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00); // x8 prescaler -> 0.5µs
    OCR0A  = 63;                  // compare match register A (TOP) -> 32µs
    TIMSK0 = (1<<OCIE0A);         // Output Compare Match A Interrupt Enable (not working: TOIE1 with ISR TIMER0_TOIE1_vect because it is already defined by timing functions)
    
    // *****************************
    // ***** Timer1: VGA VSYNC *****
    // *****************************
    TCNT1  = 0;
    TCCR1A = (1<<COM1B1) | (1<<COM1B0) | (1<<WGM11) | (1<<WGM10);         // mode 15 (Fast PWM), set OC1B on Compare Match, clear OC1B at BOTTOM, controlling OC1B pin 10
    TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10); // x1024 prescaler -> 64µs
    OCR1A  = 259;                 // compare match register A (TOP) -> 16.64ms
    OCR1B  = 0;                   // compare match register B -> 64µs
    TIMSK1 = (1<<TOIE1);          // enable timer overflow interrupt setting vlines = 0
  
    // *************************************************
    // ***** Timer2: 74165 Parallel Load Enable PE *****
    // *************************************************
    TCNT2  = 0;
    TCCR2A = (1<<COM2A1) | (1<<COM2A0) | (1<<WGM21) | (1<<WGM20); // mode 7: Fast PWM, COM2A0=0: normal port HIGH, COM2A0=1: Toggle OC2A pin 11 on Compare Match
    TCCR2B = (1<<WGM22) | (0<<CS22) | (0<<CS21) | (1<<CS20) ;     // set x0 prescaler -> 62.5ns;
    OCR2A  = 7;                   // compare match register A (TOP) -> 250ns
    TIMSK2 = 0;                   // no interrupts here
  
    GTCCR = 0;                    // clear TSM => all timers start synchronously
    interrupts();
  }
  
  ISR(TIMER1_OVF_vect)            // timer1 overflow interrupt resets vline at HSYNC
  {
    vline = 0;
    frames++;
  }
		
  ISR(TIMER0_COMPA_vect)          // HSYNC generation and drawing of a scanline
  {
		asm("lds	r24, 0x00B2\n"			// compensate interrupt jitter
				"cpi	r24, 0x06\n"
				"breq	.+8\n"
				"cpi	r24, 0x05\n"
				"breq	.+6\n"
				"cpi	r24, 0x04\n"
				"breq	.+4\n"
				"nop\n"
				"nop\n"
				"nop\n"	);

    DDRD  = 0b00000000;           													// D = input
    PORTB = 0b00000010;           													// /HSYNC=0, /MR=1
    byte lin = ((vline++) >> 1) - 18;       								// skip 2 lines (VSYNC pulse) + 33 lines (vertical back porch)
		volatile byte* vrow = vram[lin >> 3];  									// pointer to the vram row 0...24 to display
    const byte* cset = charset[lin & 0b111] - 32; 					// pointer to the charset line 0..7 to use starting @ character 32
    byte scan = (PINC & 0b00111111) | (PIND & 0b11000000);	// read only after output has stabilized
    PORTB = 0b00010010;           													// /HSYNC=1, /MR=1
    DDRD  = 0b11111111;           													// D = output

    __builtin_avr_delay_cycles(4);// shift the whole picture to the right
  
    if (lin < 200)								// draw a line of pixels
    {  
      TCCR2A &= ~(1<<COM2A1);     // enable OC2A toggling pin 11					
      PORTD = cset[*vrow++];			// send 40 bytes
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      PORTD = cset[*vrow++];
      TCCR2A |= (1<<COM2A1);      // stop any further parallel load /PE (disabling OC2A toggling pin 11)
      PORTD = 0;
    }
    
    static byte prev = 0;         // remember the previous valid keyboard scan code
		if (scan != 0)
    {
			if ((vline & 7) == 0)				// every 256µs the 74HC173 keyboard register is sampled
			{
				if(scan == prev)          // was the data stable?
				{
					PORTB = 0b00010000;     // /HSYNC=1, /MR=0 
					PORTB = 0b00010010;     // /HSYNC=1, /MR=1 
					reg[regin++] = scan;    // queue of register data
					regin &= (SIZE-1);
				}
				prev = scan;
			}
    }
  }

  char getkey()
  {
    static bool alt = false;      // state of some important keys of the PS2 keyboard
    static bool shift = false;
    static bool release = false;  // indicating that the next key was released
    while(regout != regin)
    {
      byte scan = reg[regout++];  // process this character
      regout &= (SIZE-1);      
      switch (scan)
      {
        case 17: alt = !release; release = false; break;             // ALT, ALTGR
        case 18: case 89: shift = !release; release = false; break;  // SHIFT LEFT, SHIFT RIGHT     
        case 240: release = true; break;                             // key release indicator      
        default:                                                     // any other key
          if (release == true) release = false;
          else  									// key was pressed
          {
            byte s=0; if (shift) s = 1; else if (alt) s = 2;         // select bank of lookup according the states of special keys
            return pgm_read_byte_near(&LookupScanToASCII[s][scan & 127]);
          }
          break;
      }
    }
    return 0;
  }
  
  void fill(char c = 32)
  {
    char* p = (char*)&vram[0][0];
    for(int i=0; i<40*25; i++) *p++ = c;
  }
  
  void scroll()
  {
    volatile byte* p1 = &vram[0][0];
    volatile byte* p2 = &vram[1][0];
    for (int i=0; i<24*40; i++) *p1++ = *p2++;
    for (byte i=0; i<40; i++) *p1++ = 32;
  }

  void text(const String s, byte x, byte y)
  {    
    volatile byte* p = &vram[y][x];
    for (int i=0; i<s.length(); i++) p[i] = s[i];
  }
}

int main()                        // enforce main() loop w/o serial handler
{
  os::init();
  setup();
  while(true) loop();
}

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Carsten Herting
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
