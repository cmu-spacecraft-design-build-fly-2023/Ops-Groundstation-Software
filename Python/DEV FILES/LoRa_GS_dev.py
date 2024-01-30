from raspi_lora import LoRa, ModemConfig
from message_database_dev import *
from declarations_dev import *
import time
import signal

GS = GROUNDSTATION()

# Use chip select 0. GPIO pin 17 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 10, modem_config=ModemConfig.Bw125Cr45Sf128, acks=False)
lora.on_recv = on_recv

signal.signal(signal.SIGINT, lora.close)

while True:
    GS.receive_message(lora)
    GS.transmit_message(lora)

lora.close()