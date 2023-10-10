#include <stdint.h>

#ifndef PAYLOAD
#define PAYLOAD

/* ---------- Message Struct ---------- */
typedef struct payload_message {
    char message_name[128];
    uint16_t message_length;
} message;

/* ---------- Signal Struct ---------- */
typedef struct signal_information {
    char signal_name[128];
    uint16_t start_bit, end_bit, factor, offset;
} signal;

#endif