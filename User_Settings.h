#ifndef _USER_SETTINGS_H_
#define _USER_SETTINGS_H_

/* ===============================================================
 *  Role:
 * Uncomment only one of the following lines for each transceiver
 * then upload the sketch to the board.
 * 
 * Each transceiver plays a role. A transceiver connected to the 
 * Arduino is a "sender". The other transceiver is a "receiver".
 * =============================================================== */
#define SENDER
//#define RECEIVER

/* =============================================================
 *  Communication Protocol:
 * Uncomment one line to indicate which you will be using.
 * 
 * This tells the transceivers to communicate via Serial or I2C.
 * Currently, this code supports use of only one communication
 * protocol at a time.  
 * ============================================================= */
#define SERIAL
//#define I2C   // Future: This is not yet supported.

/* =================================================================
 *  Encryption 
 * Modify each hexidecimal number here to a value of your choosing.
 * Valid values range from 0x00 to 0xFF.
 * 
 * This is important. The 915MHz radio frequency is shared by other
 * people. The unique encryption key makes sure you process only
 * your transmissions.
 * ================================================================= */
// The encryption key has to be the same as the one in the server
uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

/* =========================================
 *  Mode:
 * Uncomment this line to enable debug mode.
 * ========================================= */
#define DEBUG

#endif
