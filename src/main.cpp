#include <Arduino.h>
#include <TrafficLight.h>

// -------------------------------------------------------------
//                        Global constant
// -------------------------------------------------------------
#define NUM_MODE             4
#define STANDARD_MODE        0
#define YELLOW_BLINK_MODE    1
#define SETUP_RED            2
#define SETUP_GREEN          3
#define NUM_LIGHT            2
#define LIGHT_1              0
#define LIGHT_2              1
#define TIMES_FLASH          80
#define FLASH_MS             5
#define TIME_DEBOUNCE_US     20


// -------------------------------------------------------------------------------------
//                        Global variable
//
// mode = {STANDARD_MODE, YELLOW_BLINK_MODE, YELLOW_BRIGHT_MODE, SETUP_RED, SETUP_GREEN} indicates current mode
// lightNumber = {LIHGT_1, LIGHT_2} indicates which light is being setup time
// flagMode: set if change mode(execute func changeMode() - INT0), clear every new loops
// flagLightChange: set if 'lightNumber' change(execute func changeLightNumber() - INT1), clear every new loop
//
// -------------------------------------------------------------------------------------
static volatile int mode;
static volatile int lightNumber;
static volatile int flagMode;
static volatile int flagLightChange;



// -------------------------------------------------------------------------------------
// Pin on Arduino Board
// Use to config TrafficLight
// -------------------------------------------------------------------------------------
int DS_PIN_L1     =   5;
int STCP_PIN_L1   =   6;
int SHCP_PIN_L1   =   7;
int DS_PIN_L2     =   8;
int STCP_PIN_L2   =   9;
int SHCP_PIN_L2   =   10;
int BUTTON_UP     =   1;
int BUTTON_DOWN   =   11;

// -------------------------------------------------------------------------------------
// config parameters for TrafficLight
// -------------------------------------------------------------------------------------
int TIME_RED_L1     =   68;
int TIME_GREEN_L1   =   46;
int TIME_YELLOW_L1  =   3;
int INIT_STATE_L1   =   RED;

int TIME_RED_L2     =   28;
int TIME_GREEN_L2   =   20;
int TIME_YELLOW_L2  =   5;
int INIT_STATE_L2   =   GREEN;


// -------------------------------------------------------------------------------------
// Pins in 2-IC 74HC595 use to config TrafficLight (0-15)
// sP: pins use to control 2-digit 7-segment
// dP: pins use to control digits(2 digits)
// lP: pins use to control Light{RED, GREEN, BLUE}
// -------------------------------------------------------------------------------------
int sP[] = {0, 1, 2, 3, 4, 5, 6};
int dP[] = {8, 9};
int lP[] = {13, 14, 15};


// -------------------------------------------------------------------------------------
// global function
// -------------------------------------------------------------------------------------
void changeMode();          //  Interrupt functions
void changeLightNumber();   //  Interrupt functions

// start standard mode: RED -> GREEN -> YELLOW, counting from TIME_RED, TIME_GREEN to 0
void startStandardMode(TrafficLight& tf1, TrafficLight& tf2);

// start yellow light blink mode: yellow light will blink until change state
void startBlinkYellowMode(TrafficLight tf1, TrafficLight tf2);

// setup time for TrafficLight, 
// "state" parameter use for specify what time to setup {RED, GREEN}
void setupTime(TrafficLight& tf, int state);


// Declare two TrafficLight
TrafficLight t1, t2;


// ================================================================================================================
//      setup() function
// ================================================================================================================
void setup() {
  pinMode(DS_PIN_L1, OUTPUT);
  pinMode(STCP_PIN_L1, OUTPUT);
  pinMode(SHCP_PIN_L1, OUTPUT);
  pinMode(DS_PIN_L2, OUTPUT);
  pinMode(STCP_PIN_L2, OUTPUT);
  pinMode(SHCP_PIN_L2, OUTPUT);

  // Button as Input
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);

  t1.init(DS_PIN_L1, STCP_PIN_L1, SHCP_PIN_L1, sP, dP, lP, TIME_RED_L1, TIME_GREEN_L1, TIME_YELLOW_L1, INIT_STATE_L1);
  t2.init(DS_PIN_L2, STCP_PIN_L2, SHCP_PIN_L2, sP, dP, lP, TIME_RED_L2, TIME_GREEN_L2, TIME_YELLOW_L2, INIT_STATE_L2);

  attachInterrupt(digitalPinToInterrupt(2), changeMode, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), changeLightNumber, FALLING);
}
// ================================================================================================================
// ================================================================================================================



