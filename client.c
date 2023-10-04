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

    /* Sent or received values */
    header send_head;
    send_head.version_number = 0b0;
    send_head.packet_type = 0b0;
    send_head.sec_header_flag = 0b0;
    send_head.APID = 100;
    send_head.sequence_flag = 0b11;
    send_head.sequence_count = 0;
    send_head.data_length = 100;

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

        if (send(client_fd, &send_head, sizeof(send_head), 0) == -1) {
            perror("Send failed");
            break;
        } 
        else {
            printf("Packet sent from cubesat!\n\n");
        }

        int bytes_received = recv(client_fd, &send_head, sizeof(send_head), 0);
        if (bytes_received == -1) {
            perror("Receive failed");
            break;
        } else if (bytes_received == 0) {
            printf("Server closed the connection.\n");
            break;
        } else {
            printf("Version number: %u\n",send_head.version_number);
            printf("Packet type: %u\n",send_head.packet_type);
            printf("Secondary header flag: %u\n",send_head.sec_header_flag);
            printf("APID: %u\n",send_head.APID);

            printf("Sequence flag: %u\n",send_head.sequence_flag);
            printf("Sequence count: %u\n",send_head.sequence_count);
            printf("Data length: %u\n\n",send_head.data_length);
        }
    }
  
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