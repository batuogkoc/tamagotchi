struct Particle{
  uint64_t expiry_time;
  float pos_x;
  float pos_y;
  float vel_x;
  float vel_y;
  (static const unsigned char)* bitmap;
}