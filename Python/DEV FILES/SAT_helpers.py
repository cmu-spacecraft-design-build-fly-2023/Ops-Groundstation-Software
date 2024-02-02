from enum import Enum
import os
from message_database_dev import *
from image_class_dev import *
import time
import sys

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
        self.sat_images.image_1_size = int(os.path.getsize('IMAGES/nyc_small.jpg'))
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

        self.heartbeat_sent = False

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
        self.rx_message_ID = int.from_bytes(lora._last_payload.message[0:1],byteorder='big')
        self.rx_message_sequence_count = int.from_bytes(lora._last_payload.message[1:3],byteorder='big')
        self.rx_message_size = int.from_bytes(lora._last_payload.message[3:4],byteorder='big')
        print("Message received header:",list(lora._last_payload.message[0:4]))

        if (self.rx_message_ID == GS_ACK):
            self.gs_rx_message_ID = int.from_bytes(lora._last_payload.message[4:5],byteorder='big')
            self.gs_req_message_ID = int.from_bytes(lora._last_payload.message[5:6],byteorder='big')
            self.gs_req_seq_count = int.from_bytes(lora._last_payload.message[6:8],byteorder='big')
            
    '''
        Name: transmit_message
        Description: Ground station transmits a message via the LoRa module when the function is called.
        Inputs:
            lora - Declaration of lora class
    '''
    def transmit_message(self,lora):
        time.sleep(0.25)

        if not self.heartbeat_sent:
            tx_header = bytes([0x1,0x0,0x0,0xF])
            tx_payload = bytes([0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0])
            tx_message = tx_header + tx_payload
            self.heartbeat_sent = True
        elif self.gs_req_message_ID == SAT_IMAGES:
            tx_header = bytes([SAT_IMAGES,0x0,0x0,0x18])
            tx_payload = (self.sat_images.image_1_CMD_ID.to_bytes(1,'big') + self.sat_images.image_1_UID.to_bytes(1,'big') +
                          self.sat_images.image_1_size.to_bytes(4,'big') + self.sat_images.image_1_message_count.to_bytes(2,'big'))
            tx_message = tx_header + tx_payload
        else:
            tx_header = (self.gs_req_message_ID.to_bytes(1,'big') + (0x0).to_bytes(1,'big') + (0x0).to_bytes(1,'big') + (0x0).to_bytes(1,'big'))
            tx_message = tx_header
    
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

'''
    Name: hard_exit
    Description: Shutdown when ctrl-c is pressed
    Inputs: 
        lora - Declaration of lora class
'''
def hard_exit(lora, signum, frame):
    lora.close()
    sys.exit(0)