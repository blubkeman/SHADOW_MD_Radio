/* =================================================================================
 *    SHADOW_MD_Radio: Radio Transmission of Marcduino Commands
 * =================================================================================
 *                          Last Revised Date: 27 June 2020
 *                          Revised By: Brian E. Lubkeman
 *  Inspired by the effort of DBoz
 * =================================================================================
 *
 * Please review and modify the settings found in the User_Settings.h tab as needed.
 * 
 * =================================================================================
 *
 * This program is free software: you can redistribute it and/or modify it for
 * your personal use and the personal use of other astromech club members.  
 *
 * This program is distributed in the hope that it will be useful 
 * as a courtesy to fellow astromech club members wanting to develop
 * their own droid control system.
 *
 * IT IS OFFERED WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You are using this software at your own risk and, as a fellow club member, it is
 * expected you will have the proper experience / background to handle and manage that 
 * risk appropriately.  It is completely up to you to insure the safe operation of
 * your droid and to validate and test all aspects of your droid control system.
 *
 * =======================================================================================
 *
 * This was developed and tested on the following equipment.
 *   Arduino Mega 2560                (https://www.sparkfun.com/products/11061)
 *   Adafruit Feather M0 RFM69HCW     (https://www.adafruit.com/product/3176)
 *   4-channel I2C-safe Bi-directional
 *     Logic Level Converter          (https://www.adafruit.com/product/757)
 *   Compact Marcduino v1.5           (https://astromech.net/forums/showthread.php?30724-Compact-Marcduino-v1-5-BC-Approved-Continuous-Various-(Jan-2017)-Open)
 *   Sainsmart USB host shield        (https://www.amazon.com/gp/product/B006J4G000)
 *   TRENDNet USB bluetooth dongle    (https://www.amazon.com/gp/product/B002AQSTXM)
 *   PS3 Move Navigation controller   (used, from www.gamestop.com)
 *
 * ======================================================================================= */

#include <SPI.h>
#include "Wire.h"
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

#include "Globals.h"

/* ====================
 * Function: setup()
 * ==================== */
void setup()
{

  // Start the serial communication.

  #if (defined(SERIAL) || defined(DEBUG))
  beginSerial();
  #endif

  // Set up the pins.

  beginPins();

  // Start the Feather radio.

  beginRadio();

}

/* ====================
 * Function: loop()
 * ==================== */
void loop()
{
  #if defined(SENDER)
  serialRX();
  #endif

  #ifdef RECEIVER
  radioRX();  // Further processing is called from within this function.
  #endif
}


// -------------------- Radio Functions --------------------


/* ==============================
 * Function: beginRadio()
 * ============================== */
void beginRadio()
{
  if (!rf69_manager.init()) {
    #ifdef DEBUG
    Serial.println("RFM69 radio init failed");
    #endif
    while (1);
  }

  #ifdef DEBUG
  Serial.println("RFM69 radio init OK!");
  #endif

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    #ifdef DEBUG
    Serial.println("setFrequency failed");
    #endif
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  rf69.setEncryptionKey(key);

  #ifdef DEBUG
  Serial.print("RFM69 radio @");
  Serial.print((int)RF69_FREQ);
  Serial.println(" MHz");
  Serial.print("RFM69 ");
    #if defined(SENDER)
    Serial.print("sender ");
    #elif defined(RECEIVER)
    Serial.print("receiver ");
    #endif
  Serial.println("has started.");
  #endif
}

/* ==========================================================
 * Function: radioTX()
 * Send content via radio. Wait to receive an acknowledgment.
 * When acknowledged, clear the buffer.
 * ========================================================== */
void radioTX(const char *radioTXPacket)
{
  #ifdef DEBUG
  Serial.print("Radio sending: ");
  Serial.println(radioTXPacket);
  #endif
  
  // Send content through the radio.

  if (rf69_manager.sendtoWait((uint8_t *)radioTXPacket, strlen(radioTXPacket), DEST_ADDRESS)) {

    // Content sent. Look for an acknowledgement.

    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {

      // Acknowledgement received. Reset buf[].

      #ifdef DEBUG
      Serial.println("  Acknowledgement received.");
      #endif

//      Blink(LED, 40, 3);            // Blink the LED 3 times, 40ms between blinks.

      buf[len] = 0;                 // Zero out remaining string.
      memset(buf, 0, sizeof(buf));  // Clear out buf[].
  #ifndef DEBUG
    }
  }
  #else
    } else {
      Serial.println("  Acknowledgement not received.");
    }
  } else {
    Serial.println("  Send failed.");
  }
  #endif
}

/* =====================================================
 * Function: radioRX()
 * This function receives content via radio and sends 
 * back an acknowledgement. All content received is then 
 * passed along to Serial1 which should be connected to 
 * a Marcduino master board.
 * ===================================================== */
