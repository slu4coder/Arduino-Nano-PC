# Squeezing Blood from a Stone: The Arduino Nano Personal Computer

I am pushing the limits of how much you can get out of a single Arduino Nano and as few as possible additional logic:

Sketches and schematics of my minimalistic 'Arduino Nano Personal Computer':

o 320x200 pixels of VGA monochrome (optional: 16 colors) output

o PS/2 keyboard interface

o runs on a single Arduino Nano with just 1 (2) logic IC. 

The following global variables and functions are provided by the library 'os' (use '#include <os.h>'):

	int os::frames                                  built-in 16-bit 60Hz frame counter

	byte os::vram[25][40] 			        character video RAM array

	byte os::cram[25]                               row color RAM array, available colors are:

       						WHITE, BLACK, GRAY, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,
			    
							DKGRAY, DKRED, DKGREEN, DKYELLOW, DKBLUE, DKMAGENTA, DKCYAN.
		
	os::text(String s, byte x, byte y, byte col=0)  prints text at screen position (x, y) [optional: sets row color]

	os::fill(char c=32)                             clears the video RAM [optional: fills video RAM with character c]

	os::scroll()                                    scrolls the video RAM one step upwards

	os::wait(int n)					waits for n frames

	byte os::getkey()                               returns keystroke ASCII code (none: 0)

Have fun!
