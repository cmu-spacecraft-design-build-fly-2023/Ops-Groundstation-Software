#include <bcm2835.h>
#include <stdio.h>
#include "RFM98W_lib.h"

int main() {
    if (!bcm2835_init()) {
        printf("bcm2835_init failed. Are you running as root??\n");
    } 
    else if (!bcm2835_spi_begin()) {
        printf("bcm2835_spi_begin failed\n");
    } 
    else {
        bool rfm_done = false;
        uint8_t rfm_status = 0; //0=idle,1=tx,2=rx

        // Init SPI
        bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
        bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
        bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default

        // We control CS line manually don't assert CEx line!
        bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

        uint8_t my_config[6] = {0x72,0x74,0x04,0xAC,0xCD,0x08};
        configure(my_config);

        while(1) {
            if(rfm_status == 0) {
                printf("beginRx\n");
                beginRX(&rfm_done, &rfm_status); 
                // printf("beginRx complete!");
            }
            // we got it ?
            if (bcm2835_gpio_eds(dio0)) {
                // Now clear the eds flag by setting it to 1
                bcm2835_gpio_set_eds(dio0);
                printf("Rising event detect for pin GPIO%d\n", dio0);
                rfm_done = true;
                Packet rx;
                endRX(&rx, &rfm_done, &rfm_status);
                for(uint8_t i = 0;i<rx.len;i++){
                   printf("%u ",rx.data[i]);
                }
                printf("\n");
            }
        }
    }
    return 0;
}