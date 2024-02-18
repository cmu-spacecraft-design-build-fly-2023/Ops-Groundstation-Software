#include <stdint.h>
#include "header.h"


/* ---------- Getter functions ---------- */
/** Function: get_version_number
 *  Address Info: Byte 0, Bits 0-2
 */
void get_version_number(header* data) {
    (*data).version_number = ((*data).header_array[0] >> 5) & (uint8_t)(0b00000111);
}

/** Function: get_packet_type
 *  Address Info: Byte 0, Bit 3
 */
void get_packet_type(header* data) {
    (*data).packet_type = ((*data).header_array[0] >> 4) & (uint8_t)(0b00000001);
}

/** Function: get_secondary_header_flag
 *  Address Info: Byte 0, Bit 4
 */
void get_sec_header_flag(header* data) {
    (*data).sec_header_flag = ((*data).header_array[0] >> 3) & (uint8_t)(0b00000001);
}

/** Function: get_APID
 *  Address Info: 
 *      MSB - Byte 0, Bits 5-7
 *      LSB - Byte 1, Bits 0-7
 */
void get_APID(header* data) {
    (*data).APID = (((uint16_t)((*data).header_array[0]) & (uint16_t)(0b00000111)) << 8) + (*data).header_array[1];
}

/** Function: get_sequence_flag
 *  Address Info: Byte 2, Bits 0-1
 */
void get_sequence_flag(header* data) {
    (*data).sequence_flag = ((*data).header_array[2] >> 6) & (uint8_t)(0b00000011);
}

/** Function: get_sequence_count
 *  Address Info: 
 *      MSB - Byte 2, Bits 2-7
 *      LSB - Byte 3, Bits 0-7
 */
void get_sequence_count(header* data) {
    (*data).sequence_count = (((uint16_t)((*data).header_array[2]) & (uint16_t)(0b00111111)) << 8) + (*data).header_array[3];
}

/** Function: get_data_length
 *  Address Info: 
 *      MSB - Byte 4, Bits 0-7
 *      LSB - Byte 5, Bits 0-7
 */
void get_data_length(header* data) {
    (*data).data_length = ((uint16_t)((*data).header_array[4]) << 8) + (*data).header_array[5];
}


/* ---------- Setter functions ---------- */
/** Function: set_version_number
 *  Address Info: Byte 0, Bits 0-2
 */
void set_version_number(header* data) {
    (*data).header_array[0] = (((*data).header_array[0] & (uint8_t)0b00011111) | ((*data).version_number << 5));
} 

/** Function: set_packet_type
 *  Address Info: Byte 0, Bits 3
 */
void set_packet_type(header* data) {
    (*data).header_array[0] = (((*data).header_array[0] & (uint8_t)0b11101111) | ((*data).packet_type << 4));
} 

/** Function: set_secondary_header_flag
 *  Address Info: Byte 0, Bit 4
 */
void set_sec_header_flag(header* data) {
    (*data).header_array[0] = (((*data).header_array[0] & (uint8_t)0b11110111) | ((*data).sec_header_flag << 3));
}

/** Function: set_APID
*  Address Info: 
 *      MSB - Byte 0, Bits 5-7
 *      LSB - Byte 1, Bits 0-7
 */ 
void set_APID(header* data) {
    (*data).header_array[0] = (((*data).header_array[0] & (uint8_t)0b11111000) | (uint8_t)(((*data).APID >> 8) & 0b00000111));
    (*data).header_array[1] = (uint8_t)((*data).APID & 0b11111111);
}

/** Function: set_sequence_flag
*   Address Info: Byte 2, Bits 0-1
 */ 
void set_sequence_flag(header* data) {
    (*data).header_array[2] = (((*data).header_array[2] & (uint8_t)0b00111111) | ((*data).sequence_flag << 6));
}

/** Function: set_sequence_count
 *  Address Info: 
 *      MSB - Byte 2, Bits 2-7
 *      LSB - Byte 3, Bits 0-7
*/
void set_sequence_count(header* data) {
    (*data).header_array[2] = (((*data).header_array[2] & (uint8_t)0b11000000) | (uint8_t)(((*data).sequence_count >> 8) & 0b00111111));
    (*data).header_array[3] = (uint8_t)((*data).sequence_count & 0b11111111);
}

/** Function: set_data_length
 *  Address Info: 
 *      MSB - Byte 4, Bits 0-7
 *      LSB - Byte 5, Bits 0-7
*/
void set_data_length(header* data) {
    (*data).header_array[4] = (uint8_t)(((*data).data_length >> 8) & 0b11111111);
    (*data).header_array[5] = (uint8_t)((*data).data_length & 0b11111111);
}


/* ---------- Implementation Functions ---------- */

void unpack(header* data) {
    get_version_number(data);
    get_packet_type(data);
    get_sec_header_flag(data);
    get_APID(data);
    get_sequence_flag(data);
    get_sequence_count(data);
    get_data_length(data);
}

void pack(header* data) {
    set_version_number(data);
    set_packet_type(data);
    set_sec_header_flag(data);
    set_APID(data);
    set_sequence_flag(data);
    set_sequence_count(data);
    set_data_length(data);
}