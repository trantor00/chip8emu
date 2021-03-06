#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
#include <unistd.h>
#include <string.h>



class chip8{

private:
uint8_t chip8_memory[4096]; //(0x1000), 4096
uint16_t opcode;
uint8_t chip8_registers[16];  // V
uint16_t PC;
uint16_t I;
uint16_t chip8_stack[16];
uint16_t SP;
bool draw;
uint8_t delaytimer;  // Delay timer: This timer is intended to be used for timing the events of games. Its value can be set and read.
uint8_t soundtimer;  // Sound timer: This timer is used for sound effects. When its value is nonzero, a beeping sound is made.
// CHIP-8 has two timers. They both count down at 60 hertz, until they reach 0.


public:
chip8();
uint8_t chip8_keys[16];
uint32_t chip8_gfx[64 * 32];  // I need opengl for graphics and sounds!
void chip8_cpu();
void chip8_initializesystem();
void chip8_loadROM(const char * ROMname);
uint8_t getdelayTimer();
uint8_t getsoundTimer();
bool getdraw();
void setdraw(bool d);


};









/*



Input
Input is done with a hex keyboard that has 16 keys ranging 0 to F. The '8', '4', '6', and '2' keys are typically used for directional input. Three opcodes are used to detect input. 
One skips an instruction if a specific key is pressed, while another does the same if a specific key is not pressed. The third waits for a key press, and then stores it in one of the data registers.

Graphics and sound
Original CHIP-8 Display resolution is 64×32 pixels, and color is monochrome. Graphics are drawn to the screen solely by drawing sprites, which are 8 pixels wide and may be from 1 to 16 pixels in height. Sprite pixels are XOR'd with corresponding screen pixels. 
In other words, sprite pixels that are set flip the color of the corresponding screen pixel, while unset sprite pixels do nothing. The carry flag (VF) is set to 1 if any screen pixels are flipped from set to unset when a sprite is drawn and set to 0 otherwise. This is used for collision detection.

As previously described, a beeping sound is played when the value of the sound timer is nonzero.

*/


