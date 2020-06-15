#include  "TrafficLight.h"

// =================================================================================== //
//                                TrafficLight.cpp
// Definite class TrafficLight
// =================================================================================== //
// =================================================================================== //


void TrafficLight::init(int dataPin, int latchPin, int clkPin, int segP[], int digitsP[], int ledP[], 
                          int tR, int tG, int tY, int initState) {
  SPI_MOSI = dataPin;
  SPI_CS = latchPin;
  SPI_CLK = clkPin;
  
  timeRed = tR;
  timeGreen = tG;
  timeYellow = tY;

  state = initState;
  if(state == RED) {
    disTime = timeRed;
  } else if(state == GREEN) {
    disTime = timeGreen;
  } else {
    disTime = timeYellow;
  }

  for(int i=0; i<NUM_SEG; i++) {
    segPin[i] = segP[i];
  }
  
  for(int i=0; i<NUM_DIGIT; i++) {
    digitsPin[i] = digitsP[i];
  }
  
  for(int i=0; i<NUM_LED; i++) {
    ledPin[i] = ledP[i];
  }  
}


void TrafficLight::init(int data, int latch, int clk, int segP[], int digitsP[], int hour_1, int hour_2) {
  SPI_MOSI = data;
  SPI_CS = latch;
  SPI_CLK = clk;

  timeRed = hour_1;
  timeGreen = hour_2;
  disTime = timeRed;

  for(int i=0; i<NUM_SEG; i++) {
    segPin[i] = segP[i];
  }
  
  for(int i=0; i<NUM_DIGIT; i++) {
    digitsPin[i] = digitsP[i];
  }
}


void TrafficLight::writeBitOrder(int* order, byte& firstPart, byte& secondPart) {
  for(int i=0; i<8; i++) {
    firstPart |= (order[15 - i] << i);
    secondPart |= (order[7-i] << i);
  }
}


void TrafficLight::setState(int s) {
  state = s;
}


void TrafficLight::setDisTime(int t) {
  disTime = t;
}


void TrafficLight::setTimeRed(int tR) {
  timeRed = tR;
}


void TrafficLight::setTimeGreen(int tG) {
  timeGreen = tG;
}


void TrafficLight::setTimeYellow(int tY) {
  timeYellow = tY;
}


int TrafficLight::getTimeRed() {
  return timeRed;
}


int TrafficLight::getTimeGreen() {
  return timeGreen;
}


int TrafficLight::getTimeYellow() {
  return timeYellow;
}


int TrafficLight::getDisTime() {
  return disTime;
}


int TrafficLight::getState() {
  return state;
}


bool TrafficLight::isChangeState() {
  return disTime < 0;
}


void TrafficLight::timeDecreaseOne() {
  disTime--;
}


void TrafficLight::timeRedInc() {
  timeRed++;
}


void TrafficLight::timeRedDec() {
  timeRed--;
}


void TrafficLight::timeGreenInc() {
  timeGreen++;
}


void TrafficLight::timeGreenDec() {
  timeGreen--;
}


BitOrder TrafficLight::generateBitOrder() {
  int order[16]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  // if disTime > 100 (3 digits) only show 2 least significant digits
  int tmpTime = (disTime > 99) ? (disTime - 100) : disTime;
  int firstDigit = tmpTime / 10;
  int secondDigit = tmpTime % 10;
  BitOrder result = {0, 0, 0, 0};

  // ######################################################################################################
  if(state == RED) {
    order[ledPin[RED]] = 0; // turn on RED
    order[ledPin[GREEN]] = 1; // set to turn off GREEN
    order[ledPin[YELLOW]] = 1; // set to turn off YELLOW
  }
  else if(state == GREEN) {
    order[ledPin[GREEN]] = 0; // turn on GREEN
    order[ledPin[RED]] = 1; // set to turn off RED
    order[ledPin[YELLOW]] = 1; // set to turn off YELLOW
  }
  else {
    order[ledPin[YELLOW]] = 0; // turn on YELLOW
    order[ledPin[GREEN]] = 1; // set to turn off GREEN
    order[ledPin[RED]] = 1; // set to turn off RED
  }

  // ######################################################################################################
  if(state == YELLOW) { // if state is YELLOW => don't show digits
    order[digitsPin[FIRST_DIGIT]] = 0;
    order[digitsPin[SECOND_DIGIT]] = 0;

    writeBitOrder(order, result.first1, result.first2);
    writeBitOrder(order, result.second1, result.second2);
    
    return result;
  }


  // ######################################################################################################
  order[digitsPin[FIRST_DIGIT]] = 1;
  order[digitsPin[SECOND_DIGIT]] = 0;
  byte tmp = BIT_MAP[firstDigit];
  for(int i=0; i<NUM_SEG; i++) {
    order[segPin[NUM_SEG - i - 1]] = 0; // clear
    order[segPin[NUM_SEG - i - 1]] = !!(tmp & (1 << i));
  }

  writeBitOrder(order, result.first1, result.first2);

  order[digitsPin[FIRST_DIGIT]] = 0;
  order[digitsPin[SECOND_DIGIT]] = 1;
  tmp = BIT_MAP[secondDigit];
  for(int i=0; i<NUM_SEG; i++) {
    order[segPin[NUM_SEG - i - 1]] = 0; // clear
    order[segPin[NUM_SEG - i - 1]] = !!(tmp & (1 << i));
  }

  writeBitOrder(order, result.second1, result.second2);
  
  // ######################################################################################################

  // return value
  return result;
}


void TrafficLight::show(BitOrder bitOrder, int idx) {
  if(idx == FIRST_DIGIT) {
    digitalWrite(SPI_CS, LOW);
    shiftOut(SPI_MOSI, SPI_CLK, LSBFIRST, bitOrder.first1);
    shiftOut(SPI_MOSI, SPI_CLK, LSBFIRST, bitOrder.first2);
    digitalWrite(SPI_CS, HIGH);
  } else {
    digitalWrite(SPI_CS, LOW);
    shiftOut(SPI_MOSI, SPI_CLK, LSBFIRST, bitOrder.second1);
    shiftOut(SPI_MOSI, SPI_CLK, LSBFIRST, bitOrder.second2);
    digitalWrite(SPI_CS, HIGH);
  }
}


void TrafficLight::controlYellow(int onOff) {
  BitOrder odr = {0, 0, 0, 0};
  int b[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  if(onOff == ON) {
    b[ledPin[YELLOW]] = 0; // turn on YELLOW
    b[ledPin[RED]] = 1; // turn off RED
    b[ledPin[GREEN]] = 1; // turn off GREEN
  } else {
    b[ledPin[RED]] = 1; // turn off RED
    b[ledPin[GREEN]] = 1; // turn off GREEN
    b[ledPin[YELLOW]] = 1; // turn off YELLOW
  }


  writeBitOrder(b, odr.first1, odr.first2);
  digitalWrite(SPI_CS, LOW);
  shiftOut(SPI_MOSI, SPI_CLK, LSBFIRST, odr.first1);
  shiftOut(SPI_MOSI, SPI_CLK, LSBFIRST, odr.first2);
  digitalWrite(SPI_CS, HIGH);
}


void TrafficLight::turnOff() {
  controlYellow(OFF);
}


void TrafficLight::changeState() {
  if(state == RED) {
    state = GREEN;
    disTime = timeGreen;
  } else if(state == GREEN) {
    state = YELLOW;
    disTime = timeYellow;
  } else {
    state = RED;
    disTime = timeRed;
  }
}


// =================================================================================== //
// =================================================================================== //
// =================================================================================== //