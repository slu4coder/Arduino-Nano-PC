# Arduino Nano Personal Computer - Squeezing Water from a Stone

I am pushing the limits of how much you can get out of a single Arduino Nano as few as possible additional logic ICs:

Sketches and schematics of my minimalistic 'Nano Personal Computer': 320x200 pixels of VGA monochrome (optional: 16 colors)
output and PS/2 interface on a single Arduino Nano with just 1 (2) logic IC. 

# HOW TO USE THE 'NANO HOME COMPUTER 2.0'

1. Set the Nano's fuses for output of the 16MHz system clock on pin D8 (see appendix A below).

2. Copy the library files 'os.h' and 'os.S' into a folder '../Arduino/libraries/os'.

3. Start a new sketch with '#include <os.h>'. The following global variables and functions are available:

        int os::frames                                    built-in 16-bit 60Hz frame counter
  
        byte os::vram[25][40]                             character video RAM array
  
        byte os::cram[25]                                 row color RAM array, available colors are:
  
                                                      WHITE, BLACK, GRAY, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,
                                                    
                                                      DKGRAY, DKRED, DKGREEN, DKYELLOW, DKBLUE, DKMAGENTA, DKCYAN.
                                                    
        os::text(String s, byte x, byte y, byte col = 0)  prints text at screen position (x, y), col = 0: don't set row color
  
        os::fill(char c = 32)                             fills the video RAM with character c
  
        os::scroll()                                      scrolls the video RAM one step upwards
  
        os::wait(int n)                                   waits for n frames
  
        byte os::getkey()                                 returns keystroke ASCII code (none: 0)
  
4. 26KB FLASH (1KB SRAM) of the Nano's 30KB FLASH (2KB SRAM) are usable, respectively.
   To save SRAM, define constant variables in FLASH memory by adding PROGMEM to the data type.
   Constant string arguments can be defined in FLASH by using the F("...") macro.

5. Have fun!

APPENDIX

A. Setting Arduino Nano's fuse bytes via ISP (in-situ programming) for output of system CLK

  o Connect two Arduino Nanos in the following way (programmer -> target):
    D13 -> D13, D12 -> D12, D11 -> D11, D10 -> RESET, 5V -> 5V, GND -> GND
    
  o Arduino IDE: Upload sketch 'examples/ArduinoISP' into programmer and select 'Tools/Programmer/Arduino as ISP'.
  
  o Open the file 'C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt'.
  
  o In section '## Arduino Nano w/ ATmega328P' (line 141), change line 149 'nano.menu.cpu.atmega328.bootloader.low_fuses=0xFF'
    (default) to '...=0xBF' (output system CLK on Pin D8 (B0)) and save the file.
    
  o Close and re-open the Arduino IDE for the changes to become active.
  
  o Select target board: 'Arduino Nano', processor: 'AtMega328P' and 'Tools/Burn bootloader'.

B. Hardware

  Pin A0 (C0):     keyboard input CLK
  
  Pin A1 (C1):     keyboard input DAT
  
  Pin A2-5 (C2-5): 4-bit color output to 74HC08 (optional)
  
  Pin D0-7 (D0-7): 8-bit pixel output to 74HC166
  
  Pin D8 (B0):     CLKO (16MHz system clock output)
  
  Pin D9 (B1):     unused
  
  Pin D10 (B2):    /VSYNC VGA (timer1, every 1/60s)
  
  Pin D11 (B3):    /PE 74166 pixel shift register synchroneous parallel load on rising edge of CP
  
  Pin D12 (B4):    /HSYNC VGA (inside ISR, every 32µs)
  
  Pin D13 (B5):    LED

Have fun!
