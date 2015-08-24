#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"


int runChip8(Chip8 *chip8)
{
	chip8->pc = chip8->ram + CODE_START / sizeof(short);
	for(;;) {
		switch(*(chip8->pc)) {
			default:
				printf("%4X ", *(chip8->pc));
				chip8->pc++;
				break;
		}
		if(chip8->pc >= (chip8->ram + (CODE_START + chip8->romSize) / sizeof(short)))
			break;
	}
	
	return 0;
}


int loadRom(Chip8 *chip8, const char *romPath)
{	
	FILE *fp = fopen(romPath, "rb");
	
	fseek(fp, 0L, SEEK_END);
	chip8->romSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	fread(chip8->ram + CODE_START / sizeof(short), sizeof(short), chip8->romSize / sizeof(short), fp);
	
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
