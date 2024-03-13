from raspi_lora import LoRa, ModemConfig
from protocol_database import *
from GS_helpers import *
import time
import signal
import sys

GS = GROUNDSTATION()
tx_message = b'IL\x00\x00\x05\x00\x00\x00\x808mI\xf3\x11R\x00\n@\t\x00\x00\x00\xb1\x00R\x01\xa4\x00\x01\t\x00Q\xfe\xb5\xfe\xdd\xff\x02\x00\x1f\x00\x18\xc6\x00x\x00H\x01\x98\x04\x13\x0e\xd8\x00\x00\x00\x00\x00\x00I'

## ---------- MAIN CODE STARTS HERE! ---------- ##
# LoRa module setup
# Use chip select 0. GPIO pin 19 will be used for interrupts
# The address of this device will be set to 10
lora = LoRa(0, 19, tx_message[1], modem_config=ModemConfig.Bw125Cr45Sf128, acks=False, freq=433.0)
lora.on_recv = on_recv

# Setup interrupt
signal.signal(signal.SIGINT, lambda signum, frame: hard_exit(lora, signum, frame))

while True:
    status = lora.send(tx_message[4:], tx_message[0], header_id=tx_message[2], header_flags=tx_message[3])

    # Check for groundstation acknowledgement 
    if status is True:
        print("Satellite sent message")
        print("\n")
    else:
        print("No acknowledgment from recipient")
        print("\n")

    while not lora.wait_packet_sent():
        pass
    time.sleep(5)

lora.close()