/**
 * @name: RFM_rec
 * 
 * @authors: David J. Morvay (dmorvay@andrew.cmu.edu)
 * Carnegie Mellon University
 * Fall 2023 - Spring 2024
 * ECE 18-873 - Spacecraft Build, Design, & Fly Lab
 * Satellite <> Groundstation Communications
 * 
 * @brief: This file receives a packets of data RFM98 module.
*/
#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "RFM98W_lib.h"

FILE *file;

/* Function Prototypes */
void GS_sigint_handler(int sig);

/**
 * @name: main()
 * 
 * @brief: The following function waits for an interrupt on dio0 and
 *          unpacks the message received by the RFM module. 
 * 
 * Inputs: NONE
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: ints
*/
int main() {
    /* SIGINT Handler s*/
    signal(SIGINT, GS_sigint_handler);

    // Open the file in write mode
    file = fopen("rec.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Initialize I/O
    if (!bcm2835_init()) {
        printf("bcm2835_init failed. Are you running as root??\n");
    } 
    // Initialize SPI
    else if (!bcm2835_spi_begin()) {
        printf("bcm2835_spi_begin failed\n");
    } 
    else {
        // Configure the RFM module to receive packets
        configure();
        set_mode_RX();

        while(1) {
            // Wait for rising edge on dio0
            if (bcm2835_gpio_eds(dio0)) {
                // Clear the eds flag on dio0
                bcm2835_gpio_set_eds(dio0);
                printf("Rising event detect for pin GPIO%d\n", dio0);

                // Get received packet from RFM buffer
                Packet rx;
                RX_transmission(&rx);
                bool msg_unpack = false;

                // Print received message
                for(uint8_t i = 0;i<rx.len;i++){
                    if (i>0) {
                        if (((rx.data[i-1] == 0) && (rx.data[i] != 0)) || msg_unpack == true) {
                            printf("%c",(char)rx.data[i]);
                            msg_unpack = true;

                            if ((char)rx.data[i] != '\0') {
                                if (fputc((char)rx.data[i], file) == EOF) {
                                    perror("Error writing to file");
                                    fclose(file);
                                    return 1;
                                }
                            }
                        }
                    }
                }
                printf("\n");
                printf("Communication complete!\n");
            }
        }
    }
    // Close I/O and SPI connection
    bcm2835_spi_end();
    bcm2835_close();
    fclose(file);
    return 0;
}

/**
 * @name: GS_sigint_handler
 * 
 * @brief: The following function handles a SIGINT 
 *          and shuts down Pi I/0.
 * 
 * Inputs: 
 *  @param: SIGINT - CTRL-c press
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: NONE
*/
void GS_sigint_handler(int sig) {
    printf("Shutting down Groundstation!\n");
    radioMode(MODE_SLEEP);
    // bcm2835_spi_end();
    bcm2835_close();
    fclose(file);
    exit(0);
}