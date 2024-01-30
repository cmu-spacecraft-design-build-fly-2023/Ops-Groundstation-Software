from enum import Enum
import os
from message_database_dev import *
from gs_class_dev import *
import time

# Globals
received_success = False

class SATELLITE:
    '''
        Name: __init__
        Description: Initialization of GROUNDSTATION class
    '''
    def __init__(self):
        # Get image from memory
        self.image_array = []
        # Get image size and number of messages it requires
        self.image_size = os.path.getsize('tinyimage.jpg')
        self.image_message_count = int(self.image_size / 128)

        if ((self.image_size % 128) > 0):
            self.image_message_count += 1    

        print("Image size is",self.image_size,"bytes")
        print("This image requires",self.image_message_count,"messages")

        bytes_remaining = self.image_size
        send_bytes = open('tinyimage.jpg','rb')

        # Loop through image and store contents in an array
        while (bytes_remaining > 0):
            if (bytes_remaining >= 128):
                payload = send_bytes.read(128)
            else:
                payload = send_bytes.read(bytes_remaining)
                
            bytes_remaining -= 128

            self.image_array.append(payload)
        
        send_bytes.close()

        # Sequence counter for image transfer
        self.sequence_counter = int(0)
        self.byte_counter = int(self.image_size)

    '''
        Name: received_message
        Description: This function waits for a message to be received from the LoRa module
        Inputs:
            lora - Declaration of lora class
    '''
    def receive_message(self,lora):
        global received_success 
        received_success = False
        time.sleep(0.1)
        lora.set_mode_rx()
        time.sleep(0.1)

        while received_success == False:
            time.sleep(0.1)

        # print(lora._last_payload.message) 
        # print("From:", payload.header_from)
        # print("Received:", payload.message)
        # print("RSSI: {}; SNR: {}".format(payload.rssi, payload.snr))
        # print('')

        self.unpack_message(lora)

    '''
        Name: unpack_message
        Description: This function unpacks a message based on its ID
        Inputs:
            lora - Declaration of lora class
    '''
    def unpack_message(self,lora):
        self.message_ID = int.from_bytes(lora._last_payload.message[0:1],byteorder='little')

    '''
        Name: transmit_message
        Description: Ground station transmits a message via the LoRa module when the function is called.
        Inputs:
            lora - Declaration of lora class
    '''
    def transmit_message(self,lora):
        # Set radio to TX mode
        time.sleep(0.1)
        lora.set_mode_tx()
        time.sleep(0.1)

        if (self.sequence_counter < self.image_message_count):
            seq_counter_bytes = self.sequence_counter.to_bytes(2,'big')
        
            if (self.byte_counter > 128):
                payload_size = bytes(128)
            else:
                payload_size = self.byte_counter.to_bytes(1,'big')

            tx_header = [0x50,seq_counter_bytes[1:2],seq_counter_bytes[0:1],payload_size]
            tx_payload = self.image_array[self.byte_counter]
            tx_message = tx_header + tx_payload

            self.byte_counter -= 128
            self.sequence_counter += 1
        else:
            tx_header = [0x21,0x0,0x0,0x8]
            tx_payload = [0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7]
            tx_message = tx_header + tx_payload

        # Send a message to the satellite device with address 10
        # Retry sending the message twice if we don't get an acknowledgment from the recipient
        status = lora.send(tx_message, 10)

        # Check for groundstation acknowledgement 
        if status is True:
            print("Satellite sent message")
            print("\n")
        else:
            print("No acknowledgment from recipient")
            print("\n")

'''
    Name: on_recv
    Description: Callback function that runs when a message is received.
    Inputs: 
        payload - message that was received
'''
def on_recv(payload):
    global received_success 
    received_success = True