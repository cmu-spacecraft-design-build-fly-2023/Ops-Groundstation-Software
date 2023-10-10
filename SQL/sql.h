#include <stdint.h>

#ifndef SQL_DATA
#define SQL_DATA

/* ---------- Includes ---------- */
#include "../helpers/payload.h"
#include "../helpers/header.h"

/* ---------- Function Prototypes ---------- */
int get_payload_message(header cube_header, message* cube_msg);
int get_start_end(message cube_msg, signal* cube_signal);

#endif