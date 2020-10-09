#include "chip8.h"

chip8::chip8(){}

void chip8::chip8_initializesystem(){
// Resetting pointers and initializing PC.
 PC = 0x200;
 opcode = 0;
 I = 0;
 SP = 0;
 // Resetting timers
delaytimer = 0;
soundtimer = 0;
// fontsent 
for(int i = 0; i < 80; ++i){
  //  chip8_memory[i] = chip8_fontset[i];
}

}

void chip8::chip8_loadROM(std::string ROMname){
     std::vector<int8_t> lines;
     std::ifstream ROMinput;
     ROMinput.open(ROMname, std::ios::binary);
     while(ROMinput){
     std::string line;
     getline(ROMinput, line);
     lines.push_back(stoi(line));
     }

  for(int i = 0; i < lines.size(); ++i){
  chip8_memory[i + 512] = lines[i];
  }
}


void chip8::chip8_cpu(){
     // Fetching opcode
     opcode = chip8_memory[PC] << 8 | chip8_memory[PC+1];
     switch(opcode & 0xF000){   
     case 0x0000:
         switch(opcode & 0x0FFF){
            case 0x00E0:  //Clears the screen.
               //clear screen
               PC += 2;
               break;
            case 0x00EE:  //Returns from a subroutine.
               PC = chip8_stack[SP];
               SP--;          
               break;
            default: //0NNN -> Calls machine code routine (RCA 1802 for COSMAC VIP) at address NNN. Not necessary for most ROMs.
            
               break;                              
        }
        break; 
     case 0x1000:  //Jumps to address NNN.
        PC = opcode & 0x0FFF;
        break;
     case 0x2000:  //Calls subroutine at NNN.
        chip8_stack[SP] = PC;
        SP++;
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
          PC = opcode & 0x0FFF + chip8_registers[0];
        break; 
     case 0x8000:
        switch(opcode & 0x000F){
            case 0x0000:  //Sets VX to the value of VY.
                chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x00F0) >> 4];
                PC += 2;
               break;           
            case 0x0001:  //Sets VX to VX or VY. (Bitwise OR operation)
                chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[opcode & 0x0F00] | chip8_registers[(opcode & 0x00F0) >> 4];
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
               if(chip8_registers[(opcode & 0x00F0) >> 4] > (0xFF - chip8_registers[(opcode & 0x0F00) >> 8])){ // 255
                  chip8_registers[0xF] = 1;
               }
               else{
                  chip8_registers[0xF] = 0;
               }
               chip8_registers[opcode & 0x0F00 >> 8] += chip8_registers[opcode & 0x00F0 >> 4 ];
               PC += 2;
               break;
            case 0x0005:  //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
               if(chip8_registers[(opcode & 0x00F0) >> 4] < (0xFF - chip8_registers[(opcode & 0x0F00) >> 8])){ 
                  chip8_registers[0xF] = 0;
               }
               else{
                  chip8_registers[0xF] = 1;
               }
               chip8_registers[opcode & 0x0F00 >> 8] -= chip8_registers[opcode & 0x00F0 >> 4];
               PC += 2;
               break;
            case 0x0006:  //Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[b]
               chip8_registers[0xF] = chip8_registers[(opcode & 0x0F00) >> 8] & 00000001;
               chip8_registers[(opcode & 0x0F00) >> 8] <<= 1;
               PC += 2;
               break;
            case 0x0007:  //Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
               if(chip8_registers[(opcode & 0x0F00) >> 8] < (0xFF - chip8_registers[(opcode & 0x00F0) << 4])){ 
                  chip8_registers[0xF] = 0;
               }
               else{
                  chip8_registers[0xF] = 1;
               }
               chip8_registers[(opcode & 0x0F00) >> 8] = chip8_registers[(opcode & 0x00F0) >> 4]  - chip8_registers[(opcode & 0x0F00) >> 8];
               PC +=2;
               break;
            case 0x000E:  //Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[b]
               chip8_registers[0xF] = chip8_registers[(opcode & 0x0F00) >> 8] >> 7;
               chip8_registers[(opcode & 0x0F00) >> 8] <<= 1;
               PC +=2;
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
         srand(time(0));
         chip8_registers[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & (int)rand()%255;         
         PC += 2;
         break;
      case 0xD000:
         /*Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; 
           I value doesn’t change after the execution of this instruction.
          As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen*/
           // will be completed later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
         PC +=2;
         break; 
      case 0xE000:
         switch(opcode & 0x00FF){
            case 0x009E:  // Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
               if(keys[chip8_registers[(opcode & 0x0F00) >> 8]] == 1){
                  PC += 4;
               }
               else{
                  PC += 2;
               }
               break;
            case 0x00A1:  // Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
               if(keys[chip8_registers[(opcode & 0x0F00) >> 8]] == 0){
                  PC += 4;
               }
               else{
                  PC += 2;
               }
               break;
         }
         break;    
      case 0xF000:
         switch(opcode & 0x00FF){
            case 0x007:  //Sets VX to the value of the delay timer.
               chip8_registers[(opcode & 0x0F00) >> 8] = getdelayTimer();
               PC += 2;
               break;
            case 0x00A:  //A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
             /*A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)*/
             // will be completed later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
               break;  
            case 0x015:  //Sets the delay timer to VX.
               delaytimer = chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2;  
               break;
            case 0x018:  //Sets the sound timer to VX.
               soundtimer = chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2;  
               break;    
            case 0x01E:  //Adds VX to I. VF is not affected.
               I += chip8_registers[(opcode & 0x0F00) >> 8];
               PC += 2; 
               break;
            case 0x029:  //Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            /*Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.*/
             // will be completed later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
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
         }
         break;        

 }
}


int8_t chip8::getdelayTimer(){
   return delaytimer;
}
int8_t chip8::getsoundTimer(){
   return soundtimer;
}

int main(int argc, char **argv){






    return 0;
}