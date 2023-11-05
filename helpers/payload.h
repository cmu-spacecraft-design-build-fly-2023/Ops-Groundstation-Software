#include <stdint.h>
#include "header.h"

#ifndef PAYLOAD
#define PAYLOAD

/* ---------- Signal Struct ---------- */
typedef struct signal_information {
    char signal_name[128];
    uint16_t start_bit, end_bit, factor, offset;
} msg_signal;

/* ---------- Message Struct ---------- */
typedef struct payload_message {
    char message_name[128];
    uint8_t* message_ptr;
} message;

/* ---------- Payload Struct ---------- */
typedef struct complete_payload {
    header header_payload;
    message message_payload;
} space_payload;

enum byte_alignment { zero_aligned = 0, byte_aligned = 8};

/* Function Prototypes */
uint8_t extractBits(uint32_t value, uint16_t startBit, uint16_t numBits);
void pack_payload(uint8_t array[], uint32_t sig, msg_signal send_sig);
uint32_t unpack_payload(uint8_t array[], msg_signal rec_sig);

#endif