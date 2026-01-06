/*********************************************************************
  This is an example for our Monochrome OLEDs based on SH110X drivers
c:\Batu\Projects\tamagotchi\tamagotchi\sprites\particles.h
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
#include "sprites/particles.h"

#include "sprite.h"
#include "particle.h"


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

#define MAX_PARTICLE_NUM 20

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

enum AnimationState {idle, wandering, beingInteractedWith, beingFed};
enum AnimationState animation_state = idle; 
long animation_end_time = 0;

enum MenuState {mainMenu, statsMenu}; 
enum MenuState menu_state = mainMenu;

enum SmileyState {happy, neutral, sad, hungry, dead};
enum SmileyState smiley_state = happy;

long menu_reversion_time = 0;

float happiness=1;
float hunger=1;


volatile SemaphoreHandle_t button_0_semaphore;
volatile SemaphoreHandle_t button_1_semaphore;
volatile SemaphoreHandle_t button_2_semaphore;

Sprite* smiley_sprite = &smiley_happy_sprite;

Particle particles[MAX_PARTICLE_NUM];

void IRAM_ATTR debounce_timer_isr(){
  static long button_0_integrated = DEBOUNCE_COUNTER_MAX;
  static bool button_0_debounced = true;
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

  static long button_1_integrated = DEBOUNCE_COUNTER_MAX;
  static bool button_1_debounced = true;
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

  static long button_2_integrated = DEBOUNCE_COUNTER_MAX;
  static bool button_2_debounced = true;
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

bool spawn_particle(uint64_t expiry_time, float pos_x, float pos_y, float vel_x, float vel_y, Sprite* sprite){
  for(int i=0; i<MAX_PARTICLE_NUM; i++){
    if(particles[i].expiry_time<millis()){
      particles[i].expiry_time = expiry_time;
      particles[i].pos_x = pos_x;
      particles[i].pos_y = pos_y;
      particles[i].vel_x = vel_x;
      particles[i].vel_y = vel_y;
      particles[i].sprite = sprite;
      return true;
    }
  }
  return false;
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
  increase_hunger(-0.03*deltaT);

  if(happiness>0.7){
    smiley_state = happy;
  }
  else if(0.3<happiness && happiness<=0.7){
    smiley_state = neutral;
  }
  else{
    smiley_state = sad;
  }

  if(hunger<0.2){
    smiley_state = hungry;
  }
  if (hunger<0.001){
    smiley_state = dead;
  }

  if(xSemaphoreTake(button_1_semaphore, 0)){
    if(smiley_state != hungry && smiley_state != dead){
      increase_happiness(0.2);
      for(int i = 0; i<random(3,5); i++){
        spawn_particle(millis() + random(1000, 3000), 48+x_offset+random(0, 32), 16+y_offset+random(0, 32), 0, -((float)random(300, 700))/100.0f , &heart_sprite);
      }
    }

  }
  if(xSemaphoreTake(button_2_semaphore, 0)){
    if(smiley_state != dead){
          increase_hunger(0.2);
      for(int i = 0; i<random(3,5); i++){
        spawn_particle(millis() + random(5000, 15000), 48+x_offset+random(0, 32), 40+y_offset+random(0, 8), 0, 0, &crumbs_sprite);
      }
    }
  }

  //menu state transitions
  if(menu_state == mainMenu){
    if(xSemaphoreTake(button_0_semaphore, 0)){
      menu_state = statsMenu;
      menu_reversion_time = millis() + 5000;
    }
  }
  else if(menu_state == statsMenu){
    if(xSemaphoreTake(button_0_semaphore, 0)){
      menu_state = mainMenu;
    }
    if(millis()>menu_reversion_time){
      menu_state = mainMenu;
    }
  }

  //animation state transition
  if(animation_state == idle){
    if(animation_end_time < millis()){

      switch (smiley_state) {
        case happy:
        case neutral:
        default:
          animation_state = wandering;
          animation_end_time = random(1000, 3000) + millis();
          x_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(3, 5));
          y_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(1, 3));
        break;

        case sad:
          animation_state = wandering;
          animation_end_time = random(3000, 5000) + millis();
          x_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(10, 20)/10.0f);
          y_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(5, 10)/10.0f);
        break
        case hungry:
          animation_state = wandering;
          animation_end_time = random(3000, 5000) + millis();
          x_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(10, 20)/10.0f);
          y_speed = (((float)random(0,2))*2.0f-1.0f)*((float)random(5, 10)/10.0f);
        break;

        case dead:
          //do nothing as you are dead
        break;

      }
      
    }
  }
  else if(animation_state == wandering){
    if(animation_end_time < millis()){
        x_speed = 0;
        y_speed = 0;
        case happy:
        case neutral:
        default:
          animation_end_time = random(1000, 3000) + millis();
          animation_state = idle;
        break;

        case sad:
        case hungry:
          animation_end_time = random(1000, 3000) + millis();
          animation_state = idle;
        break;

        case dead:
          //do nothing as you are dead
        break;
      

    }
  }

  for(int i = 0; i<MAX_PARTICLE_NUM; i++){
    if(particles[i].expiry_time>millis()){
      particles[i].pos_x += particles[i].vel_x * deltaT;
      particles[i].pos_y += particles[i].vel_y * deltaT;
    }
  }
  

  //logic inside of the state
  if(animation_state == idle){

  }
  else if(animation_state == wandering){
    if(abs(x_offset)>=x_borders){
      x_speed*=-1;
    }
    x_offset += x_speed*deltaT;

    if(abs(y_offset)>=y_borders){
      y_speed*=-1;
    }
    y_offset += y_speed*deltaT;
  }

  switch (smiley_state)
  {
	case happy:
		smiley_sprite = &smiley_happy_sprite;
		break;
	case neutral:
		smiley_sprite = &smiley_neutral_sprite;
		break;
	case sad:
		smiley_sprite = &smiley_sad_sprite;
		break;
	case hungry:
		smiley_sprite = &smiley_hungry_sprite;
		break;
	case dead:
		smiley_sprite = &smiley_dead_sprite;
		break;

  default:
    smiley_sprite = &smiley_neutral_sprite;
    break;
  }

  
  if(menu_state == mainMenu){
    display.clearDisplay();
    display.drawBitmap((int)(48+x_offset), (int)(16+y_offset), smiley_sprite->buf, smiley_sprite->w, smiley_sprite->h, SH110X_WHITE);
    
    
    for(int i = 0; i<MAX_PARTICLE_NUM; i++){
      if(particles[i].expiry_time>millis() && particles[i].sprite != NULL){
        display.drawBitmap((int)particles[i].pos_x, (int)particles[i].pos_y, particles[i].sprite->buf, particles[i].sprite->w, particles[i].sprite->h, SH110X_WHITE);
      }
    }

    display.display();
  }
  else if(menu_state == statsMenu){
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
