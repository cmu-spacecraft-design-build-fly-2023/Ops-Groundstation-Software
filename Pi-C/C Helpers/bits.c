#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <stdlib.h>
#include "../helpers/payload.h"
 
 /* 
int main() {
   // Array Pointer //
   uint8_t* ptr;

   // Message struct //
   message send_msg;
   send_msg.message_length = 4;

   // Signal struct //
   msg_signal send_sig1;
   send_sig1.start_bit = 0;
   send_sig1.end_bit = 7;
   uint8_t dummy1 = 0b11111010;

   msg_signal send_sig2;
   send_sig2.start_bit = 8;
   send_sig2.end_bit = 19;
   uint16_t dummy2 = 0b111111110000;

   msg_signal send_sig3;
   send_sig3.start_bit = 20;
   send_sig3.end_bit = 27;
   uint8_t dummy3 = 0b11001100;

   msg_signal send_sig4;
   send_sig4.start_bit = 28;
   send_sig4.end_bit = 31;
   uint8_t dummy4 = 0b0011;

   ptr = (uint8_t*)calloc(send_msg.message_length, sizeof(uint8_t));

   pack_payload(ptr,dummy1,send_sig1);
   pack_payload(ptr,dummy2,send_sig2);
   pack_payload(ptr,dummy3,send_sig3);
   pack_payload(ptr,dummy4,send_sig4);

   for (int i = 0; i < send_msg.message_length; i++) {
      printf("%u\n",ptr[i]);
   }

   uint8_t unpack1 = (uint8_t)unpack_payload(ptr, send_sig1);
   uint16_t unpack2 = (uint16_t)unpack_payload(ptr, send_sig2);
   uint8_t unpack3 = (uint8_t)unpack_payload(ptr, send_sig3);
   uint8_t unpack4 = (uint8_t)unpack_payload(ptr, send_sig4);

   printf("%u - %u - %u - %u\n",unpack1,unpack2,unpack3, unpack4);

   free(ptr);

   return 0;
} */

/**
 * @name: extractedBits
 * 
 * @details: Input a string of bits and this function will return a section of
 *           the string based on the specified start bit and the number of bits requested. 
 * 
 * Inputs
 *    @param value: String of bits
 *    @param startBit: Start bit where bits are being extracted
 *    @param numBits: Number of bits being extracted
 * 
 * @return Extracted bits for the payload 
*/
uint8_t extractBits(uint32_t value, uint16_t startBit, uint16_t numBits) {
    uint8_t mask = (1 << numBits) - 1;  // Create a mask with the specified number of bits
    return ((uint8_t)(value >> startBit)) & mask;       // Shift the value and apply the mask
}

