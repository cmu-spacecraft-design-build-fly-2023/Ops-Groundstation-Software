from raspi_lora import LoRa, ModemConfig
from protocol_database import *
from declarations import *
import time
import signal

## ---------- MAIN CODE STARTS HERE! ---------- ##
# LoRa module setup
# Use chip select 0. GPIO pin 19 will be used for interrupts
# The address of this device will be set to 10
lora = LoRa(0, 19, 10, modem_config=ModemConfig.Bw125Cr45Sf128, acks=False)
lora.on_recv = on_recv

signal.signal(signal.SIGINT, lora.close)

# LoRa header
lora_tx_message = [GS_ACK, 0x00, 0x01, 0x04]
# Generate LoRa payload for dummy GS acknowledgement
ack_payload = [0x01, 0x01, 0x00, 0x01]
lora_tx_message += ack_payload
# Convert to bytes format 
lora_tx_message_bits = bytes(lora_tx_message)

print("GS: Receiving with address 10")

while True:
    received_success = receive_message(lora)

    if received_success == True:
        # Decode and print received LoRa packet 
        lora_rx_message = list(lora._last_payload.message)
        print("GS received:", lora_rx_message)
        deconstruct_message(lora_rx_message)
        print("")

        gs_transmit_message(lora, lora_tx_message_bits)

        # Reset received_success
        received_success = False
        time.sleep(1)

lora.close()