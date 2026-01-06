#ifndef PARTICLE_H
#define PARTICLE_H

#import "sprite.h"

typedef struct Particle{
  uint64_t expiry_time = 0;
  float pos_x = 0.0f;
  float pos_y = 0.0f;
  float vel_x = 0.0f;
  float vel_y = 0.0f;
  Sprite* sprite = NULL;
};

#endif