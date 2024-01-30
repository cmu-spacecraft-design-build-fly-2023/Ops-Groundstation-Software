from raspi_lora import LoRa, ModemConfig
from protocol_database import *
from declarations import *
import time
import signal

## ---------- MAIN CODE STARTS HERE! ---------- ##
# LoRa module setup
# Use chip select 0. GPIO pin 19 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 2, modem_config=ModemConfig.Bw125Cr45Sf128, tx_power=3, acks=False)
lora.on_recv = on_recv

# Setup interrupt
signal.signal(signal.SIGINT, lora.close)

# Construct dummy heartbeat to transmit 
lora_tx_message = construct_message(SAT_HEARTBEAT)
# Convert to bytes format 
lora_tx_message_bits = bytes(lora_tx_message)

received_success = False

print("SAT: Transmitting with address 2")

while True:
    sat_transmit_message(lora, lora_tx_message_bits)
    received_success = receive_message(lora)

    if received_success == True:
        # Decode and print received LoRa packet 
        lora_rx_message = list(lora._last_payload.message)
        print("SAT received:", lora_rx_message)
        deconstruct_message(lora_rx_message)
        print("")

        # Reset received_success
        received_success = False
        time.sleep(1)

# And remember to call this as your program exits...
lora.close()
