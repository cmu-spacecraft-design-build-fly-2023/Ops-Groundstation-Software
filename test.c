#include <stdio.h>
#include <stdint.h>
#include "helpers/header.h"

int main() {
    header rec, send;

    rec.header_array[0] = 0b00111101;
    rec.header_array[1] = 0b01010101;

    rec.header_array[2] = 0b11101010;
    rec.header_array[3] = 0b11111111;
    rec.header_array[4] = 0b11111010;
    rec.header_array[5] = 0b10101111;

    /* Unpack Header Information */
    unpack(&rec);

    printf("Version number: %u\n",rec.version_number);
    printf("Packet type: %u\n",rec.packet_type);
    printf("Secondary header flag: %u\n",rec.sec_header_flag);
    printf("APID: %u\n",rec.APID);

    printf("Sequence flag: %u\n",rec.sequence_flag);
    printf("Sequence count: %u\n",rec.sequence_count);
    printf("Data length: %u\n\n",rec.data_length);

    /* Package Header Information */
    send.version_number = rec.version_number;
    send.packet_type = rec.packet_type;
    send.sec_header_flag = rec.sec_header_flag;
    send.APID = rec.APID;
    send.sequence_flag = rec.sequence_flag;
    send.sequence_count = rec.sequence_count;
    send.data_length = rec.data_length;

    pack(&send);

    printf("Header array - byte 0: %u\n", send.header_array[0]);
    printf("Header array - byte 1: %u\n", send.header_array[1]);
    printf("Header array - byte 2: %u\n", send.header_array[2]);
    printf("Header array - byte 3: %u\n", send.header_array[3]);
    printf("Header array - byte 4: %u\n", send.header_array[4]);
    printf("Header array - byte 5: %u\n", send.header_array[5]);

    return 0;
}