from raspi_lora import LoRa, ModemConfig
from message_database_dev import *
from SAT_helpers import *
import time
import signal

SAT = SATELLITE()

## ---------- MAIN CODE STARTS HERE! ---------- ##
# LoRa module setup
# Use chip select 0. GPIO pin 19 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 2, modem_config=ModemConfig.Bw125Cr45Sf128, tx_power=3, acks=False)
lora.on_recv = on_recv

# Setup interrupt
signal.signal(signal.SIGINT, lora.close)

while True:
    SAT.transmit_message(lora)
    SAT.receive_message(lora)

# And remember to call this as your program exits...
lora.close()
