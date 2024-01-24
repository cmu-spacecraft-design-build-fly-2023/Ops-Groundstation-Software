/**
 * @name: RFM_TX
 * 
 * @authors: David J. Morvay (dmorvay@andrew.cmu.edu)
 * Carnegie Mellon University
 * Fall 2023 - Spring 2024
 * ECE 18-873 - Spacecraft Build, Design, & Fly Lab
 * Satellite <> Groundstation Communications
 * 
 * @brief: This file transmits one packet of data from the RFM98 module.
*/
#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "RFM98W_lib.h"

/* Function Prototypes */
void GS_sigint_handler(int sig);

/**
 * @name: main()
 * 
 * @brief: The following function attempts to transmit one packet
 *          of data and then waits for the TX DONE interrupt before exiting. 
 * 
 * Inputs: NONE
 * Outputs: NONE
 * 
 * Saved Values: NONE
 * @return: ints
*/
int main() {
    // SIGINT Handler
    signal(SIGINT, GS_sigint_handler);

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

        printf("%u\n",getVersion());

        // Create packet we want to send
        Packet TX_packet;
        const char my_msg[] = "D.J. was here!";
        TX_packet.len = strlen(my_msg)+4;

        TX_packet.data[0] = 10;
        TX_packet.data[1] = 2;
        TX_packet.data[2] = 0;
        TX_packet.data[3] = 0;

        for (size_t i = 0; i < TX_packet.len; i++) {
            TX_packet.data[i+4] = (uint8_t)my_msg[i];
        }

        TX_transmission(TX_packet);

        while(1) {
            // Wait for rising edge on dio0
            if (bcm2835_gpio_eds(dio0)) {
                // Clear the eds flag on dio0
                bcm2835_gpio_set_eds(dio0);
                printf("Rising event detect for pin GPIO%d\n", dio0);

                // Message sent, reset mode to standby
                radioMode(MODE_STDBY);
                wRFM(REG_12_IRQ_FLAGS,0xFF);
                printf("Message successfully sent!\n");
                break;
            }
        }
    }
    // Close I/O and SPI connection
    // bcm2835_spi_end();
    bcm2835_close();
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
    exit(0);
}