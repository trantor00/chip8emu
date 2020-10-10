#include "chip8.h"

chip8::chip8(){}
uint8_t chip8_fontset[80] ={ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
void chip8::chip8_initializesystem(){
// Resetting pointers and initializing PC.
 PC = 0x200;
 opcode = 0;
 I = 0;
 SP = 0;
 // Resetting timers
delaytimer = 0;
soundtimer = 0;
    // setting memories 0
   memset(chip8_gfx, 0, sizeof(chip8_gfx));
	memset(chip8_stack, 0, sizeof(chip8_stack));
   memset(chip8_keys, 0, sizeof(chip8_keys));
   memset(chip8_memory, 0, sizeof(chip8_memory));

// fontsent 
      for(int i = 0; i < 80; ++i){
    chip8_memory[i+ 0x50] = chip8_fontset[i];
}
//draw = true;
srand(time(NULL));
}

void chip8::chip8_loadROM(const char * ROMname){
    std::ifstream FILE(ROMname, std::ios::binary | std::ios::ate);
    if (FILE){
		std::streampos size = FILE.tellg();
		char* buffer = new char[size];

		FILE.seekg(0, std::ios::beg);
		FILE.read(buffer, size);
		FILE.close();

		for (long i = 0; i < size; ++i){
			chip8_memory[512 + i] = buffer[i];
		}		
		delete[] buffer;
	}
}

void chip8::chip8_cpu(){  
     // Fetching opcode
     opcode = chip8_memory[PC] << 8 | chip8_memory[PC + 1];
     switch(opcode & 0xF000){   
     case 0x0000:
         switch(opcode & 0x0FFF){
            case 0x00E0:  //Clears the screen.
              memset(chip8_gfx, 0, sizeof(chip8_gfx));
              // draw = true;
               PC += 2;
               break;
            case 0x00EE:  //Returns from a subroutine.
               SP--;   
               PC = chip8_stack[SP];    
               break;
            default: //0NNN -> Calls machine code routine (RCA 1802 for COSMAC VIP) at address NNN. Not necessary for most ROMs.
                   std::cerr << "cerr : " << opcode <<  std::endl;   
                   break;                               
        }
        break; 
     case 0x1000:  //Jumps to address NNN.
        PC = opcode & 0x0FFF;
        break;
     case 0x2000:  //Calls subroutine at NNN.
        chip8_stack[SP] = PC;
        ++SP;
        PC = opcode & 0x0FFF; 
        break;
     case 0x3000:  //Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
        if((opcode & 0x00FF) == (chip8_registers[(opcode & 0x0F00) >> 8])){
           PC += 4;
        }
        else{
           PC += 2;
        }
        break;  
     case 0x4000:  //Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
        if((opcode & 0x00FF) != (chip8_registers[(opcode & 0x0F00) >> 8])){
           PC += 4;
        }
        else{
           PC += 2;
        }        
        break;
     case 0x5000:  //Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
        if(chip8_registers[(opcode & 0x00F0) >> 4] == chip8_registers[(opcode & 0x0F00) >> 8]){
           PC += 4;
        }
        else{
           PC += 2;
        }        
        break;      
     case 0x6000:  //Sets VX to NN.
        chip8_registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        PC += 2;
        break; 
     case 0x7000:  //Adds NN to VX. (Carry flag is not changed)
        chip8_registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        PC += 2;
        break;                   
     case 0xA000:  //I = NNN	Sets I to the address NNN.
         I = opcode & 0x0FFF;
         PC += 2;
        break;       
     case 0xB000:  //Jumps to the address NNN plus V0.
          PC = (opcode & 0x0FFF) + chip8_registers[0];
        break; 
     case 0x8000:
        switch(opcode & 0x000F){
            case 0x0000:  //Sets VX to the value of VY.
                chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x00F0) >> 4];
                PC += 2;
               break;           
            case 0x0001:  //Sets VX to VX or VY. (Bitwise OR operation)
                chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x0F00) >> 8] | chip8_registers[(opcode & 0x00F0) >> 4];
                PC += 2;
               break;
            case 0x0002:  //Sets VX to VX and VY. (Bitwise AND operation)
                chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x0F00) >> 8] & chip8_registers[(opcode & 0x00F0) >> 4];
                PC += 2;
               break;               
            case 0x0003:  //Sets VX to VX xor VY. 
                chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x0F00) >> 8] ^ chip8_registers[(opcode & 0x00F0) >> 4];
                PC += 2;
               break;
            case 0x0004:  //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. 
              { uint16_t carry = chip8_registers[(opcode & 0x0F00) >> 8] + chip8_registers[(opcode & 0x00F0) >> 4] ;
               if(carry > 255){ 
                  chip8_registers[0xF] = 1;
               }
               else{
                  chip8_registers[0xF] = 0;
               }
               chip8_registers[opcode & 0x0F00 >> 8] += chip8_registers[opcode & 0x00F0 >> 4];
               PC += 2;
               break;}
            case 0x0005:  //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
               if(chip8_registers[(opcode & 0x0F00) >> 8] > chip8_registers[(opcode & 0x00F0) >> 4]){ 
                  chip8_registers[0xF] = 1;
               }
               else{
                  chip8_registers[0xF] = 0;
               }
               chip8_registers[opcode & 0x0F00 >> 8] -= chip8_registers[opcode & 0x00F0 >> 4];
               PC += 2;
               break;
            case 0x0006:  //Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[b]
               chip8_registers[0xF] = chip8_registers[(opcode & 0x0F00) >> 8] & 0x1;
               chip8_registers[(opcode & 0x0F00) >> 8] >>= 1;
               PC += 2;
               break;
            case 0x0007:  //Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
               if(chip8_registers[(opcode & 0x0F00) >> 8] < chip8_registers[(opcode & 0x00F0) >> 4]){ 
                  chip8_registers[0xF] = 1;
               }
               else{
                  chip8_registers[0xF] = 0;
               }
               chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x00F0) >> 4]  - chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2;
               break;
            case 0x000E:  //Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[b]
               chip8_registers[0xF] = chip8_registers[(opcode & 0x0F00) >> 8] >> 7;
               chip8_registers[(opcode & 0x0F00) >> 8] <<= 1;
               PC +=2;
               break;   
                default : 
                  std::cerr << "cerr : " << opcode <<  std::endl;       
                  break; 
        }
         break;
      case 0x9000:  //Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
        if(chip8_registers[(opcode & 0x00F0) >> 4] != chip8_registers[(opcode & 0x0F00) >> 8]){
           PC += 4;
        }
        else{
           PC += 2;
        }        
        break;
      case 0xC000:  //Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
         srand(time(NULL));
         chip8_registers[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & rand()%255;         
         PC += 2;
         break;
      case 0xD000:{/*Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; 
           I value doesn’t change after the execution of this instruction.
          As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen*/
     /*    uint16_t x = chip8_registers[(opcode & 0x0F00) >> 8];
         uint16_t y = chip8_registers[(opcode & 0x00F0) >> 4];
         uint16_t height = opcode & 0x000F;
         chip8_registers[0xF] = 0;
        
         for(int h = 0; h < height; h++){
           uint16_t newpixel = chip8_memory[I + h];
            for(int w = 0; w < 8; w++){
               if((newpixel & (0x80 >> w)) != 0){{
                   if(chip8_gfx[(x + w + ((y + h) * 64))] == 1){
                       chip8_registers[0xF] = 1; 
                   }
      }
                chip8_gfx[(x + w + ((y + h) * 64)) % (64 * 32)] ^= 1; // setting pixels
          }
   }
}
         draw = true;
         PC +=2; */
   uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // found this implementation on internet but haven't tried yet...
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = chip8_registers[Vx] % 64;
	uint8_t yPos = chip8_registers[Vy] % 32;

	chip8_registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = chip8_memory[I + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &chip8_gfx[(yPos + row) * 64 + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				
				if (*screenPixel == 0xFFFFFFFF)
				{
					chip8_registers[0xF] = 1;
				}

				
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}            
PC +=2;

         break; }
      case 0xE000:
         switch(opcode & 0x00FF){
            case 0x009E:  // Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
               if(chip8_keys[chip8_registers[(opcode & 0x0F00) >> 8]] != 0){
                  PC += 4;
               }
               else{
                  PC += 2;
               }
               break;
            case 0x00A1:  // Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
               if(chip8_keys[chip8_registers[(opcode & 0x0F00) >> 8]] == 0){
                  PC += 4;
               }
               else{
                  PC += 2;
               }
               break;
                default : 
                  std::cerr << "cerr : " << opcode <<  std::endl;
                  break; 
         }
         break;   
      case 0xF000:
         switch(opcode & 0x00FF){
            case 0x0007:  //Sets VX to the value of the delay timer.
               chip8_registers[(opcode & 0x0F00) >> 8] = delaytimer;
               PC += 2;
               break;
            case 0x000A:{   /*A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)*/
              bool keyPress = false;
					for(int i = 0; i < 16; ++i){
						if(chip8_keys[i] != 0){
							chip8_registers[(opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}
					if(!keyPress){
                 PC -= 2;
               }					
						
               PC += 2;
               break;  }
            case 0x0015:  //Sets the delay timer to VX.
               delaytimer = chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2;  
               break;
            case 0x0018:  //Sets the sound timer to VX.
               soundtimer = chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2;  
               break;    
            case 0x001E:  //Adds VX to I. VF is not affected.
               I += chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2; 
               break;
            case 0x0029: //Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
               I = 0x50 + (chip8_registers[(opcode & 0x0F00) >> 8] * 0x5);            
               PC += 2;            
               break; 
            case 0x033:  //Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
               chip8_memory[I] = chip8_registers[(opcode & 0x0F00) >> 8] / 100;
               chip8_memory[I + 1] = (chip8_registers[(opcode & 0x0F00) >> 8] / 10) % 10;
               chip8_memory[I + 2] = (chip8_registers[(opcode & 0x0F00) >> 8] % 100) % 10;
               PC += 2;          
               break; 
            case 0x055:  //Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            for(int i= 0; i <= ((opcode & 0x0F00) >> 8); i++){
              chip8_memory[I + i] = chip8_registers[i];
            }
               PC += 2;          
               break; 
            case 0x065:  //	Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
            for(int i= 0; i <= ((opcode & 0x0F00) >> 8); i++){
             chip8_registers[i] = chip8_memory[I + i];
            }
               PC += 2;         
               break;   
                default : 
                  std::cerr << "cerr : " << opcode <<  std::endl; 
                  break;                                                               
         }        
         break;
            default : 
               std::cerr << "cerr : " << opcode <<  std::endl;  
               break;     
 }

 if(delaytimer > 0){
    --delaytimer;}
 
  if(soundtimer > 0){
    if(soundtimer == 1)
      printf("BEEP!\n");
    --soundtimer;
}
}


uint8_t chip8::getdelayTimer(){
   return delaytimer;
}
uint8_t chip8::getsoundTimer(){
   return soundtimer;
}
bool chip8::getdraw(){
return draw;
}
void chip8::setdraw(bool d){
   draw = d;
}


/*int main(int argc, char **argv){

    return 0;
}*/