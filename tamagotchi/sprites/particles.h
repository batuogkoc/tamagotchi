#ifndef PARTICLES_H
#define PARTICLES_H
#import "sprite.h"

static const unsigned char PROGMEM diamond_buf[] = {
	0b00100000,
	0b01010000,
	0b10001000,
	0b01010000,
	0b00100000,
};
static Sprite diamond_sprite = {
	.h = 5,
	.w = 8,
	.buf = diamond_buf
};


static const unsigned char PROGMEM heart_buf[] = {
	0b01101100,
	0b10010010,
	0b01000100,
	0b00101000,
	0b00010000,
};
static Sprite heart_sprite = {
	.h = 5,
	.w = 8,
	.buf = heart_buf
};

static const unsigned char PROGMEM crumbs_buf[] = {
	0b10000000,
	0b00000000,
	0b00000000,
};
static Sprite crumbs_sprite = {
	.h = 3,
	.w = 8,
	.buf = crumbs_buf
};


#endif