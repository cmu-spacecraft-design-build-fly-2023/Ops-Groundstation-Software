from enum import Enum
import time

# Globals
received_success = False

class SAT_STATES(Enum):
    IDLE = 1
    RECEIVING = 2
    TRANSMITTING = 3

'''
    Name: on_recv
    Description: Callback function that runs when a message is received.
    Inputs: 
        payload - message that was received
'''
def on_recv(payload):
    print(payload.message) 
    # print("From:", payload.header_from)
    # print("Received:", payload.message)
    # print("RSSI: {}; SNR: {}".format(payload.rssi, payload.snr))
    print('')
    global received_success 
    received_success = True

'''
    Name: transmit_message
    Description: Transmits a message via the LoRa module when the function is called.
    Inputs:
        lora_tx_message - message to be transmitted
'''
def transmit_message(lora, lora_tx_message):
    # Set radio to TX mode
    lora.set_mode_tx()

    # Send a message to a recipient device with address 10
    # Retry sending the message twice if we don't get an acknowledgment from the recipient
    status = lora.send(lora_tx_message, 10)

    # Check for groundstation acknowledgement 
    if status is True:
        print("Ground station sent message: [", *[hex(num) for num in lora_tx_message], "]")
        print("\n")
    else:
        print("No acknowledgment from recipient")
        print("\n")

'''
    Name: received_message
    Description: This function waits for a message to be received from the LoRa module
'''
def receive_message(lora):
    global received_success 
    received_success = False
    lora.set_mode_rx()

    while received_success == False:
        time.sleep(0.1)

    time.sleep(0.1)