#ifndef CHIP8_H
#define CHIP8_H

#define CODE_START	0x200
#define RAM_SIZE		0x1000
#define N_REGISTERS	0x10
#define STACK_SIZE	0x10


/* This structure defines the complete sate of the chip8 machine (CPU and RAM) */
typedef struct {
	
	unsigned short ram[RAM_SIZE];
	
	int romSize;
	
	/* General purpose registers */
	unsigned char rV[N_REGISTERS];
	
	/* 16 bit address register only the 12 least significant bits are used */
	unsigned short rI;
	
	/* Special purpose registers */
	unsigned char rDelay;
	unsigned char rSound;

	/*  Pseudo registers */
	unsigned short *pc;
	unsigned char *sp;
	
	unsigned short stack[STACK_SIZE];
} Chip8;

#endif