void radioRX()
{
  const char *radioRXPacket = ""; // Prevents RF command repeat.

  if (rf69_manager.available()) {

    // Read from the radio. Received content is stored in buf[].

    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAck(buf, &len, &from)) {

      // Data received.

      buf[len] = 0;       // Zero out remaining string.
      radioRXPacket = ((char *)buf);

//      Blink(LED, 40, 3);  // Blink the LED 3 times, 40ms between blinks.

      // Send acknowledgement.

      rf69_manager.sendtoWait(ackMsg, sizeof(ackMsg), from);

      // Give a moment to complete the transmission.

      waitTime(50);

      // Now, transmit the content of buf[] over Serial1.

      #ifdef DEBUG
      Serial.print("Radio received: ");
      Serial.println(radioRXPacket);
      #endif

      serialTX(radioRXPacket);
    }
  }
}

/* =========================================================================
 * Function: radioTX_Marcduino()
 * This function evaluates each character read from Serial1 to assemble 
 * Marcduino commands. When a full command is assembled, the command is sent 
 * to the remote transceiver.
 * ========================================================================= */
void radioTX_Marcduino(const byte c)
{
  if (isCommandInProgress) {

    // Append this character to the Marcduino command.

    mdCommandAppend(c);

    // Look for command terminator.

    if (c == '\r') {
      // This character terminates a Marcduino command.
      // Send the command via radio then prepare for the next.
      radioTX((char *)mdCommand);
      mdCommandReset();
    }

/*
    switch (c)
    {
      case '\r':
        // This character terminates a Marcduino command.
        // Send the command via radio then prepare for the next.
        radioTX((char *)mdCommand);
        mdCommandReset();
        break;
      case 'r' :
        // When 'r' is escaped (\r), it terminates a Marcduino command.
        // Send the completed command, and reset for the next command.
        if (isEscaped == true) {
          radioTX((char *)mdCommand);
          mdCommandReset();
        }
        break;
      case '\\' :
        // The backslash is an escape character.
        // Toggle the escape character flag.
        isEscaped = (isEscaped ? false : true);
        #ifdef DEBUG
        if (isEscaped)
          Serial.println("Escape flag started");
        else
          Serial.println("Escape flag ended");
        #endif
        break;
      default:
        // The escape character is valid for only one character.
        // If this is that one character then reset the flag.
        if (isEscaped == true)
          isEscaped = false;
        break;
    }
*/
  } else {

    // Look for the start of a new Marcduino command.

    switch (c)
    {
      case ':' :
      case '*' :
      case '@' :
      case '$' :
      case '!' :
      case '%' :
      case '&' :
        // Start a new Marcduino command.
        isCommandInProgress = true;
        mdCommandAppend(c);
        break;
      default:
        // Ignore any other characters.
        break;
    }
  }
}


// -------------------- Serial Functions --------------------


/* =========================================================
 * Function: serialRX()
 * Read one byte(character) at a time from Serial1. Evaluate
 * each character to determine what to do with it.
 * ========================================================= */
void serialRX()
{
  if (Serial1.available()) {
    byte c;
    while (Serial1.available()) {
      c = Serial1.read();
      radioTX_Marcduino(c);
    }
  }
}

/* ========================================================
 * Function: serialTX()
 * Write the contents of buf[] to Serial1.
 * ======================================================== */
void serialTX(const char *serialTXPacket)
{
  #ifdef DEBUG
  Serial.print("Serial sending: ");
  Serial.println((char *)serialTXPacket);
  #endif
  Serial1.println((char *)serialTXPacket);
}


// -------------------- Support Functions --------------------



#if defined(SERIAL) || defined(DEBUG)
/* ==========================================================
 * Function: beginSerial()
 * This starts our Serial communication on the Feather radio.
 * In debug mode, this also starts the Serial monitor.
 * ========================================================== */
void beginSerial()
{
  #ifdef DEBUG
  Serial.begin(9600);
  while (!Serial) { waitTime(1); } // wait until serial console is open.
  Serial.println("Serial started.");
  #endif

  Serial1.begin(9600);
}
#endif

/* ===================================
 * Function: beginPins()
 * This function initializes our pins.
 * =================================== */
void beginPins()
{
  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  // Manual reset. Not sure why this is done, but it was in the example code.

  digitalWrite(RFM69_RST, HIGH);
  waitTime(10);
  digitalWrite(RFM69_RST, LOW);
  waitTime(10);
}

/* ================================================
 * Function: mdCommandAppend()
 * This function helps build the Marcduino command.
 * ================================================ */
void mdCommandAppend(const byte c) {
  mdCommand[mdIdx] = (char)c;
  mdIdx++;
}

/* ===========================================
 * Function: mdCommandReset()
 * This function clears the Marcduino command.
 * =========================================== */
void mdCommandReset() {
  memset(mdCommand, 0, sizeof(mdCommand));
  mdIdx = 0;
  isCommandInProgress = false;
}

/* =====================================================
 * Function: Blink()
 * This function causes the Feather radio's LED to blink
 * giving a visual indication of the radio transmission.
 * ===================================================== */
void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    waitTime(DELAY_MS);
    digitalWrite(PIN,LOW);
    waitTime(DELAY_MS);
  }
  
}

/* ================================
 * Function: waitTime()
 * This is an alternate to delay().
 * ================================ */
void waitTime(unsigned long waitTime)
{
  unsigned long endTime = millis() + waitTime;
  while (millis() < endTime) {} // do nothing
}
