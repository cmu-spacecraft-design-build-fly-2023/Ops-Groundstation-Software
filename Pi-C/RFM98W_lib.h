/* (C) Team Impulse 2015
 * Made available under a modification of the MIT license - see repository root.
 * Comms library: RFM98W
 * 
 * Modified and converted to C by D.J. Morvay (dmorvay@andrew.cmu.edu)
 * Carnegie Mellon University - Fall/Spring 2023
 * 18-873 - Spacecraft Build & Fly Lab
 */

#ifndef RFMLib_h
#define RFMLib_h

#include <bcm2835.h>
#include <stdint.h>
#include <stdbool.h>

//Set to true to enable high frequency mode
static const uint8_t high_frequency = 0;
static const uint32_t Fosc = 32000000;
static const uint32_t Fstep = Fosc / 524288;

// ========== Structs ========== // 
typedef struct lora_Packet { //data structure for storing a packet
    uint8_t len;
    uint8_t data[256];
    int snr;
    int rssi;
    bool crc;//true for success, false for failure. Can read values if true.
} Packet;

// ========== ENUMS ========== //
enum GPIO_ASSIGN {
    nss = 8,
    dio0 = 19,
    dio5 = 16,
    rfm_rst = 12
};

// ========== Functions ========== // 
// Configure radio module
void configure(uint8_t config[6]);
// Set the radio frequency to a given value in Hz
void setFrequency(uint32_t frequency);
// Begin transmit
void beginTX(Packet tx, bool* rfm_done, uint8_t* rfm_status);
// End transmit
void endTX(bool* rfm_done, uint8_t* rfm_status);
// Begin receive
void beginRX(bool* rfm_done, uint8_t* rfm_status);
// End receive
void endRX(Packet* received, bool* rfm_done, uint8_t* rfm_status);
// Change radio
void radioMode(uint8_t m);//set the mode of the radio

// ----- Low-level I/O functions ----- // 
void wRFM(uint8_t ad, uint8_t val);//IO functions
void bwRFM(uint8_t ad, uint8_t vals[], int n);
void brRFM(uint8_t ad, uint8_t vals[], uint8_t len);
//Return silicon version
uint8_t getVersion();
// Single byte read
uint8_t rRFM(uint8_t ad);
// Arduino Bit Read
uint8_t bitRead(uint8_t x, uint8_t n);

#endif