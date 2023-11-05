/**
 * client.c
 * 
 * Simple socket client from geeksforgeeks.org
 * Link -> https://www.geeksforgeeks.org/socket-programming-cc/
 * 
 * Purpose: Utilizing a simple socket client to simulate Cubesat 
 * message sending and receiving. 
 * 
 * Files edited by: D.J. Morvay (dmorvay@andrew.cmu.edu)
*/

/* Includes */
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "helpers/header.h"
#include "helpers/payload.h"
#define PORT 8080

/* Function Prototypes */
void client_sigint_handler(int sig);

/**
 * main
 * 
 * Purpose: Main routine for client tasks.
 * Sets up a socket client to send and receive information from the server.
 * 
 * Inputs:
 *  argc
 *  argv[]
 * 
 * Return: NONE
 * 
 * Saved States: NONE
*/
int main(int argc, char const* argv[])
{
    /* Client configuration variables */
    int status, client_fd;
    ssize_t bytes_received_client;
    struct sockaddr_in serv_addr;
    ssize_t payload_received_client = -1;
    uint8_t message_buf[256] = {0};

    /* Sent or received values */
    space_payload my_payload;
    my_payload.header_payload.version_number = 0b0;
    my_payload.header_payload.packet_type = 0b0;
    my_payload.header_payload.sec_header_flag = 0b0;
    my_payload.header_payload.APID = 100;
    my_payload.header_payload.sequence_flag = 0b11;
    my_payload.header_payload.sequence_count = 0;
    my_payload.header_payload.data_length = 1;
    my_payload.message_payload.message_ptr = (uint8_t*)calloc(my_payload.header_payload.data_length, sizeof(uint8_t));

    // Test Signals //
    msg_signal test_sig1;
    test_sig1.start_bit = 0;
    test_sig1.end_bit = 7;
    uint8_t dummy1 = 0b00000000;

    pack_payload(my_payload.message_payload.message_ptr,dummy1,test_sig1);

    /* SIGINT Handler*/
    signal(SIGINT, client_sigint_handler);

    /* Create client socket */
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
  
    /* Convert IPv4 and IPv6 addresses from text to binary form */
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    /* Connect client to the server */
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))< 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    char cmd[50];
    while(1) {
        printf("Enter command: ");
        fgets(cmd, 50, stdin);

        memcpy(message_buf, &my_payload.header_payload, sizeof(my_payload.header_payload));
        memcpy(message_buf+sizeof(my_payload.header_payload),my_payload.message_payload.message_ptr,my_payload.header_payload.data_length);

        // Send header
        if (send(client_fd, message_buf, sizeof(my_payload.header_payload)+my_payload.header_payload.data_length, 0) == -1) {
            perror("Send failed");
            break;
        } 
        else {
            printf("Message sent from cubesat!\n\n");
        }

        // Get response from groundstation
        payload_received_client = recv(client_fd, message_buf, sizeof(message_buf), 0);

        // Display Header Information
        if (payload_received_client == -1) {
            perror("Receive header failed");
            break;
        } else if (payload_received_client == 0) {
            printf("Server closed the connection.\n");
            break;
        } else {
            memcpy(&my_payload.header_payload, message_buf, sizeof(my_payload.header_payload));
            memcpy(my_payload.message_payload.message_ptr, message_buf+sizeof(my_payload.header_payload), my_payload.header_payload.data_length);

            printf("Version number: %u\n",my_payload.header_payload.version_number);
            printf("Packet type: %u\n",my_payload.header_payload.packet_type);
            printf("Secondary header flag: %u\n",my_payload.header_payload.sec_header_flag);
            printf("APID: %u\n",my_payload.header_payload.APID);

            printf("Sequence flag: %u\n",my_payload.header_payload.sequence_flag);
            printf("Sequence count: %u\n",my_payload.header_payload.sequence_count);
            printf("Data length: %u\n",my_payload.header_payload.data_length);

            dummy1 = (uint8_t)unpack_payload(my_payload.message_payload.message_ptr, test_sig1);
            printf("Message: %u\n\n",dummy1);
        }
    }

    /* Free memory */
    free(my_payload.message_payload.message_ptr);
  
    /* closing the connected socket */
    close(client_fd);
    return 0;
}

/**
 * Function: client_sigint_handler
 * 
 * Purpose: Report SIGINT when CTRL-C is pressed.
*/
void client_sigint_handler(int sig) {
    printf("Shutting down client!\n");
    exit(0);
}