#include "RFM98W_lib.h"
#include <bcm2835.h>
#include <stdint.h>
#include <stdbool.h>

volatile bool rfm_done;
uint8_t rfm_status;//0=idle,1=tx,2=rx

//Set to true to enable high frequency mode
bool high_frequency = false;
const uint32_t Fosc = 32000000;
uint8_t nss, dio0, dio5, rfm_rst;//pins - to be assigned at instantiation.

void RFMLib(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    // Chip Select - GPIO8
    nss = a;
    // DIO0 - GPIO19
    dio0 = b;
    // DIO5 - GPIO16
    dio5 = c;
    // RFM_RST - GPIO12
    rfm_rst = d;
    rfm_done = false;
    rfm_status = 0;
}

void configure(uint8_t config[6]){
    bcm2835_gpio_fsel(nss, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(nss, 1);

    if(rfm_rst!=255){
        bcm2835_gpio_fsel(rfm_rst, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(rfm_rst, 1);
    }

    // Set RPI GPIO19 or DIO0
    bcm2835_gpio_fsel(dio0, BCM2835_GPIO_FSEL_INPT);
    //  with a pulldown
    bcm2835_gpio_set_pud(dio0, BCM2835_GPIO_PUD_DOWN);

    if(dio5!=255) {
        // Set RPI GPIO16 or DIO5
        bcm2835_gpio_fsel(dio5, BCM2835_GPIO_FSEL_INPT);
        //  with a pulldown
        bcm2835_gpio_set_pud(dio5, BCM2835_GPIO_PUD_DOWN);
    }

    radioMode(0);
    wRFM(0x1D,config[0]);
    wRFM(0x1E,config[1]);//modem config registers
    wRFM(0x09,config[2]);//use PA_BOOST - even at the same power setting
    //it seems to give a stronger RSSI.

    //Use setFrequency for frequency adjustment

    //wRFM(0x07,config[3]);//freq to 434.7MHz - mid SB
    //wRFM(0x08,config[4]);//freq -LSB
	wRFM(0x26, config[5]);
}

void setFrequency(uint32_t frequency) {
    uint32_t freqVal = ((uint64_t)(frequency) << 19ULL) / Fosc;

    wRFM(0x06, (freqVal >> 16) & 0xFF);
    wRFM(0x07, (freqVal >> 8) & 0xFF);
    wRFM(0x08, (freqVal) & 0xFF);

}

uint8_t getVersion() {
    return rRFM(0x42);
}

void beginRX(){
    rfm_status = 2;
    rfm_done = false;
    radioMode(1);
    wRFM(0x12,255);//reset IRQ
    wRFM(0x0D,rRFM(0x0F));//set RX base address
    wRFM(0x40,0);//set up DIO0 interrupt
    radioMode(2);
    //You need to attach an interrupt function which sets this object's "rfm_done" bool to TRUE on a RISING interrupt on DIO0
}

void endRX(Packet* received){//function to be called on, or soon after, reception of RX_DONE interrupt
    rfm_done = false;
    rfm_status = 0;
    radioMode(1);//stby
    uint8_t len = rRFM(0x13);//length of packet
    (*received).len = len;
    uint8_t packet[(int)len];

    if(bitRead(rRFM(0x12),5)){
        (*received).crc = false;
    }
    else{
	    (*received).crc = true;
        wRFM(0x0D,0);
        brRFM(0x00,(*received).data,len);
    }

    (*received).rssi = (int)(rRFM(0x1B)-137);
    uint8_t rawSNR = rRFM(0x19);

    if(bitRead(rawSNR,7)){
        (*received).snr = 0-(255-rawSNR);
    }
    else{
        (*received).snr = rawSNR;
    }

    (*received).snr /= 4;
    radioMode(0);//now sleeps
    wRFM(0x12,255);//clear IRQ again.
}

void endTX() {//function to be called at the end of transmission; cleans up.
    rfm_status = 0;
    rfm_done = false;
    radioMode(1);//stby
    wRFM(0x12,255);//clear IRQ
    radioMode(0);//sleep
}

void beginTX(Packet transmit_pkt){
    rfm_status = 1;
    rfm_done = false;
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
        case 0://sleep
            wRFM(0x01,0x80 | (high_frequency << 3));
            break;
        case 1://stby
            wRFM(0x01,0x81 | (high_frequency << 3));
            break;
        case 2://rx cont
            wRFM(0x01,0x85 | (high_frequency << 3));
            break;
        case 3://rx single
            wRFM(0x01,0x86 | (high_frequency << 3));
            break;
        case 4://tx
            wRFM(0x01,0x83 | (high_frequency << 3));
            break;
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