#include <Arduino.h>
#include <TrafficLight.h>
#include <Wire.h>
#include <RTClib.h>

// -------------------------------------------------------------
//                        Global constant
// -------------------------------------------------------------
#define    NUM_MODE             6
#define    STANDARD_MODE        0
#define    YELLOW_BLINK_MODE    1
#define    AUTO_MODE            2
#define    SET_TIME_AUTO        3 
#define    SETUP_RED            4 
#define    SETUP_GREEN          5
#define    NUM_LIGHT            2
#define    LIGHT_1              0
#define    LIGHT_2              1
#define    TIMES_FLASH          80
#define    FLASH_MS             5
#define    TIME_DEBOUNCE_US     20
#define    START                0
#define    END                  1


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
static volatile int startEnd; // choose which will be setup <START, END> in SET_TIME_AUTO mode

int START_HOUR = 6;
int END_HOUR   = 22;
typedef TrafficLight TimeBox;



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
int DS_PIN_TB     =   11;
int STCP_PIN_TB   =   12;
int SHCP_PIN_TB   =   13;
int BUTTON_UP     =   1;
int BUTTON_DOWN   =   0;

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

// Auto Mode:
//     ex. start = 6h
//         end   = 22h
//     => 6h-22h: run standard mode < normal mode >
//        22h-6h: run blink yellow mode
// function config start and end times for Auto Mode
// param TimeBox tb saved timeRed as time start, timeGreen as time end
void startSetTimeAutoMode(TimeBox& tb);


// Declare two TrafficLight
TrafficLight t1, t2;
RTC_DS1307 rtc;
TimeBox timeBox;


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
  pinMode(DS_PIN_TB, OUTPUT);
  pinMode(STCP_PIN_TB, OUTPUT);
  pinMode(SHCP_PIN_TB, OUTPUT);
  

  // Button as Input
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);

  t1.init(DS_PIN_L1, STCP_PIN_L1, SHCP_PIN_L1, sP, dP, lP, TIME_RED_L1, TIME_GREEN_L1, TIME_YELLOW_L1, INIT_STATE_L1);
  t2.init(DS_PIN_L2, STCP_PIN_L2, SHCP_PIN_L2, sP, dP, lP, TIME_RED_L2, TIME_GREEN_L2, TIME_YELLOW_L2, INIT_STATE_L2);
  timeBox.init(DS_PIN_TB, STCP_PIN_TB, SHCP_PIN_TB, sP, dP, START_HOUR, END_HOUR);

  attachInterrupt(digitalPinToInterrupt(2), changeMode, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), changeLightNumber, FALLING);

  rtc.begin();
  rtc.adjust(DateTime(__DATE__, __TIME__));
}
// ================================================================================================================
// ================================================================================================================



// ================================================================================================================
//      loop() function
// ================================================================================================================
void loop() {
  flagMode = 0; // reset flagMode
  flagLightChange = 0; // reset flagLightChange
  DateTime now = rtc.now();

  switch (mode) {
    case STANDARD_MODE:
      timeBox.turnOff();
      startStandardMode(t1, t2);
      break;
  
    case YELLOW_BLINK_MODE:
      timeBox.turnOff();
      startBlinkYellowMode(t1, t2);
      break;
    case AUTO_MODE:
      if(now.hour() < timeBox.getTimeRed() || now.hour() > timeBox.getTimeGreen()) startBlinkYellowMode(t1, t2);
      else startStandardMode(t1, t2);
      break;
    case SET_TIME_AUTO:
      t1.turnOff();
      t2.turnOff();
      startSetTimeAutoMode(timeBox);
      break;
    case SETUP_RED:
      timeBox.turnOff();
      if(lightNumber == LIGHT_1) {
        t2.turnOff();
        setupTime(t1, RED);
      } else {
        t1.turnOff();
        setupTime(t2, RED);
      }
      break;

    case SETUP_GREEN:
      timeBox.turnOff();
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
  if(mode == SET_TIME_AUTO) {
    startEnd++;
    if(startEnd > 1) startEnd = 0;
  }
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

void startSetTimeAutoMode(TimeBox& tb) {
  if(startEnd == START) {
    tb.setDisTime(tb.getTimeRed());
  } else {
    tb.setDisTime(tb.getTimeGreen());
  }

  BitOrder bdr = tb.generateBitOrder();
  tb.show(bdr, FIRST_DIGIT);
  delay(FLASH_MS);
  tb.show(bdr, SECOND_DIGIT);
  delay(FLASH_MS);

  if(digitalRead(BUTTON_UP) == 0) {
    while (digitalRead(BUTTON_UP) == 0);
    if(startEnd == START) tb.timeRedInc();
    else tb.timeGreenInc();
    if(tb.getTimeRed() > 23) tb.setTimeRed(0);
    if(tb.getTimeGreen() > 23) tb.setTimeGreen(0);
  }

  if(digitalRead(BUTTON_DOWN) == 0) {
    while (digitalRead(BUTTON_DOWN) == 0);
    if(startEnd == START) tb.timeRedDec();
    else tb.timeGreenDec();
    if(tb.getTimeRed() < 0) tb.setTimeRed(23);
    if(tb.getTimeGreen() < 0) tb.setTimeGreen(23);
  }
}


// ======================================================================= //
// ======================================================================= //
// ======================================================================= //