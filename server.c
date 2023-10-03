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
#include "helpers/header.h"
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
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    /* Sent or received values */
    header rec_head;
  
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

    valread = recv(new_socket, &rec_head, sizeof(rec_head),0);
    printf("Version number: %u\n",rec_head.version_number);
    printf("Packet type: %u\n",rec_head.packet_type);
    printf("Secondary header flag: %u\n",rec_head.sec_header_flag);
    printf("APID: %u\n",rec_head.APID);

    printf("Sequence flag: %u\n",rec_head.sequence_flag);
    printf("Sequence count: %u\n",rec_head.sequence_count);
    printf("Data length: %u\n\n",rec_head.data_length);

    rec_head.APID += 10;
    rec_head.data_length += 100;
    
    send(new_socket, &rec_head, sizeof(rec_head), 0);
    printf("Header sent back to client!\n\n");
  
    /* closing the connected socket */
    close(new_socket);
    /* closing the listening socket */
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}