// ================================================================================================================
//      loop() function
// ================================================================================================================
void loop() {
  flagMode = 0; // reset flagMode
  flagLightChange = 0; // reset flagLightChange

  switch (mode) {
    case STANDARD_MODE:
      startStandardMode(t1, t2);
      break;
  
    case YELLOW_BLINK_MODE:
      startBlinkYellowMode(t1, t2);
      break;

    case SETUP_RED:
      if(lightNumber == LIGHT_1) {
        t2.turnOff();
        setupTime(t1, RED);
      } else {
        t1.turnOff();
        setupTime(t2, RED);
      }
      break;

    case SETUP_GREEN:
      if(lightNumber == LIGHT_1) {
        t2.turnOff();
        setupTime(t1, GREEN);
      } else {
        t1.turnOff();
        setupTime(t2, GREEN);
      }
      break; 
    default:
      break;
  }
}
// ================================================================================================================
// ================================================================================================================


// ======================================================================= //
//     GLOBAL FUNCTION DEFINITIONS
// ======================================================================= //

void changeMode() {
  delayMicroseconds(TIME_DEBOUNCE_US);
  mode++;
  if(mode > ( NUM_MODE - 1 ) ) {
    mode = 0;
  }
  flagMode = 1;
}


void changeLightNumber() {
  delayMicroseconds(TIME_DEBOUNCE_US);
  lightNumber++;
  if(lightNumber > ( NUM_LIGHT -1 ) ) {
    lightNumber = 0;
  }
  flagLightChange = 1;
}


void startStandardMode(TrafficLight& tf1, TrafficLight& tf2) {
  BitOrder bodr1 = tf1.generateBitOrder();
  BitOrder bodr2 = tf2.generateBitOrder();

  for(int i=0; i<TIMES_FLASH; i++) {

    tf1.show(bodr1, FIRST_DIGIT);
    tf2.show(bodr2, FIRST_DIGIT);
    delay(FLASH_MS);

    tf1.show(bodr1, SECOND_DIGIT);
    tf2.show(bodr2, SECOND_DIGIT);
    delay(FLASH_MS);

    if(flagMode) return;
  }
  
  tf1.timeDecreaseOne();
  tf2.timeDecreaseOne();

  if(tf1.isChangeState()) tf1.changeState();
  if(tf2.isChangeState()) tf2.changeState();
}


void startBlinkYellowMode(TrafficLight tf1, TrafficLight tf2) {
  tf1.controlYellow(ON);
  tf2.controlYellow(ON);
  for(int i=0; i<TIMES_FLASH; i++) {
    delay(FLASH_MS);
    if(flagMode) return;
  }
  
  tf1.controlYellow(OFF);
  tf2.controlYellow(OFF);
  for(int i=0; i<TIMES_FLASH; i++) {
    delay(FLASH_MS);
    if(flagMode) return;
  }
}


void setupTime(TrafficLight& tf, int state) {
  // save old state
  int oldTime = tf.getDisTime();
  int oldState = tf.getState();
  
  bool isChange = true;
  tf.setState(state);
  BitOrder bdr;

  while (1) {
    if(isChange) {
      if(state == RED) tf.setDisTime(tf.getTimeRed());
      else tf.setDisTime(tf.getTimeGreen());
      bdr = tf.generateBitOrder();
      isChange = false;
    }

    tf.show(bdr, FIRST_DIGIT);
    delay(FLASH_MS);
    tf.show(bdr, SECOND_DIGIT);
    delay(FLASH_MS);

    if(digitalRead(BUTTON_UP) == 0) {
      while (digitalRead(BUTTON_UP) == 0);
      if(state == RED) tf.timeRedInc();
      else tf.timeGreenInc();
      isChange = true;
    }

    if(digitalRead(BUTTON_DOWN) == 0) {
      while (digitalRead(BUTTON_DOWN) == 0);
      if(state == RED) tf.timeRedDec();
      else tf.timeGreenDec();
      isChange = true;
    }

    if(flagMode || flagLightChange) {
      tf.setState(oldState);
      tf.setDisTime(oldTime);
      return;
    }
  }   
}
// ======================================================================= //
// ======================================================================= //
// ======================================================================= //