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
        // Init SPI
        bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
        bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
        bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default

        // We control CS line manually don't assert CEx line!
        bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

        uint8_t my_config[6] = {0x44,0x84,0x88,0xAC,0xCD,0x08};
        configure(my_config);

        // Set RPI pin to be an input
        bcm2835_gpio_fsel(dio0, BCM2835_GPIO_FSEL_INPT);
        //  with a puldown
        bcm2835_gpio_set_pud(dio0, BCM2835_GPIO_PUD_DOWN);
        // And a rising edge detect enable
        bcm2835_gpio_ren(dio0);

        while(1) {
            if(rfm_status == 0) {
                beginRX(); 
            }
            // we got it ?
            if (bcm2835_gpio_eds(dio0)) {
                // Now clear the eds flag by setting it to 1
                bcm2835_gpio_set_eds(dio0);
                printf("Rising event detect for pin GPIO%d\n", dio0);
                rfm_done = true;
                Packet rx;
                endRX(&rx);
                for(uint8_t i = 0;i<rx.len;i++){
                    printf("%u ",rx.data[i]);
                }
                printf("\n");
            }
        }
    }
    return 0;
}