/**
 * @name: pack_payload
 * 
 * @details: Takes in a signal of varying length and packs it in 
 *          the correct sequence in the payload array.
 * 
 * Inputs
 *    @param array: Payload array
 *    @param sig: Signal being packed into the payload array
 *    @param send_sig: Information about the signal being packed
 * 
 * Outputs
 *    @param array: Payload array being packed
 * 
 * @return NONE
*/
void pack_payload(uint8_t array[], uint32_t sig, msg_signal send_sig) {
   /* Amount of data that still needs packed */
   uint16_t pack_remaining = (send_sig.end_bit - send_sig.start_bit) + 1;
   /* Bit position of pack index */
   uint16_t pack_position = send_sig.start_bit;
   /* Bit position of var index */
   uint16_t var_position = 0;

   while (pack_remaining > 0) {
      uint16_t start_byte = pack_position/byte_aligned;
      uint16_t end_byte = send_sig.end_bit/byte_aligned;

      if (start_byte == end_byte) {
         // Extract untouched bits of array[i]
         // Extract bits of var that will be packed
         uint8_t lower_bits_array = extractBits(array[start_byte],0,pack_position % byte_aligned);
         uint8_t lower_bits_var = extractBits(sig,var_position,pack_remaining);

         // Get the shift distance (Amount of bits clearing out)
         uint8_t shift_amount = (send_sig.end_bit % byte_aligned) + 1;

         // Shift and clear out lower bits
         uint8_t tmp = array[start_byte] >> shift_amount;
         // Shift in enough space for var and add it in
         tmp = ((tmp << pack_remaining)+lower_bits_var) << (pack_position % byte_aligned);
         // Add back in lower static bits
         array[start_byte] = lower_bits_array + tmp;

         // Decrement the bits remaining
         // Increment the bits index
         pack_position += pack_remaining;
         var_position += pack_remaining;
         pack_remaining -= pack_remaining;
      } 
      else {
         // Get number of bits that will not be modified
         // and the number that will not be modified.
         uint8_t static_bits = pack_position % byte_aligned;
         uint8_t modified_bits = byte_aligned - static_bits;

         // Extract untouched bits of array[i]
         // Extract bits of var that will be packed
         uint8_t lower_bits_array = extractBits(array[start_byte],0,static_bits);
         uint8_t lower_bits_var = extractBits(sig, var_position, modified_bits);

         // Add in bits of var, shift, and add in static bits
         array[start_byte] = (lower_bits_var << static_bits) + lower_bits_array;

         // Decrement the bits remaining
         // Increment the bits index
         pack_remaining -= modified_bits;
         var_position += modified_bits;
         pack_position += modified_bits;
      }
   }
}

/**
 * @name: unpack_payload
 * 
 * @details: Inputs the payload array and outputs a signal based on the 
 *           start and end bit indices of the signal. 
 * 
 * Inputs
 *    @param array: Payload array
 *    @param rec_sig: Information about the signal being packed.w
 *                     Includes the start and end bit indices. 
 * 
 * Outputs
 *    NONE
 * @return NONE
*/
uint32_t unpack_payload(uint8_t array[], msg_signal rec_sig) {
   uint32_t fetched_sig = 0;
   /* Amount of data that still needs packed */
   uint16_t unpack_remaining = (rec_sig.end_bit - rec_sig.start_bit) + 1;
   /* Bit position of pack index */
   uint16_t pack_position = rec_sig.end_bit;

   while (unpack_remaining > 0) {
      uint16_t start_byte = pack_position/byte_aligned;
      uint16_t end_byte = rec_sig.start_bit/byte_aligned;

      if (start_byte == end_byte) {

         if (unpack_remaining < 8) {
            // Extract remaining bytes
            fetched_sig += extractBits(array[start_byte], ((pack_position % byte_aligned) - unpack_remaining + 1), unpack_remaining);

            // Decrement the bits remaining
            // Decrement the bits index
            pack_position -= unpack_remaining;
            unpack_remaining -= unpack_remaining;
         } 
         else {
            // Extract remaining bytes
            // Set bit zero as lower bound
            fetched_sig += extractBits(array[start_byte], 0, unpack_remaining);

            // Don't let pack position underflow out of precaution
            // Decrement the bits remaining
            pack_position = 0;
            unpack_remaining -= unpack_remaining;
         }
      } 
      else {
         // Get number of bits that will not be modified
         // and the number that will not be modified.
         uint8_t unpack_bits = (pack_position % byte_aligned) + 1;
         uint8_t fluff_bits = byte_aligned - unpack_bits;

         // Extract the packed bits of array[i]
         uint8_t lower_bits_array = extractBits(array[start_byte], 0, unpack_bits);

         // Decrement the bits remaining
         // Decrement the bits index
         unpack_remaining -= unpack_bits;
         pack_position -= unpack_bits;

         // Add in bits of var, shift, and add in static bits
         if (unpack_remaining >= 8) {
            fetched_sig = (fetched_sig + lower_bits_array) << byte_aligned;
         }
         else {
            fetched_sig = (fetched_sig + lower_bits_array) << unpack_remaining;
         }
      }
   }
   return fetched_sig;
}