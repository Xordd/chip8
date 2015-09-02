#ifndef CHIP8_H
#define CHIP8_H

#define CODE_START        0x200
#define RAM_SIZE          0x1000
#define N_REGISTERS       0x10
#define SCREEN_WIDTH      64
#define SCREEN_HEIGHT     32
#define SCREEN_BIT_DEPTH  32
#define SCREEN_ZOOM       10
#define FONT_LETTER_SIZE  5

/* The original implementation had a stack depth of 12  */
#define STACK_SIZE        0x10


/* This structure defines the complete sate of the chip8 machine (CPU and RAM) */
typedef struct {
	
	unsigned char halt;
	
	unsigned char ram[RAM_SIZE];
	
	int romSize;
	
	/* General purpose registers */
	unsigned char rV[N_REGISTERS];
	
	/* 16 bit address register only the 12 least significant bits are used */
	unsigned short rI;
	
	/* Special purpose registers */
	unsigned char rDelay;
	unsigned char rSound;

	/*  Pseudo registers */
	unsigned short pc; // Only the 12 lowest bits are used
	unsigned short sp;
	
	unsigned short input; // Each bits are turn on or off to reflect the sate of the input
	
	unsigned char videoMem[SCREEN_WIDTH * SCREEN_HEIGHT];
	SDL_Surface *screen;
	
	unsigned short stack[STACK_SIZE];
} Chip8;



unsigned char font[] = {
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
	0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};


int runChip8(Chip8 *chip8);

void updateTimers(Chip8 *chip8);

void readInput(Chip8 *chip8);

void draw(Chip8 *chip8);

int init(Chip8 *chip8, const char *roomPath);

int loadRom(Chip8 *chip8, const char *romPath);


#endif

