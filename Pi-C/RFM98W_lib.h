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

enum radio_configs {
    FLAGS_ACK = 0x80,
    BROADCAST_ADDRESS = 255,

    REG_00_FIFO = 0x00,
    REG_01_OP_MODE = 0x01,
    REG_06_FRF_MSB = 0x06,
    REG_07_FRF_MID = 0x07,
    REG_08_FRF_LSB = 0x08,
    REG_0E_FIFO_TX_BASE_ADDR = 0x0E,
    REG_0F_FIFO_RX_BASE_ADDR = 0x0F,
    REG_10_FIFO_RX_CURRENT_ADDR = 0x10,
    REG_12_IRQ_FLAGS = 0x12,
    REG_13_RX_NB_BYTES = 0x13,
    REG_1D_MODEM_CONFIG1 = 0x1D,
    REG_1E_MODEM_CONFIG2 = 0x1E,
    REG_19_PKT_SNR_VALUE = 0x19,
    REG_1A_PKT_RSSI_VALUE = 0x1A,
    REG_20_PREAMBLE_MSB = 0x20,
    REG_21_PREAMBLE_LSB = 0x21,
    REG_22_PAYLOAD_LENGTH = 0x22,
    REG_26_MODEM_CONFIG3 = 0x26,

    REG_4D_PA_DAC = 0x4D,
    REG_40_DIO_MAPPING1 = 0x40,
    REG_0D_FIFO_ADDR_PTR = 0x0D,

    PA_DAC_ENABLE = 0x07,
    PA_DAC_DISABLE = 0x04,
    PA_SELECT = 0x00,
    OUTPUT_PWR = 0x0F,
    MAX_PWR = 0x7,

    CAD_DETECTED_MASK = 0x01,
    RX_DONE = 0x40,
    TX_DONE = 0x08,
    CAD_DONE = 0x04,
    CAD_DETECTED = 0x01,

    LONG_RANGE_MODE = 0x80,
    MODE_SLEEP = 0x00,
    MODE_STDBY = 0x01,
    MODE_FS_TX = 0x02,
    MODE_TX = 0x03,
    MODE_FS_RX = 0x04,
    MODE_RXCONTINUOUS = 0x05,
    MODE_CAD = 0x07,

    REG_09_PA_CONFIG = 0x09,
    FXOSC = 32000000,
    FSTEP = (FXOSC / 524288),

    HIGH_FREQUENCY = 0
};

enum modem_config {
    Bw125Cr45_LSB = 0x72,
    Bw125Cr45_MID = 0x74,
    Bw125Cr45_MSB = 0x04,

    Bw500_LSB = 0x92,
    Bw500_MID = 0x74,
    Bw500_MSB = 0x04,

    Bw31_LSB = 0x48,
    Bw31_MID = 0x94,
    Bw31_MSB = 0x04,

    Bw125Cr48_LSB = 0x78,
    Bw125Cr48_MID = 0xC4,
    Bw125Cr48_MSB = 0x0C
};

// ========== Functions ========== // 
// Configure radio module
void configure();
// Set the radio frequency to a given value in Hz
void setFrequency(uint32_t frequency);
// Begin transmit
void beginTX(Packet tx, bool* rfm_done, uint8_t* rfm_status);
// End transmit
void endTX(bool* rfm_done, uint8_t* rfm_status);
// Begin receive
void set_mode_RX();
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