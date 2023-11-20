/**
 * @name: RFM98W_lib
 * 
 * @authors: David J. Morvay (dmorvay@andrew.cmu.edu)
 * Carnegie Mellon University
 * Fall 2023 - Spring 2024
 * ECE 18-873 - Spacecraft Build, Design, & Fly Lab
 * Satellite <> Groundstation Communications
 * 
 * With inspiration from: https://github.com/weustace/LoRaLib/tree/master
 * As well as: https://gitlab.com/the-plant/raspi-lora
 * 
 * @brief: This file contains all the helper functions
 *          for the RFM98 LoRa module. 
*/

#include "RFM98W_lib.h"
#include <bcm2835.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @name: configure
 * 
 * @brief: The following function configures the RFM radio module
 *          registers for transmitting and receiving data. 
 * 
 * Inputs: NONE
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void configure() {
    // Initialize SPI communication
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST); // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);              // The default
    bcm2835_spi_set_speed_hz(5000000);                       // The default

    // Control CS line manually and don't assert CEx line!
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
    
    // Set CE0 line
    bcm2835_gpio_fsel(nss, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(nss, 1);

    if(rfm_rst!=255){
        bcm2835_gpio_fsel(rfm_rst, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(rfm_rst, 1);
    }

    // Set interrupt pin to be an input
    bcm2835_gpio_fsel(dio0, BCM2835_GPIO_FSEL_INPT);
    // with a puldown
    bcm2835_gpio_set_pud(dio0, BCM2835_GPIO_PUD_DOWN);
    // And a rising edge detect enable
    bcm2835_gpio_ren(dio0);

    if(dio5!=255) {
        // Set RPI GPIO16 or DIO5
        bcm2835_gpio_fsel(dio5, BCM2835_GPIO_FSEL_INPT);
        //  with a pulldown
        bcm2835_gpio_set_pud(dio5, BCM2835_GPIO_PUD_DOWN);
    }

    // Set radio mode to sleep
    radioMode(MODE_SLEEP);

    // Set Tx and Rx base address
    wRFM(REG_0E_FIFO_TX_BASE_ADDR,0);
    wRFM(REG_0F_FIFO_RX_BASE_ADDR,0);
     if ((rRFM(REG_0E_FIFO_TX_BASE_ADDR) == 0) && (rRFM(REG_0F_FIFO_RX_BASE_ADDR) == 0)) {
        printf("LoRa base addresses set!\n");
    }

    // Set radio mode to standby
    radioMode(MODE_STDBY);

    // Set modem configuration
    wRFM(REG_1D_MODEM_CONFIG1, Bw125Cr45_LSB); // Modem congif1
    wRFM(REG_1E_MODEM_CONFIG2, Bw125Cr45_MID); // Modem config2
    wRFM(REG_26_MODEM_CONFIG3, Bw125Cr45_MSB); // Modem config3
    if ((rRFM(REG_1D_MODEM_CONFIG1)==Bw125Cr45_LSB) && (rRFM(REG_1E_MODEM_CONFIG2)==Bw125Cr45_MID) && (rRFM(REG_26_MODEM_CONFIG3)==Bw125Cr45_MSB)) {
        printf("Modem configured properly!\n");
    }

    // Set preamble
    wRFM(REG_20_PREAMBLE_MSB,0);
    wRFM(REG_21_PREAMBLE_LSB,8);
    if ((rRFM(REG_20_PREAMBLE_MSB)==0) && (rRFM(REG_21_PREAMBLE_LSB)==8)) {
        printf("Preamble set properly!\n");
    }

    // Use setFrequency for frequency adjustment
    setFrequency(915);

    // Set tx power
    wRFM(REG_4D_PA_DAC, PA_DAC_DISABLE);
    if (rRFM(REG_4D_PA_DAC) == PA_DAC_DISABLE) {
        printf("PA DAC DISABLED!\n");
    }

    // Set PA configuration
    uint8_t PA_CONFIG = ((uint8_t)(PA_SELECT << 7)) + ((uint8_t)(MAX_PWR_BENCH << 4)) + ((uint8_t)OUTPUT_PWR_BENCH);
    wRFM(REG_09_PA_CONFIG, PA_CONFIG);
    if (rRFM(REG_09_PA_CONFIG) == PA_CONFIG) {
        printf("PA configuration set!\n");
    }
}

/**
 * @name: setFrequency
 * 
 * @brief: The following function sets the frequency
 *         of the RFM module to the requested input frequency. 
 * 
 * Inputs
 *  @param: frequency - Requested frequency to transmit or receive
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void setFrequency(uint32_t frequency) {
    // Convert input to register form
    uint32_t freqVal = 14991360;
    // (frequency * 1000000) / (FXOSC / 524288);
    printf("%u\n",freqVal);

    // Send the frequency to the RFM
    wRFM(REG_06_FRF_MSB, (freqVal >> 16) & 0xFF); // MSB
    wRFM(REG_07_FRF_MID, (freqVal >> 8) & 0xFF); // MID
    wRFM(REG_08_FRF_LSB, (freqVal) & 0xFF); // LSB
    uint32_t getfreq = (((uint32_t)rRFM(REG_06_FRF_MSB)) << 16) + (((uint32_t)rRFM(REG_07_FRF_MID)) << 8) + ((uint32_t)rRFM(REG_08_FRF_LSB));
    printf("Frequency set to: %u Mhz\n",(getfreq*(FXOSC / 524288))/1000000);
}

/**
 * @name: getVersion
 * 
 * @brief: The following function gets the hardware version 
 *         of the RFM module.
 * 
 * Inputs: NONE
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: Version - Hardware version of the RFM
*/
uint8_t getVersion() {
    return rRFM(0x42);
}

/**
 * @name: set_mode_RX
 * 
 * @brief: The following function sets the
 *          RFM module to receiving mode. 
 * 
 * Inputs: NONE
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void set_mode_RX() {
    if ((rRFM(REG_01_OP_MODE) & MODE_RXCONTINUOUS) != MODE_RXCONTINUOUS) {
        radioMode(MODE_RXCONTINUOUS);
        wRFM(REG_40_DIO_MAPPING1, 0x0);
    }
}

/**
 * @name: RX_transmission
 * 
 * @brief: The following function unpacks
 *          a received message from a sender. 
 *          Message is stored in RFM buffer until
 *          requested by the Raspberry Pi. 
 * 
 * Inputs
 *  @param: received - Pointer to Pi onboard message buffer.
 * Outputs
 *  @param: received - Message buffer loaded with received data.
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void RX_transmission(Packet* received) {//function to be called on, or soon after, reception of RX_DONE interrupt
    // Get current frequency and IRQ flag status
    uint32_t getfreq = (((uint32_t)rRFM(REG_06_FRF_MSB)) << 16) + (((uint32_t)rRFM(REG_07_FRF_MID)) << 8) + ((uint32_t)rRFM(REG_08_FRF_LSB));
    uint8_t irq_flags = rRFM(REG_12_IRQ_FLAGS);

    // If messages ready... get it!
    if (((rRFM(REG_01_OP_MODE) & MODE_RXCONTINUOUS) == MODE_RXCONTINUOUS) && (irq_flags & RX_DONE)) {
        // Get message length and set address pointer
        (*received).len = rRFM(REG_13_RX_NB_BYTES);
        wRFM(REG_0D_FIFO_ADDR_PTR,rRFM(REG_10_FIFO_RX_CURRENT_ADDR));

        // Get message from RFM
        brRFM(REG_00_FIFO, (*received).data, (*received).len);
        // Clear IRQ flags
        wRFM(REG_12_IRQ_FLAGS, 0xFF);

        // Receive and calculate SNR and RSSI
        (*received).snr = rRFM(REG_19_PKT_SNR_VALUE) / 4;
        (*received).rssi = rRFM(REG_1A_PKT_RSSI_VALUE);

        if ((*received).snr < 0) {
            (*received).rssi = (*received).snr + (*received).rssi;
        }
        else {
            (*received).rssi = (*received).rssi * 16 / 15;
        }

        if (getfreq >= 779) {
            (*received).rssi = (*received).rssi - 157;
        }
        else {
            (*received).rssi = (*received).rssi - 164;
        }
    }
    // Reset RFM to receiving mode
    set_mode_RX();
    // Clear IRQ again
    wRFM(REG_12_IRQ_FLAGS,0xFF);
}

/**
 * @name: set_mode_TX
 * 
 * @brief: The following function sets the
 *          RFM module to transmitting mode
 * 
 * Inputs: NONE
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void set_mode_TX() {
    if ((rRFM(REG_01_OP_MODE) & MODE_TX) != MODE_TX) {
        wRFM(REG_40_DIO_MAPPING1, 0x40);
        radioMode(MODE_TX);
    }
}

/**
 * @name: TX_transmission
 * 
 * @brief: The following function packs a message 
 *          and transfers it to the RFM. 
 * 
 * @warning: Needs a rewrite and has not been tested yet!
 * 
 * Inputs
 *  @param: transmit_pkt - Pointer to message that will be sent.
 * 
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void TX_transmission(Packet transmit_pkt) {
    // Set radio to standby and clear IRQ flags
    radioMode(MODE_STDBY);
    wRFM(REG_12_IRQ_FLAGS,0xFF);

    // Set payload length
    wRFM(REG_22_PAYLOAD_LENGTH,transmit_pkt.len);
    // Get base address
    uint8_t base_addr = rRFM(REG_0E_FIFO_TX_BASE_ADDR);
    //Put transmit base FIFO addr in FIFO pointer
    wRFM(REG_0D_FIFO_ADDR_PTR,base_addr);

    uint8_t new_data[transmit_pkt.len];
    for(int i = 0;i<transmit_pkt.len;i++){
        new_data[i] = transmit_pkt.data[i];
    }

    // Send message to RFM to be transmitted!
    bwRFM(REG_00_FIFO,new_data,transmit_pkt.len);
    // Reset FIFO pointer
    wRFM(REG_0D_FIFO_ADDR_PTR,base_addr);
    // Repeat; might not be needed?
    wRFM(REG_0D_FIFO_ADDR_PTR,base_addr);
    // Set radio mode to tx
    set_mode_TX();
}

/**
 * @name: radioMode
 * 
 * @brief: The following function changes the mode of the RFM module.
 *         See RFM98W_lib header file for ENUM definitions.
 * 
 * Inputs
 *  @param: m - Requested radio mode.
 * 
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void radioMode(uint8_t m){
    switch(m){
        case MODE_SLEEP: // Sleep Mode
            wRFM(REG_01_OP_MODE, LONG_RANGE_MODE | MODE_SLEEP);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_SLEEP)) {
                printf("LoRa radio mode set to sleep!\n");
            }
            break;
        case MODE_STDBY: // Standby Mode
            wRFM(REG_01_OP_MODE, MODE_STDBY);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_STDBY)) {
                 printf("LoRa radio mode set to standby!\n");
            }
            break;
        case MODE_FS_TX: // FS Mode TX
            wRFM(REG_01_OP_MODE, MODE_FS_TX);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_FS_TX)) {
                 printf("LoRa radio mode set to FS TX mode!\n");
            }
            break;
        case MODE_TX: // TX Mode
            wRFM(REG_01_OP_MODE, MODE_TX);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_TX)) {
                printf("LoRa radio mode set to TX mode!\n");
            }
            break;
        case MODE_FS_RX: // FS Mode RX
            wRFM(REG_01_OP_MODE, MODE_FS_RX);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_FS_RX)) {
                printf("LoRa radio mode set to FS RX mode!\n");
            }
            break;
        case MODE_RXCONTINUOUS: // RX Continuous
            wRFM(REG_01_OP_MODE, MODE_RXCONTINUOUS);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_RXCONTINUOUS)) {
                 printf("LoRa radio mode set to RX cont!\n");
            }
            break;
        default:
            wRFM(REG_01_OP_MODE, MODE_SLEEP);
            sleep(1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_SLEEP)) {
                printf("LoRa radio mode set to sleep!\n");
            }
    }
}

// ========== Low-level IO functions beyond this point ========== //
/**
 * @name: wRFM
 * 
 * @brief: The following function writes one byte of data
 *          to the RFM module via SPI.
 * 
 * Inputs
 *  @param: ad - Address of the register for which will be set to val.
 *  @param: val - Value being sent and what will be set in the register.
 * 
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void wRFM(uint8_t ad, uint8_t val){
    // Preparation
    uint8_t ad_buf_tx[1] = {ad | 128}, ad_buf_rx[1] = {0}; 
    uint8_t val_buf_tx[1] = {val}, val_buf_rx[1] = {0};

    // Pull CE0 and transfer data
    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(val_buf_tx, val_buf_rx, sizeof(val_buf_tx));
    bcm2835_gpio_write(nss, 1);
}

/**
 * @name: bwRFM
 * 
 * @brief: The following function writes n number of bytes 
 *          of data to the RFM module via SPI.
 * 
 * Inputs
 *  @param: ad - Address of the register for which will be set to val.
 *  @param: val - Data array being sent and what will be set in the register.
 *  @param: n - Number of bytes to be transmitted.
 * 
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void bwRFM(uint8_t ad, uint8_t vals[], int n) { 
    // Preparation
    uint8_t ad_buf_tx[1] = {ad | 128}, ad_buf_rx[1] = {0}; 
    uint8_t val_buf_rx[n];

    // Pull CE0 and transfer data
    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(vals, val_buf_rx, sizeof(val_buf_rx));
    bcm2835_gpio_write(nss, 1);
}

/**
 * @name: rRFM
 * 
 * @brief: The following function reads one byte of data
 *          from the RFM module via SPI.
 * 
 * Inputs
 *  @param: ad - Address of the register we are reading data from.
 * 
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: val_buf_rx - One byte of data being read from ad register.
*/
uint8_t rRFM(uint8_t ad){
    // Preparation
    uint8_t ad_buf_tx[1] = {ad & 0b01111111}, ad_buf_rx[1] = {0}; 
    uint8_t val_buf_tx[1] = {0}, val_buf_rx[1] = {0};

    // Pull CE0 and read data
    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(val_buf_tx, val_buf_rx, sizeof(val_buf_tx));
    bcm2835_gpio_write(nss, 1);
    return val_buf_rx[0];
}

/**
 * @name: brRFM
 * 
 * @brief: The following function reads n number bytes
 *          of data from the RFM module via SPI.
 * 
 * Inputs
 *  @param: ad - Address of the register we are reading data from.
 *  @param: vals - Pointer to the array where we will write the received data.
 *  @param: len - Length of the received data packet. AKA how much data 
 *                we will fetch from the RFM buffer.
 * Outputs: 
 *  @param: vals - Data array that will store the received data.
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void brRFM(uint8_t ad, uint8_t vals[], uint8_t len) { 
    // Preparation
    uint8_t ad_buf_tx[1] = {ad & 0x7F}, ad_buf_rx[1] = {0}; 

    uint8_t val_buf_tx[len];
    // Play it safe and zero val_buf_tx
    for (int i=0; i<len; i++) {
        val_buf_tx[i] = 0;
    }

    // Pull CE0 and read data!
    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(val_buf_tx, vals, sizeof(val_buf_tx));
    bcm2835_gpio_write(nss, 1);
}

/**
 * @name: bitRead
 * 
 * @brief: Reads a bit of a variable, e.g. bool, int. Note that float & double are not supported. 
 *         You can read the bit of variables up to an unsigned long long (64 bits / 8 bytes).
 *         Inspired by: https://www.arduino.cc/reference/en/language/functions/bits-and-bytes/bitread/
 * 
 * Inputs
 *  @param: x - Byte we are reading a bit from. 
 *  @param: n - Number of the bit we are trying to extract
 * 
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: bit - The lonely bit we are trying to read.
*/
uint8_t bitRead(uint8_t x, uint8_t n) {
    return (x >> n) & 0b1;
}