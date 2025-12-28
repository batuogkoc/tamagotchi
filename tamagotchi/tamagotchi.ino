/*********************************************************************
  This is an example for our Monochrome OLEDs based on SH110X drivers

  This example is for a 128x64 size display using I2C to communicate
  3 pins are required to interface (2 I2C and one reset)

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada  for Adafruit Industries.
  BSD license, check license.txt for more information
  All text above, and the splash screen must be included in any redistribution

  i2c SH1106 modified by Rupert Hirst  12/09/21
*********************************************************************/



#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "sprites/smiley.h"

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
                         // e.g. the one with GM12864-77 written on it
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float x_offset = 0;
float y_offset = 0;
float x_speed = 4;
float y_speed = 10;

float deltaT = 0;

void setup()   {

  Serial.begin(9600);

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
  display.clearDisplay();
  display.display();
  delay(2000);

  // Clear the buffer.
  
}

void loop() {
  long start_micros = micros();
  int x_borders = 16;
  int y_borders = 9;

  if(abs(x_offset)>=x_borders){
    x_speed*=-1;
  }
  if(abs(y_offset)>=y_borders){
    y_speed*=-1;
  }
  x_offset += x_speed*deltaT;
  y_offset += y_speed*deltaT;

  display.clearDisplay();
  display.drawBitmap((int)(48+x_offset), (int)(16+y_offset), smiley_buf, SMILEY_COLS, SMILEY_ROWS, SH110X_WHITE);
  
  display.display();
  
  deltaT = ((float)(micros()-start_micros))/1000000.0f;
  //25 hz loop
  while(deltaT<0.004){;}
}
