from raspi_lora import LoRa, ModemConfig
from message_database import *
from declarations import *
import time
import signal

# Use chip select 0. GPIO pin 17 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 10, modem_config=ModemConfig.Bw125Cr45Sf128, acks=False)
lora.on_recv = on_recv

# LoRa header
lora_tx_message = [GS_ACK, 0x00, 0x01, 0x04]

# Generate LoRa payload for dummy GS acknowledgement
ack_payload = [0x01, 0x01, 0x00, 0x01]
lora_tx_message += ack_payload

signal.signal(signal.SIGINT, lora.close)

while True:
    receive_message(lora)
    gs_transmit_message(lora,lora_tx_message)

lora.close()