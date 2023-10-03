#include <stdint.h>

#ifndef UNPACK
#define UNPACK

/* ---------- Header Struct ---------- */
typedef struct spp_header {
    uint8_t header_array[6];
    uint8_t version_number;
    uint8_t packet_type;
    uint8_t sec_header_flag;
    uint16_t APID;
    uint8_t sequence_flag;
    uint16_t sequence_count;
    uint16_t data_length;
} header;

/* ---------- Getter functions ---------- */
void get_version_number(header* data);
void get_packet_type(header* data);
void get_sec_header_flag(header* data);
void get_APID(header* data);
void get_sequence_flag(header* data);
void get_sequence_count(header* data);
void get_data_length(header* data);

/* ---------- Setter functions ---------- */
void set_version_number(header* data);
void set_packet_type(header* data);
void set_sec_header_flag(header* data);
void set_APID(header* data);
void set_sequence_flag(header* data);
void set_sequence_count(header* data);
void set_data_length(header* data);

/* ---------- Implementation Functions ---------- */
void unpack(header* data);
void pack(header* data);

#endif