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

#define BUTTON_0 12
#define BUTTON_1 13
#define BUTTON_2 26

#define DEBOUNCE_COUNTER_MAX 10
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
hw_timer_t *debounce_timer = NULL;

float x_offset = 0;
float y_offset = 0;
float x_speed = 0;
float y_speed = 0;
int x_borders = 32;
int y_borders = 16;

float deltaT = 0;

int animation_state = 0; 
long animation_end_time = 0;

int menu_state = 0;
long menu_reversion_time = 0;

float happiness=1;
float hunger=1;


volatile SemaphoreHandle_t button_0_semaphore;
volatile SemaphoreHandle_t button_1_semaphore;
volatile SemaphoreHandle_t button_2_semaphore;

struct Sprite* smiley_sprite = &smiley_happy;

void IRAM_ATTR debounce_timer_isr(){
  static long button_0_integrated = 0;
  static bool button_0_debounced = 0;
  if(digitalRead(BUTTON_0)){
    if(button_0_integrated<DEBOUNCE_COUNTER_MAX)
      button_0_integrated++;
  }
  else{
    if(button_0_integrated>0)
      button_0_integrated--;
  }

  if(button_0_integrated >= DEBOUNCE_COUNTER_MAX-2 && button_0_debounced == false){
    button_0_debounced = true;
    xSemaphoreGiveFromISR(button_0_semaphore, NULL);
  }

  if(button_0_integrated <= 2 && button_0_debounced == true){
    button_0_debounced = false;
  }

  static long button_1_integrated = 0;
  static bool button_1_debounced = 0;
  if(digitalRead(BUTTON_1)){
    if(button_1_integrated<DEBOUNCE_COUNTER_MAX)
      button_1_integrated++;
  }
  else{
    if(button_1_integrated>0)
      button_1_integrated--;
  }

  if(button_1_integrated >= DEBOUNCE_COUNTER_MAX-2 && button_1_debounced == false){
    button_1_debounced = true;
    xSemaphoreGiveFromISR(button_1_semaphore, NULL);
  }

  if(button_1_integrated <= 2 && button_1_debounced == true){
    button_1_debounced = false;
  }

  static long button_2_integrated = 0;
  static bool button_2_debounced = 0;
  if(digitalRead(BUTTON_2)){
    if(button_2_integrated<DEBOUNCE_COUNTER_MAX)
      button_2_integrated++;
  }
  else{
    if(button_2_integrated>0)
      button_2_integrated--;
  }

  if(button_2_integrated >= DEBOUNCE_COUNTER_MAX-2 && button_2_debounced == false){
    button_2_debounced = true;
    xSemaphoreGiveFromISR(button_2_semaphore, NULL);
  }

  if(button_2_integrated <= 2 && button_2_debounced == true){
    button_2_debounced = false;
  }
}

void increase_happiness(float d_happiness){
  happiness += d_happiness;
  if(happiness>1)
    happiness = 1;
  if(happiness<0)
    happiness = 0;
}

void increase_hunger(float d_hunger){
  hunger += d_hunger;
  if(hunger>1)
    hunger = 1;
  if(hunger<0)
    hunger = 0;
}

void setup()   {

  Serial.begin(9600);
  pinMode(BUTTON_0, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  debounce_timer = timerBegin(1000000);
  timerAttachInterrupt(debounce_timer, debounce_timer_isr);
  timerAlarm(debounce_timer, 1000, true, 0);
  button_0_semaphore = xSemaphoreCreateBinary();
  button_1_semaphore = xSemaphoreCreateBinary();
  button_2_semaphore = xSemaphoreCreateBinary();


  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
  display.clearDisplay();
  display.display();
  delay(2000);
  randomSeed(42);

  // Clear the buffer.
  
}

void loop() {
  long start_micros = micros();

  increase_happiness(-0.06*deltaT);
  increase_hunger(-0.01*deltaT);

  if(xSemaphoreTake(button_1_semaphore, 0)){
    increase_happiness(0.2);
  }
  if(xSemaphoreTake(button_2_semaphore, 0)){
    increase_hunger(0.2);
  }

  //menu state transitions
  if(menu_state == 0){
    if(xSemaphoreTake(button_0_semaphore, 0)){
      menu_state = 1;
      menu_reversion_time = millis() + 5000;
    }
  }
  else if(menu_state == 1){
    if(xSemaphoreTake(button_0_semaphore, 0)){
      menu_state = 0;
    }
    if(millis()>menu_reversion_time){
      menu_state = 0;
    }
  }

  //animation state transition
  if(animation_state == 0){
    if(animation_end_time < millis()){
      animation_state = 1;
      animation_end_time = random(1000, 3000) + millis();
      x_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(3, 5));
      y_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(1, 3));
    }
  }
  else if(animation_state == 1){
    if(animation_end_time < millis()){
      animation_state = 0;
      animation_end_time = random(1000, 3000) + millis();
      x_speed = 0;
    }
  }

  //logic inside of the state
  if(animation_state == 0){

  }
  else if(animation_state == 1){
    if(abs(x_offset)>=x_borders){
      x_speed*=-1;
    }
    x_offset += x_speed*deltaT;

    if(abs(y_offset)>=y_borders){
      y_speed*=-1;
    }
    y_offset += y_speed*deltaT;
  }


  if(happiness > 0){
    
    if(happiness>0.7){
      smiley_sprite = &smiley_happy;
    }
    else if(0.3<happiness && happiness<=0.7){
      smiley_sprite = &smiley_neutral;
    }
    else{
      smiley_sprite = &smiley_sad;
    }
  }

  
  if(menu_state == 0){
    display.clearDisplay();
    display.drawBitmap((int)(48+x_offset), (int)(16+y_offset), smiley_sprite->buf, smiley_sprite->h, smiley_sprite->w, SH110X_WHITE);
    
    display.display();
  }
  else if(menu_state == 1){
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(5, 10);
    display.print("Happiness");
    display.drawRect(5, 22, 128-10, 10, SH110X_WHITE);
    display.fillRect(7, 24, (128-14)*happiness, 6, SH110X_WHITE);

    display.setCursor(5, 36);
    display.print("Hunger");
    display.drawRect(5, 48, 128-10, 10, SH110X_WHITE);
    display.fillRect(7, 50, (128-14)*hunger, 6, SH110X_WHITE);

    display.display();
  }


  
  deltaT = ((float)(micros()-start_micros))/1000000.0f;
  //25 hz loop
  while(deltaT<0.004){;}
}
