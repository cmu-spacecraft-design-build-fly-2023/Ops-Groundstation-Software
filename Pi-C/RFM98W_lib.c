#include "RFM98W_lib.h"
#include <bcm2835.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

void configure() {
    bcm2835_gpio_fsel(nss, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(nss, 1);

    if(rfm_rst!=255){
        bcm2835_gpio_fsel(rfm_rst, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(rfm_rst, 1);
    }

    // Set RPI pin to be an input
    bcm2835_gpio_fsel(dio0, BCM2835_GPIO_FSEL_INPT);
    //  with a puldown
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

    // Set Modem config
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

    uint8_t PA_CONFIG = ((uint8_t)(PA_SELECT << 7)) + ((uint8_t)(MAX_PWR << 4)) + ((uint8_t)OUTPUT_PWR);
    wRFM(REG_09_PA_CONFIG, PA_CONFIG);
    if (rRFM(REG_09_PA_CONFIG) == PA_CONFIG) {
        printf("PA configuration set!\n");
    }
}

void setFrequency(uint32_t frequency) {
    uint32_t freqVal = 14991360;
    // (frequency * 1000000) / (FXOSC / 524288);
    printf("%u\n",freqVal);
    wRFM(REG_06_FRF_MSB, (freqVal >> 16) & 0xFF); // MSB
    wRFM(REG_07_FRF_MID, (freqVal >> 8) & 0xFF); // MID
    wRFM(REG_08_FRF_LSB, (freqVal) & 0xFF); // LSB
    uint32_t getfreq = (((uint32_t)rRFM(REG_06_FRF_MSB)) << 16) + (((uint32_t)rRFM(REG_07_FRF_MID)) << 8) + ((uint32_t)rRFM(REG_08_FRF_LSB));
    printf("Frequency set to: %u Mhz\n",(getfreq*(FXOSC / 524288))/1000000);
}

uint8_t getVersion() {
    return rRFM(0x42);
}

void set_mode_RX() {
    if ((rRFM(REG_01_OP_MODE) & MODE_RXCONTINUOUS) != MODE_RXCONTINUOUS) {
        radioMode(MODE_RXCONTINUOUS);
        wRFM(REG_40_DIO_MAPPING1, 0x0);
    }
}

void RX_transmission(Packet* received) {//function to be called on, or soon after, reception of RX_DONE interrupt
    uint32_t getfreq = (((uint32_t)rRFM(REG_06_FRF_MSB)) << 16) + (((uint32_t)rRFM(REG_07_FRF_MID)) << 8) + ((uint32_t)rRFM(REG_08_FRF_LSB));
    uint8_t irq_flags = rRFM(REG_12_IRQ_FLAGS);

    if (((rRFM(REG_01_OP_MODE) & MODE_RXCONTINUOUS) == MODE_RXCONTINUOUS) && (irq_flags & RX_DONE)) {
        (*received).len = rRFM(REG_13_RX_NB_BYTES);
        wRFM(REG_0D_FIFO_ADDR_PTR,rRFM(REG_10_FIFO_RX_CURRENT_ADDR));

        brRFM(REG_00_FIFO, (*received).data, (*received).len);
        wRFM(REG_12_IRQ_FLAGS, 0xFF);

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
    set_mode_RX();
    wRFM(REG_12_IRQ_FLAGS,0xFF);//clear IRQ again.
}

void endTX(bool* rfm_done, uint8_t* rfm_status) {//function to be called at the end of transmission; cleans up.
    *rfm_status = 0;
    *rfm_done = false;
    radioMode(1);//stby
    wRFM(0x12,255);//clear IRQ
    radioMode(0);//sleep
}

void beginTX(Packet transmit_pkt, bool* rfm_done, uint8_t* rfm_status){
    *rfm_status = 1;
    *rfm_done = false;
    radioMode(1);//stby
    wRFM(0x12,255);//clear IRQ

    wRFM(0x22,transmit_pkt.len);//set payload length;
    uint8_t base_addr = rRFM(0x0E);
    wRFM(0x0D,base_addr);//Put transmit base FIFO addr in FIFO pointer
    uint8_t new_data[transmit_pkt.len];

    for(int i = 0;i<transmit_pkt.len;i++){
        new_data[i] = transmit_pkt.data[i];
    }

    bwRFM(0x00,new_data,transmit_pkt.len);
    wRFM(0x0D,base_addr);//reset FIFO pointer
    wRFM(0x0D,base_addr);//and again...
    wRFM(0x40,0x40);//arm DIO0 interrupt
    radioMode(4);//begin transmit
    //you need to attach a rising interrupt on DIO0.
}

void radioMode(uint8_t m){//set specified mode
    switch(m){
        case MODE_SLEEP: // Sleep Mode
            wRFM(REG_01_OP_MODE, LONG_RANGE_MODE | MODE_SLEEP);
            sleep(0.5);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_SLEEP)) {
                printf("LoRa radio mode set to sleep!\n");
            }
            break;
        case MODE_STDBY: // Standby Mode
            wRFM(REG_01_OP_MODE, MODE_STDBY);
            sleep(0.5);
            printf("%u\n",rRFM(REG_01_OP_MODE));
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_STDBY)) {
                 printf("LoRa radio mode set to standby!\n");
            }
            break;
        case MODE_FS_TX: // FS Mode TX
            wRFM(REG_01_OP_MODE, MODE_FS_TX);
            sleep(0.1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_FS_TX)) {
                 printf("LoRa radio mode set to FS TX mode!\n");
            }
            break;
        case MODE_TX: // TX Mode
            wRFM(REG_01_OP_MODE, MODE_TX);
            sleep(0.1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_TX)) {
                printf("LoRa radio mode set to TX mode!\n");
            }
            break;
        case MODE_FS_RX: // FS Mode RX
            wRFM(REG_01_OP_MODE, MODE_FS_RX);
            sleep(0.1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_FS_RX)) {
                printf("LoRa radio mode set to FS RX mode!\n");
            }
            break;
        case MODE_RXCONTINUOUS: // RX Continuous
            wRFM(REG_01_OP_MODE, MODE_RXCONTINUOUS);
            sleep(0.1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_RXCONTINUOUS)) {
                 printf("LoRa radio mode set to RX cont!\n");
            }
            break;
        default:
            wRFM(REG_01_OP_MODE, MODE_SLEEP);
            sleep(0.1);
            if (rRFM(REG_01_OP_MODE) == (LONG_RANGE_MODE | MODE_SLEEP)) {
                printf("LoRa radio mode set to sleep!\n");
            }
    }
}

