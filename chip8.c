#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <SDL/SDL.h>

#include "chip8.h"

// TODO:
// score font does not show
// brix collision not detected
// no sound


int runChip8(Chip8 *chip8)
{
	unsigned short opcode;
	int tmp, tmp2;
	unsigned char spriteByte, spriteBit;
	unsigned char *screenBit;
	
	while(!chip8->halt) {
		printf("%4X:", chip8->pc);
		updateTimers(chip8);
		
		readInput(chip8);
		
		opcode = (((unsigned short) chip8->ram[chip8->pc]) << 8) | (chip8->ram[chip8->pc+1]);
		
		// Pre-compute / extract some potentially usefull information from the opcode
		unsigned short addr = opcode & 0x0FFF;
		unsigned char nibble = opcode & 0x000F;
		unsigned char x = (opcode & 0x0F00) >> 8;
		unsigned char y = (opcode & 0x00F0) >> 4;
		unsigned char byte = opcode & 0x00FF;
		
		switch(opcode & 0xF000) {
			
		case 0x0000:
			
			switch(opcode) {
				
				/* CLS: Clear the display  */
				case 0x00E0:
					memset(chip8->videoMem, 0, sizeof(chip8->videoMem));
					chip8->pc += 2;
					break;
				
				/* RET: Return from subroutine  */
				case 0x00EE:
					chip8->pc = chip8->stack[chip8->sp--];
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
			chip8->stack[++(chip8->sp)] = chip8->pc + 2;
			chip8->pc = addr;
			break;
		
		/* SE Vx, byte: Skip next instruction if Vx == kk */
		case 0x3000:
			if(chip8->rV[x] == byte) {
				chip8->pc += 4;
			} else{
				chip8->pc += 2;
			}
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
			switch(nibble) {
			
			/* LD Vx, Vy: Set Vx = Vy */
			case 0x0000:
				chip8->rV[x] = chip8->rV[y];
				chip8->pc += 2;
				break;
			
			/* OR Vx, Vy: Set Vx = Vx OR Vy */
			case 0x0001:
				chip8->rV[x] |= chip8->rV[y];
				chip8->pc += 2;
				break;
			
			/* AND Vx, Vy: Set Vx = Vx AND Vy */
			case 0x0002:
				chip8->rV[x] &= chip8->rV[y];
				chip8->pc += 2;
				break;
			
			/* XOR Vx, Vy: Set Vx = Vx XOR Vy */
			case 0x0003:
				chip8->rV[x] ^= chip8->rV[y];
				chip8->pc += 2;
				break;
			
			/* ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry */
			case 0x0004:
				tmp = chip8->rV[x] + chip8->rV[y];
				chip8->rV[x] = (unsigned char) tmp;
				if(tmp > 0xFF)
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
				if((chip8->rV[x] & 0x1))
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
				if((chip8->rV[x] & 0x80))
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
			chip8->rV[0xF] = 0;
				
			for(tmp=0; tmp<nibble; ++tmp) {
				spriteByte = chip8->ram[chip8->rI + tmp];
				for(tmp2=0; tmp2<8; ++tmp2) {
					spriteBit = (spriteByte >> (7-tmp2)) & 0x1;
					screenBit = &(chip8->videoMem[(((chip8->rV[y] + tmp) % SCREEN_HEIGHT) * SCREEN_WIDTH) + ((chip8->rV[x] + tmp2) % SCREEN_WIDTH)]);
					
					if(*screenBit && spriteBit) {
						chip8->rV[0xf] = 1;
					}
					
					*screenBit ^= spriteBit;
			    }
			}
			
			chip8->pc += 2;
			break;
		
		case 0xE000:
			switch(byte) {
				
			/* SKP Vx: Skip next instruction if the key with the value Vx is pressed */
			case 0x009E:
				if(((chip8->input >> chip8->rV[x]) & 1))
					chip8->pc += 4;
				else
					chip8->pc += 2;
				break;
			
			/* SKNP Vx: Skip next instruction if the key with the value Vx is not pressed */
			case 0x00A1:
				if(!((chip8->input >> chip8->rV[x]) & 1))
					chip8->pc += 4;
				else
					chip8->pc += 2;
				break;
			}
			break;
		
		case 0xF000:
			switch(byte) {
			
			/* LD Vx, DT: Set Vx = Delay timer value  */
			case 0x0007:
				chip8->rV[x] = chip8->rDelay;
				chip8->pc += 2;
				break;
			
			/* LD Vx, K: Wait for a keypress, load the value of key in Vx  */
			case 0x000A:
				if(chip8->input > 0) {
					tmp = 0;
					for(;;) {
						if((chip8->input >> tmp) & 1) break;
						++tmp;
					}
					chip8->rV[x] = tmp;
					chip8->pc += 2;
				}
				break;
			
			/* LD DT, Vx: Set Delay timer = Vx */
			case 0x0015:
				chip8->rDelay = chip8->rV[x];
				chip8->pc += 2;
				break;
			
			/* LD ST, Vx, Set Sound timer = Vx */
			case 0x0018:
				chip8->rSound = chip8->rV[x];
				chip8->pc += 2;
				break;
			
			/* ADD I, Vx: Set I = I + Vx */
			case 0x001E:
				chip8->rI += chip8->rV[x];
				chip8->pc += 2;
				break;
			
			/* LD F, Vx: Set I = location of the sprite for digit Vx */
			case 0x0029:
				chip8->rI = chip8->ram[chip8->rV[x] * FONT_LETTER_SIZE];
				chip8->pc += 2;
				break;
			
			/* LD B, Vx: Store BCD representation of Vx at memory location I, I+1, I+2 */
			case 0x0033:
				chip8->ram[chip8->rI] = (unsigned char) floor(chip8->rV[x] / 100);
				chip8->ram[chip8->rI+1] = (unsigned char) floor((chip8->rV[x] % 100) / 10);
				chip8->ram[chip8->rI+2] = (unsigned char) floor((chip8->rV[x] % 10));
				chip8->pc += 2;
				break;
			
			/* LD [I], Vx, Store register V0 through Vx in memory starting at location I */
			case 0x0055:
				memcpy(chip8->ram + chip8->rI, chip8->rV, x+1);
				chip8->pc += 2;
				break;
			
			/* LD Vx, [I]: Read registers V0 through Vx from memory starting from location I */
			case 0x0065:
				memcpy(chip8->rV, chip8->ram + chip8->rI, x+1);
				chip8->pc += 2;
				break;
			
			}	
			break;
		}

		// Debug
		printf("%4X ", opcode);
		if((opcode & 0xF000) == 0xD000) printf("\n");
		//if(chip8->input) printf("%d\n", chip8->input);

		draw(chip8);
	}

	return 0;
}


void readInput(Chip8 *chip8)
{
	SDL_Event event;
	int key;
	
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
			key = event.key.keysym.sym;
			
			/* Set the input bits */
			if(key >= 48 && key <= 57) { // '0' to '9'
				if(event.type == SDL_KEYDOWN)
					chip8->input |= (short) pow(2, key-48);
				else
					chip8->input &= ~((short) pow(2, key-48));
			} else if(key >= 97 && key <= 102) { // 'a' to 'f'
				if(event.type == SDL_KEYDOWN)
					chip8->input |= (short) pow(2, key-87);
				else
					chip8->input &= ~((short) pow(2, key-87));
			}
			
		}else if(event.type == SDL_QUIT) {
			chip8->halt = 1;
		}
	}
}


