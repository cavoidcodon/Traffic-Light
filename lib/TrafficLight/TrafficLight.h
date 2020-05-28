#ifndef _TRAFFIC_LIGHT_
#define _TRAFFIC_LIGHT_

#include <Arduino.h>

#define   NUM_SEG           7
#define   NUM_DIGIT         2
#define   NUM_LED           3
#define   RED               0
#define   GREEN             1
#define   YELLOW            2
#define   FIRST_DIGIT       0
#define   SECOND_DIGIT      1
#define   ON                0
#define   OFF               1

// Struct BitOrder: Contains bit order to shift out to Max7219
// 16 bit -> 2 part {firstPart, secondPart}
// ======================================== //
struct BitOrder{
  byte first1;
  byte first2;
  byte second1;
  byte second2;
};
// ======================================== //


// Bit map for digit 0 -> 9
// 7-segment display
// ======================================== //
const static byte BIT_MAP[10] = {
  B00000001, // 0
  B01001111, // 2 ...
  B00010010,
  B00000110,
  B01001100,
  B00100100,
  B00100000,
  B00001111,
  B00000000,  // 8
  B00000100  // 9
};
// ======================================== //


// class TrafficLight declare
// ======================================== //
class TrafficLight {
    private:
      int SPI_MOSI;
      int SPI_CS;
      int SPI_CLK;
      int segPin[NUM_SEG];
      int digitsPin[NUM_DIGIT];
      int ledPin[NUM_LED];
      int disTime;
      int timeRed;
      int timeGreen;
      int timeYellow;
      int state;
      void writeBitOrder(int* order, byte& firstPart, byte& secondPart);
      
    public:
      TrafficLight() {};
      ~TrafficLight() {};
      
      // Initialize hardware config: SPI_MOSI, SPI_CS, SPI_CLK
      // Initialize which pin in Max7219 connect to the 7-segment, traffic light
      // -------------------------------------------------------------------------
      void init(int dataPin, int latchPin, int clkPin, int segP[], int digitsP[], int ledP[], int tR, int tG, int tY, int initState);

      // Getter, setter
      // -------------------------------------
      void setState(int s);
      void setDisTime(int t);
      void setTimeRed(int tR);
      void setTimeGreen(int tG);
      void setTimeYellow(int tY);
      int getTimeRed();
      int getTimeGreen();
      int getTimeYellow();
      int getState();
      int getDisTime();


      // Changes values of disTime, timeRed, timeGreen
      // ---------------------------------------------
      void timeDecreaseOne();
      void timeRedInc();
      void timeRedDec();
      void timeGreenInc();
      void timeGreenDec();


      // Turn off or turn on YELLOW light, turn of RED, GREEN light
      // -----------------------------------------------------------
      void controlYellow(int onOff);


      // Generate bit order corresponding to the status to be displayed
      // return a BitOrder struct variable
      // -------------------------------------
      BitOrder generateBitOrder();


      // shiftOut the bit order to show the current state {RED, GREEN, YELLOW}, time
      // param: bitOrder(BitOrder) - the bit order corresponding to the status display
      //        idx(int) - the index of digit, only show a digit/times -> flash 7-segment
      // --------------------------------------------------------------------------------
      void show(BitOrder bitOrder, int idx);

      // turn off the light
      // --------------------------------------------------------------------------------
      void turnOff();

      // check disTime < 0 ? (Change state)
      // --------------------------------------------------------------------------------
      bool isChangeState();

      // change the current state and display time
      // RED --> GREEN
      // GREEN --> YELLOW       
      // YELLOW --> RED
      // ---------------------------------------------------------------------------------
      void changeState();
};
// ======================================== //


// class TrafficLight definition
// ======================================= //

#endif // _TRAFFIC_LIGHT_