//Low-level IO functions beyond this point. ============================================
void wRFM(uint8_t ad, uint8_t val){//single byte write
    uint8_t ad_buf_tx[1] = {ad | 128}, ad_buf_rx[1] = {0}; //set wrn bit - WRITE = 1
    uint8_t val_buf_tx[1] = {val}, val_buf_rx[1] = {0};

    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(val_buf_tx, val_buf_rx, sizeof(val_buf_tx));
    bcm2835_gpio_write(nss, 1);
}

void bwRFM(uint8_t ad, uint8_t vals[], int n){ //burst write - less efficient but faster
    //for multiple bits
    // less efficient for singles due to array overhead, etc
    uint8_t ad_buf_tx[1] = {ad | 128}, ad_buf_rx[1] = {0}; //set wrn bit - WRITE = 1
    uint8_t val_buf_rx[n];

    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(vals, val_buf_rx, sizeof(val_buf_rx));
    bcm2835_gpio_write(nss, 1);
}

uint8_t rRFM(uint8_t ad){//single byte read
    uint8_t ad_buf_tx[1] = {ad & 0b01111111}, ad_buf_rx[1] = {0}; //wrn bit low
    uint8_t val_buf_tx[1] = {0}, val_buf_rx[1] = {0};

    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(val_buf_tx, val_buf_rx, sizeof(val_buf_tx));
    bcm2835_gpio_write(nss, 1);
    return val_buf_rx[0];
}

void brRFM(uint8_t ad, uint8_t vals[], uint8_t len) { //burst read
    uint8_t ad_buf_tx[1] = {ad & 0x7F}, ad_buf_rx[1] = {0}; //wrn bit low

    uint8_t val_buf_tx[len];
    // Play it safe and zero val_buf_tx
    for (int i=0; i<len; i++) {
        val_buf_tx[i] = 0;
    }

    bcm2835_gpio_write(nss, 0);
    bcm2835_spi_transfernb(ad_buf_tx, ad_buf_rx, sizeof(ad_buf_tx));
    bcm2835_spi_transfernb(val_buf_tx, vals, sizeof(val_buf_tx));
    bcm2835_gpio_write(nss, 1);
}

uint8_t bitRead(uint8_t x, uint8_t n) {
    return (x >> n) & 0b1;
}