void updateTimers(Chip8 *chip8)
{
	if(chip8->rDelay > 0) (chip8->rDelay)--;
	if(chip8->rSound > 0) (chip8->rSound)--;
}


void playSound()
{
	//TODO
}


void draw(Chip8 *chip8)
{
	unsigned int *pixels = (unsigned int *) chip8->screen->pixels;
	
	int i, x, y, lineOffset, columnOffset;
	for(i=0; i<SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		lineOffset = (i / SCREEN_WIDTH) * SCREEN_WIDTH * pow(SCREEN_ZOOM, 2);
		columnOffset = (i % SCREEN_WIDTH) * SCREEN_ZOOM;
		for(y=0; y<SCREEN_ZOOM; ++y) {
			for(x=0; x<SCREEN_ZOOM; ++x) {	
				pixels[lineOffset + columnOffset + (y * SCREEN_WIDTH * SCREEN_ZOOM) + x] = chip8->videoMem[i] ? 0x00FFFFFF : 0x0;
			}
		}
	}
	
	SDL_Flip(chip8->screen);
	SDL_Delay(5); // 60Hz
}


int init(Chip8 *chip8, const char *roomPath)
{
	
	if(loadRom(chip8, roomPath) == 0) {
		
		/* Seed the RNG */
		srand(time(NULL));
		
		/* Load built-in font */
		int i;
		for(i=0; i<sizeof(font) / FONT_LETTER_SIZE; ++i) {
			memset(chip8->ram + i * FONT_LETTER_SIZE, font[i], FONT_LETTER_SIZE);
		}
		
		chip8->halt = 0;
		chip8->sp = -1; // The first call to a sub will increment sp and store the ret address at 0
		chip8->pc = CODE_START;
		memset(chip8->videoMem, 0, sizeof(chip8->videoMem));
		memset(chip8->rV, 0, sizeof(chip8->rV));
		chip8->input = 0;

		
		// Graphic stuff
		SDL_Init(SDL_INIT_EVERYTHING);
		chip8->screen = SDL_SetVideoMode(SCREEN_WIDTH * SCREEN_ZOOM, SCREEN_HEIGHT * SCREEN_ZOOM, SCREEN_BIT_DEPTH, SDL_SWSURFACE);
		
	} else {
		return -1;
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
	
	// TODO: Remove debug code
	//int i;
	//for(i=0; i<chip8->romSize/2; i+=2) {
	//	printf("%4X ", (chip8->ram[CODE_START + i] << 8) | (chip8->ram[CODE_START + i + 1]));
	//}
	
	fclose(fp);
	return 0;
}


int main(int argc, char **argv)
{
	
	Chip8 chip8;
	
	if(argc == 2) {
		if(init(&chip8, argv[1]) == -1) {
			return -1;
		} else {
			return runChip8(&chip8);
		}
	}
	
	printf("Usage: chip8 rom_file\n");
	return 0;
}

