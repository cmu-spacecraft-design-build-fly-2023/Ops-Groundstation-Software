from raspi_lora import LoRa, ModemConfig
from protocol_database import *
import time
import signal

# This is our callback function that runs when a message is received
def on_recv(payload):
    print("From:", payload.header_from)
    print("Received:", payload.message)
    print("RSSI: {}; SNR: {}".format(payload.rssi, payload.snr))

    # convert message bytes back to list
    # lora_rx_message = (lora_rx_message_bits.decode('utf-8')).strip('][').split(', ')
    # lora_rx_message = [int(i) for i in lora_rx_message]

    global received_success 
    received_success = True

# Use chip select 0. GPIO pin 17 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 2, modem_config=ModemConfig.Bw125Cr45Sf128, tx_power=3, acks=False)
lora.on_recv = on_recv

# LoRa header
lora_tx_message = construct_message(SAT_HEARTBEAT)

# convert message to bytes for transmission 
# lora_tx_message_bits = str(lora_tx_message).encode('utf-8')

signal.signal(signal.SIGINT, lora.close)

received_success = False

while True:
    # Set radio to TX mode
    lora.set_mode_tx()

    # Send a message to a recipient device with address 10
    # Retry sending the message twice if we don't get an acknowledgment from the recipient
    status = lora.send(lora_tx_message, 10)

    # Check for groundstation acknowledgement 
    if status is True:
        print(lora_tx_message)
    else:
        print("No acknowledgment from recipient")

    time.sleep(0.5)

    received_success = False
    lora.set_mode_rx()

    while not received_success:
        time.sleep(0.1)

    time.sleep(1)

# And remember to call this as your program exits...
lora.close()
