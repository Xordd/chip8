#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"


int runChip8(Chip8 *chip8)
{
	/* Seed the RNG */
	srand(time(NULL));
	
	/* Load built-in font */
	int i;
	for(i=0; i<0x10; ++i) {
		memset(font(chip8, i), font[i], FONT_LETTER_SIZE);
	}
	
	/* Start code execution */
	chip8->sp = -1;
	chip8->pc = CODE_START;
	unsigned short opcode;
	int temp;
	
	for(;;) {
		opcode = (((unsigned short) chip8->ram[chip8->pc]) << 8) | (chip8->ram[chip8->pc+1]);
		
		// Pre-compute / extract some potentially usefull information from the opcode
		unsigned short addr = opcode & 0x0FFF;
		unsigned char nibble = opcode & 0x000F;
		unsigned char x = (opcode & 0x0F00) >> 8;
		unsigned char y = (opcode & 0x00F0) >> 4;
		unsigned char byte = opcode & 0x00FF;
		
		switch(opcode & 0xF000) {
			
			case 0xEEEE:
				switch(opcode) {
					
					/* CLS: Clear the display  */
					case 0x00E0:
						// TODO
						break;
					
					/* RET: Return from subroutine  */
					case 0x00EE:
						chip8->pc = chip8->stack[chip8->sp];
						chip8->sp--;
						break;
					
					/* SYS addr: Jump to a machine code routine at nnn  */
					default:
						chip8->pc = opcode;
						break;
				
				}
				break;
			
			/* JP addr: Jump to location nnn */
			case 0x1000:
				chip8->pc = addr;
				break;
			
			/* Call addr: Call subroutine at nnn */
			case 0x2000:
				chip8->sp++;
				chip8->stack[chip8->sp] = chip8->pc;
				chip8->pc = addr;
				break;
			
			/* SE Vx, byte: Skip next instruction if Vx == kk */
			case 0x3000:
				if(chip8->rV[x] == byte)
					chip8->pc += 4;
				else
					chip8->pc += 2;
				break;
			
			/* SNE Vx, byte: Skip next instruction if Vx != kk */
			case 0x4000:
				if(chip8->rV[x] != byte)
					chip8->pc += 4;
				else
					chip8->pc += 2;
				break;

			/* SE Vx, Vy: Skip next instruction if Vx == Vy */
			case 0x5000:
				if(chip8->rV[x] == chip8->rV[y])
					chip8->pc += 4;
				else
					chip8->pc += 2;
				break;

			/* LD Vx, byte: Set Vx to kk */
			case 0x6000:
				chip8->rV[x] = byte;
				chip8->pc += 2;
				break;

			/* ADD Vx, byte: Set Vx to Vx + kk */
			case 0x7000:
				chip8->rV[x] += byte;
				chip8->pc += 2;
				break;
			
			case 0x8000:
				switch(opcode & 0x000F) {
					
					/* LD Vx, Vy: Set Vx = Vy */
					case 0x0000:
						chip8->rV[x] = chip8->rV[y];
						chip8->pc += 2;
						break;
					
					/* OR Vx, Vy: Set Vx = Vx OR Vy */
					case 0x0001:
						chip8->rV[x] = chip8->rV[x] | chip8->rV[y];
						chip8->pc += 2;
						break;
					
					/* AND Vx, Vy: Set Vx = Vx AND Vy */
					case 0x0002:
						chip8->rV[x] = chip8->rV[x] & chip8->rV[y];
						chip8->pc += 2;
						break;
					
					/* XOR Vx, Vy: Set Vx = Vx XOR Vy */
					case 0x0003:
						chip8->rV[x] = chip8->rV[x] ^ chip8->rV[y];
						chip8->pc += 2;
						break;
					
					/* ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry */
					case 0x0004:
						temp = chip8->rV[x] + chip8->rV[y];
						chip8->rV[x] = (unsigned char) temp;
						if(temp > 255)
							chip8->rV[0xF] = 1;
						else
							chip8->rV[0xF] = 0;
						chip8->pc += 2;
						break;
					
					/* SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow */
					case 0x0005:
						if(chip8->rV[x] > chip8->rV[y])
							chip8->rV[0xF] = 1;
						chip8->rV[x] = chip8->rV[x] - chip8->rV[y];
						chip8->pc += 2;
						break;
					
					/* SHR Vx {, Vy}: Set Vx = Vx SHR 1 */
					case 0x0006:
						if((chip8->rV[x] & 0x1) == 1)
							chip8->rV[0xF] = 1;
						else
							chip8->rV[0xF] = 0;
						chip8->rV[x] /= 2;
						chip8->pc += 2;
						break;
					
					/* SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow */
					case 0x0007:
						if(chip8->rV[y] > chip8->rV[x])
							chip8->rV[0xF] = 1;
						chip8->rV[x] = chip8->rV[y] - chip8->rV[x];
						chip8->pc += 2;
						break;
					
					/* SHL Vx {, Vy}: Set Vx = SHL 1 */
					case 0x000E:
						if((chip8->rV[x] & 0x80) == 1)
							chip8->rV[0xF] = 1;
						else
							chip8->rV[0xF] = 0;
						chip8->rV[x] *= 2;
						chip8->pc += 2;
						break;
				}
				break;
			
			/* SNE Vx, Vy: Skip next instruction if Vx != Vy */
			case 0x9000:
				if(chip8->rV[x] != chip8->rV[y])
					chip8->pc += 4;
				else
					chip8->pc += 2;
				break;
			
			/* LD I, addr: Set I = nnn */
			case 0xA000:
				chip8->rI = addr;
				chip8->pc += 2;
				break;
			
			/* JP V0, nnn: Jump to location nnn + V0 */
			case 0xB000:
				chip8->pc = addr + chip8->rV[0];
				break;
			
			/* RND Vx, byte: Set Vx = random byte AND kk */
			case 0xC000:
				chip8->rV[x] = ((unsigned char) rand() % 256) & byte;
				chip8->pc += 2;
				break;
			
			/* DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision */
			case 0xD000:
				// TODO
				chip8->pc += 2;
				break;
			
			case 0xE000:
				switch(opcode & 0x00FF) {
					
					/*  */
					case 0x009E:
					break;
					
					/*  */
					case 0x00A1:
					break;
				}
				break;
			
			/* TODO */
			case 0xF000:
				break;
		}
		
		printf("%4X ", opcode);
		chip8->pc += 2;
		if(chip8->pc >= CODE_START + chip8->romSize) break;
	}

	return 0;
}


int loadRom(Chip8 *chip8, const char *romPath)
{	
	FILE *fp = fopen(romPath, "rb");
	
	fseek(fp, 0L, SEEK_END);
	chip8->romSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	fread(chip8->ram + CODE_START, sizeof(unsigned char), chip8->romSize, fp);
	
	fclose(fp);
	return 0;
}


int main(int argc, char **argv)
{
	
	Chip8 chip8;
	
	if(argc > 1) {
		if(loadRom(&chip8, argv[1]) == -1) {
			return -1;
		} 
		if(runChip8(&chip8) == -1) {
			return -1;
		}
	}
	
	return 0;
}
