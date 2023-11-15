#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
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
                RX_transmission(&rx);
                bool msg_unpack = false;
                for(uint8_t i = 0;i<rx.len;i++){
                    if (i>0) {
                        if (((rx.data[i-1] == 0) && (rx.data[i] != 0)) || msg_unpack == true) {
                            printf("%c",(char)rx.data[i]);
                            msg_unpack = true;
                        }
                    }
                }
                msg_unpack = false;
                printf("\n");
                printf("Communication complete!\n");
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
    // bcm2835_spi_end();
    bcm2835_close();
    exit(0);
}