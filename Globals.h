#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "User_Settings.h"

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

#ifdef SENDER
#define MY_ADDRESS     2
#define DEST_ADDRESS   1
#endif

#ifdef RECEIVER
#define MY_ADDRESS     1
#define DEST_ADDRESS   2
#endif

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined(ADAFRUIT_FEATHER_M0) // Feather M0 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM69_INT     3  // 
  #define RFM69_CS      4  //
  #define RFM69_RST     2  // "A"
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM69_CS      2    // "E"
  #define RFM69_IRQ     15   // "B"
  #define RFM69_RST     16   // "D"
  #define LED           0
#endif

#if defined(ESP32)    // ESP32 feather w/wing
  #define RFM69_RST     13   // same as LED
  #define RFM69_CS      33   // "B"
  #define RFM69_INT     27   // "A"
  #define LED           13
#endif

// RadioHead driver.
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// RadioHead manager.
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

// Radio message data.
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t ackMsg[] = "ACK";

// Marcduino command parsing.
uint8_t mdCommand[RH_RF69_MAX_MESSAGE_LEN];
bool isCommandInProgress = false;
uint8_t mdIdx = 0;

#endif
