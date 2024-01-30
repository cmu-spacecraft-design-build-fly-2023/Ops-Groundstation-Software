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
    Name: sat_transmit_message
    Description: Satellite transmits a message via the LoRa module when the function is called.
    Inputs:
        lora - Declaration of lora class
        lora_tx_message - message to be transmitted
'''
def sat_transmit_message(lora, lora_tx_message):
    # Set radio to TX mode
    lora.set_mode_tx()
    time.sleep(0.1)

    # Send a message to ground station device with address 10
    # Retry sending the message twice if we don't get an acknowledgment from the recipient
    status = lora.send(lora_tx_message, 10)

    # Check for groundstation acknowledgement 
    if status is True:
        print("Satellite sent message: [", *[hex(num) for num in lora_tx_message], "]")
        print("\n")
    else:
        print("No acknowledgment from recipient")
        print("\n")

'''
    Name: gs_transmit_message
    Description: Ground station transmits a message via the LoRa module when the function is called.
    Inputs:
        lora - Declaration of lora class
        lora_tx_message - message to be transmitted
'''
def gs_transmit_message(lora, lora_tx_message):
    # Set radio to TX mode
    lora.set_mode_tx()
    time.sleep(0.1)

    # Send a message to the satellite device with address 2
    # Retry sending the message twice if we don't get an acknowledgment from the recipient
    status = lora.send(lora_tx_message, 2)

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
    Inputs:
        lora - Declaration of lora class
'''
def receive_message(lora):
    global received_success 
    received_success = False
    lora.set_mode_rx()
    time.sleep(0.1)

    while received_success == False:
        time.sleep(0.1)