from enum import Enum
import os
from message_database_dev import *
from image_class_dev import *
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
        # Setup image class
        self.sat_images = IMAGES()
        self.sat_images.image_1_CMD_ID = 0x50
        self.sat_images.image_1_UID = 0x1
        # Get image size and number of messages it requires
        self.sat_images.image_1_size = os.path.getsize('IMAGES/nyc_small.jpg')
        self.sat_images.image_1_message_count = int(self.sat_images.image_1_size / 128)

        if ((self.sat_images.image_1_size % 128) > 0):
            self.sat_images.image_1_message_count += 1    

        print("Image size is", self.sat_images.image_1_size,"bytes")
        print("This image requires",self.sat_images.image_1_message_count,"messages")

        bytes_remaining = self.sat_images.image_1_size
        send_bytes = open('IMAGES/nyc_small.jpg','rb')

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
        self.byte_counter = int(self.sat_images.image_1_size)

    '''
        Name: received_message
        Description: This function waits for a message to be received from the LoRa module
        Inputs:
            lora - Declaration of lora class
    '''
    def receive_message(self,lora):
        global received_success 
        received_success = False
        lora.set_mode_rx()

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
        print("Message received header:",list(lora._last_payload.message[0:4]))

    '''
        Name: transmit_message
        Description: Ground station transmits a message via the LoRa module when the function is called.
        Inputs:
            lora - Declaration of lora class
    '''
    def transmit_message(self,lora):
        time.sleep(0.25)

        if (self.sequence_counter < self.sat_images.image_1_message_count):
            seq_counter_bytes = self.sequence_counter.to_bytes(2,'big')
        
            if (self.byte_counter > 128):
                payload_size = bytes([128])
            else:
                payload_size = self.byte_counter.to_bytes(1,'big')

            tx_header = bytes([0x50,seq_counter_bytes[0],seq_counter_bytes[1],payload_size[0]])
            tx_payload = self.image_array[self.sequence_counter]
            tx_message = tx_header + tx_payload

            self.byte_counter -= 128
            self.sequence_counter += 1
        else:
            tx_header = bytes([0x1,0x0,0x0,0x0])
            tx_payload = bytes([0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7])
            tx_message = tx_header + tx_payload
    
        print("Message sent header:",list(tx_header))

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

        while not lora.wait_packet_sent():
            pass

'''
    Name: on_recv
    Description: Callback function that runs when a message is received.
    Inputs: 
        payload - message that was received
'''
def on_recv(payload):
    global received_success 
    received_success = True