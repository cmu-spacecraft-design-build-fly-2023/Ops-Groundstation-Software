#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include "RFM98W_lib.h"

/* Function Prototypes */
void GS_sigint_handler(int sig);

int main() {
    /* SIGINT Handler s*/
    signal(SIGINT, GS_sigint_handler);

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
        bcm2835_spi_set_speed_hz(5000000); // The default

        // We control CS line manually don't assert CEx line!
        bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

        configure();
        set_mode_RX();

        while(1) {
            // we got it ?
            // printf("%u\n",bcm2835_gpio_lev(dio0));
            if (bcm2835_gpio_eds(dio0)) {
                // Now clear the eds flag by setting it to 1
                bcm2835_gpio_set_eds(dio0);
                printf("Rising event detect for pin GPIO%d\n", dio0);
                Packet rx;
                // endRX(&rx, &rfm_done, &rfm_status);
                for(uint8_t i = 0;i<rx.len;i++){
                   printf("%u ",rx.data[i]);
                }
                printf("\n");
            }
        }
    }
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

/**
 * Function: client_sigint_handler
 * 
 * Purpose: Report SIGINT when CTRL-C is pressed.
*/
void GS_sigint_handler(int sig) {
    printf("Shutting down Groundstation!\n");
    radioMode(MODE_SLEEP);
    bcm2835_spi_end();
    bcm2835_close();
    exit(0);
}