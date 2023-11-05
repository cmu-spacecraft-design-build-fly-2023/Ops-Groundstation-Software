/**
 * server.c
 * 
 * Simple socket server from geeksforgeeks.org
 * Link -> https://www.geeksforgeeks.org/socket-programming-cc/
 * 
 * Purpose: Utilizing a simple socket server to simulate Cubesat 
 * message sending and receiving. 
 * 
 * Files edited by: D.J. Morvay (dmorvay@andrew.cmu.edu)
*/

/* Includes */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "helpers/header.h"
#include "helpers/payload.h"
#define PORT 8080

/**
 * main
 * 
 * Purpose: Main routine for server tasks.
 * Sets up a socket server on port 8080 and waits for a message from the client. 
 * 
 * Inputs:
 *  argc
 *  argv[]
 * 
 * Return: NONE
 * 
 * Saved States: NONE
*/
int main(int argc, char const* argv[]) {
    /* Server configuration variables */
    int server_fd, new_socket;
    ssize_t payload_received_server;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    uint8_t message_buf[256];

    /* Sent or received values */
    space_payload rec_payload;
    msg_signal rec_sig1;
    rec_sig1.start_bit = 0;
    rec_sig1.end_bit = 7;
    uint8_t rec_dummy;
  
    /* Creating socket file descriptor */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    /* Forcefully attaching socket to the port 8080 */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  
    /* Forcefully attaching socket to the port 8080 */
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen))< 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    /* Loop and wait for message from client (Cubesat) */
    while(1) {

        // Get complete payload from Cubesat //
        payload_received_server = recv(new_socket, message_buf, sizeof(message_buf),0);
    
        /**
         *  If message is received, unpack and report data. 
         *  If client closed session, break.
         *  Else, reloop and check again.
        */
        if (payload_received_server > 0) {
            printf("Packet received at ground station!\n");

            rec_payload.message_payload.message_ptr = (uint8_t*)malloc(rec_payload.header_payload.data_length);

            memcpy(&rec_payload.header_payload, message_buf, sizeof(rec_payload.header_payload));
            memcpy(rec_payload.message_payload.message_ptr, message_buf+sizeof(rec_payload.header_payload), rec_payload.header_payload.data_length);

            printf("Version number: %u\n",rec_payload.header_payload.version_number);
            printf("Packet type: %u\n",rec_payload.header_payload.packet_type);
            printf("Secondary header flag: %u\n",rec_payload.header_payload.sec_header_flag);
            printf("APID: %u\n",rec_payload.header_payload.APID);

            printf("Sequence flag: %u\n",rec_payload.header_payload.sequence_flag);
            printf("Sequence count: %u\n",rec_payload.header_payload.sequence_count);
            printf("Data length: %u\n",rec_payload.header_payload.data_length);

            rec_dummy = unpack_payload(rec_payload.message_payload.message_ptr,rec_sig1);
            printf("Message Contents: %u\n\n",rec_dummy);
            pack_payload(rec_payload.message_payload.message_ptr,rec_dummy+1,rec_sig1);

            memcpy(message_buf, &rec_payload.header_payload, sizeof(rec_payload.header_payload));
            memcpy(message_buf+sizeof(rec_payload.header_payload), rec_payload.message_payload.message_ptr, rec_payload.header_payload.data_length);
            
            send(new_socket, message_buf, sizeof(rec_payload.header_payload)+rec_payload.header_payload.data_length, 0);
            printf("Message sent back to cubesat!\n\n");

            /* Free memory */
            free(rec_payload.message_payload.message_ptr);
        } 
        else if (payload_received_server == 0) {
            break;
        }
    }
  
    /* closing the connected socket */
    close(new_socket);
    /* closing the listening socket */
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}