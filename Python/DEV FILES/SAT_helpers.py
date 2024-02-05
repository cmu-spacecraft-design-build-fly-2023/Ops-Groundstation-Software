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
        self.pack_three_images()
        self.heartbeat_sent = False
    
    '''
        Name: pack_three_images
        Description: This function takes in three images and stores them in a buffer
    '''
    def pack_three_images(self):
        # Initialize image arrays
        self.image1_array = []
        self.image2_array = []
        self.image3_array = []
        # Setup image class
        self.sat_images = IMAGES()
        # Setup initial image UIDs
        self.sat_images.image_1_UID = 0x5
        self.sat_images.image_2_UID = 0x2
        self.sat_images.image_3_UID = 0x3

        ## ---------- Image Sizes and Message Counts ---------- ## 
        # Get image #1 size and message count
        self.sat_images.image_1_size = int(os.path.getsize('IMAGES/nyc_small.jpg'))
        self.sat_images.image_1_message_count = int(self.sat_images.image_1_size / 128)

        if ((self.sat_images.image_1_size % 128) > 0):
            self.sat_images.image_1_message_count += 1    

        print("Image #1 size is", self.sat_images.image_1_size,"bytes")
        print("This image #1 requires",self.sat_images.image_1_message_count,"messages")

        # Get image #2 size and message count
        self.sat_images.image_2_size = int(os.path.getsize('IMAGES/tokyo_small.jpg'))
        self.sat_images.image_2_message_count = int(self.sat_images.image_2_size / 128)

        if ((self.sat_images.image_2_size % 128) > 0):
            self.sat_images.image_2_message_count += 1    

        print("Image #2 size is", self.sat_images.image_2_size,"bytes")
        print("This image #2 requires",self.sat_images.image_2_message_count,"messages")

         # Get image #3 size and message count
        self.sat_images.image_3_size = int(os.path.getsize('IMAGES/oregon_small.jpg'))
        self.sat_images.image_3_message_count = int(self.sat_images.image_3_size / 128)

        if ((self.sat_images.image_3_size % 128) > 0):
            self.sat_images.image_3_message_count += 1    

        print("Image #3 size is", self.sat_images.image_3_size,"bytes")
        print("This image #3 requires",self.sat_images.image_3_message_count,"messages")

        ## ---------- Image Buffer Storage ---------- ##
        # Image #1 Buffer Store
        bytes_remaining = self.sat_images.image_1_size
        send_bytes = open('IMAGES/nyc_small.jpg','rb')
        # Loop through image and store contents in an array
        while (bytes_remaining > 0):
            if (bytes_remaining >= 128):
                self.image1_array.append(send_bytes.read(128))
            else:
                self.image1_array.append(send_bytes.read(bytes_remaining))
                
            bytes_remaining -= 128
        # Close file when complete
        send_bytes.close()

        # Image #2 Buffer Store
        bytes_remaining = self.sat_images.image_2_size
        send_bytes = open('IMAGES/tokyo_small.jpg','rb')
        # Loop through image and store contents in an array
        while (bytes_remaining > 0):
            if (bytes_remaining >= 128):
                self.image2_array.append(send_bytes.read(128))
            else:
                self.image2_array.append(send_bytes.read(bytes_remaining))
                
            bytes_remaining -= 128
        # Close file when complete
        send_bytes.close()

        # Image #3 Buffer Store
        bytes_remaining = self.sat_images.image_3_size
        send_bytes = open('IMAGES/oregon_small.jpg','rb')
        # Loop through image and store contents in an array
        while (bytes_remaining > 0):
            if (bytes_remaining >= 128):
                self.image3_array.append(send_bytes.read(128))
            else:
                self.image3_array.append(send_bytes.read(bytes_remaining))
                
            bytes_remaining -= 128
        # Close file when complete
        send_bytes.close()

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
        time.sleep(0.15)

        if not self.heartbeat_sent:
            tx_header = bytes([0x1,0x0,0x0,0xF])
            tx_payload = bytes([0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0])
            tx_message = tx_header + tx_payload
            self.heartbeat_sent = True

        elif self.gs_req_message_ID == SAT_IMAGES:
            tx_header = bytes([SAT_IMAGES,0x0,0x0,0x18])
            tx_payload = self.pack_image_info()
            tx_message = tx_header + tx_payload

        elif (self.gs_req_message_ID == SAT_IMG1_CMD) or (self.gs_req_message_ID == SAT_IMG2_CMD) or (self.gs_req_message_ID == SAT_IMG3_CMD):

            # Decide which image to send
            if (self.gs_req_message_ID == SAT_IMG1_CMD):
                # Header
                tx_header = (self.gs_req_message_ID.to_bytes(1,'big') + self.gs_req_seq_count.to_bytes(2,'big') \
                            + len(self.image1_array[self.gs_req_seq_count]).to_bytes(1,'big'))
                # Payload
                tx_payload = self.image1_array[self.gs_req_seq_count]
            elif (self.gs_req_message_ID == SAT_IMG2_CMD):
                # Header
                tx_header = (self.gs_req_message_ID.to_bytes(1,'big') + self.gs_req_seq_count.to_bytes(2,'big') \
                            + len(self.image2_array[self.gs_req_seq_count]).to_bytes(1,'big'))
                # Payload
                tx_payload = self.image2_array[self.gs_req_seq_count]
            elif (self.gs_req_message_ID == SAT_IMG3_CMD):
                # Header
                tx_header = (self.gs_req_message_ID.to_bytes(1,'big') + self.gs_req_seq_count.to_bytes(2,'big') \
                            + len(self.image3_array[self.gs_req_seq_count]).to_bytes(1,'big'))
                # Payload
                tx_payload = self.image3_array[self.gs_req_seq_count]
            else:
                # Header
                tx_header = (self.gs_req_message_ID.to_bytes(1,'big') + self.gs_req_seq_count.to_bytes(2,'big') \
                            + len(self.image1_array[self.gs_req_seq_count]).to_bytes(1,'big'))
                # Payload
                tx_payload = self.image1_array[self.gs_req_seq_count]

            # Pack entire message
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
        Name: pack_image_info
        Description: This funtion packs the message ID, UID, size, and message count
                    for all three of the images loaded into the buffer.
    '''
    def pack_image_info(self):
        return (self.sat_images.image_1_CMD_ID.to_bytes(1,'big') + self.sat_images.image_1_UID.to_bytes(1,'big') + \
                self.sat_images.image_1_size.to_bytes(4,'big') + self.sat_images.image_1_message_count.to_bytes(2,'big') + \
                self.sat_images.image_2_CMD_ID.to_bytes(1,'big') + self.sat_images.image_2_UID.to_bytes(1,'big') + \
                self.sat_images.image_2_size.to_bytes(4,'big') + self.sat_images.image_2_message_count.to_bytes(2,'big') + \
                self.sat_images.image_3_CMD_ID.to_bytes(1,'big') + self.sat_images.image_3_UID.to_bytes(1,'big') + \
                self.sat_images.image_3_size.to_bytes(4,'big') + self.sat_images.image_3_message_count.to_bytes(2,'big'))

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