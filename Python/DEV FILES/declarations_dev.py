from enum import Enum
from message_database_dev import *
from gs_class_dev import *
import time

# Globals
received_success = False

class IMAGE_DEFS(Enum):
    IMAGE_1 = 1
    IMAGE_2 = 2
    IMAGE_3 = 3

class GROUNDSTATION:
    '''
        Name: __init__
        Description: Initialization of GROUNDSTATION class
    '''
    def __init__(self):
        # New contact from the satellite
        # Changes to True when heartbeat is received, false when image transfer starts
        self.new_session = False
        # Track the number of commands sent before an image is requested
        self.num_commands_sent = 0
        # List of commands to send before image request
        self.cmd_queue = [SAT_BATT_INFO, SAT_GPS_INFO, SAT_IMU_INFO]
        self.cmd_queue_size = len(self.cmd_queue)
        # Commands issued by the groundstation
        self.gs_cmd = 0x0
        # Sequence counter for images
        self.sequence_counter = 0
        # References to the image we are requesting
        self.image_num = IMAGE_DEFS.IMAGE_1.value
        self.target_image_CMD_ID = 0
        self.target_image_UID = 0
        self.target_sequence_count = 0
        # Image Info class
        self.sat_images = IMAGES()

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
        if self.message_ID == SAT_HEARTBEAT:
            print("Heartbeat received!")
            self.num_commands_sent = 0
            self.new_session = True
        else:
            print("Telemetry or image received!")

    '''
        Name: transmit_message
        Description: Ground station transmits a message via the LoRa module when the function is called.
        Inputs:
            lora - Declaration of lora class
    '''
    def transmit_message(self,lora):
        # Set radio to TX mode
        lora.set_mode_tx()
        time.sleep(0.1)

        if self.num_commands_sent < self.cmd_queue_size:
            lora_tx_message = self.pack_telemetry_command()
        else:
            lora_tx_message = self.pack_image_command()

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

        self.last_gs_cmd = self.gs_cmd

    '''
        Name: pack_telemetry_command
        Description: Packs the next telemetry command to be transmitted
    '''
    def pack_telemetry_command(self):
        print("Sending command: ",self.cmd_queue[self.num_commands_sent])
        self.gs_cmd = self.cmd_queue[self.num_commands_sent]
        self.num_commands_sent += 1

        # Payload to transmit
        # Simulated for now!
        lora_tx_header = [GS_ACK, 0x00, 0x01, 0x4]
        lora_tx_payload = [self.message_ID, self.gs_cmd, 0x0]
        lora_tx_message = lora_tx_header + lora_tx_payload

        return lora_tx_message

    '''
        Name: pack_image_command
        Description: Packs the next image command to be transmitted
    '''
    def pack_image_command(self):
        # Payload to transmit
        # Simulated for now!

        if ((self.gs_cmd == SAT_DEL_IMG1) or (self.gs_cmd == SAT_DEL_IMG2) or (self.gs_cmd == SAT_DEL_IMG3) or (self.new_session == True)):
            self.gs_cmd = SAT_IMAGES
            lora_tx_header = [GS_ACK, 0x00, 0x01, 0x4]
            lora_tx_payload = [self.message_ID, self.gs_cmd, 0x0]
            lora_tx_message = lora_tx_header + lora_tx_payload
            # Session is no longer "new" after telemetry has been retrieved
            self.new_session = False
        elif ((self.sequence_counter >= self.target_sequence_count) and (self.target_sequence_count != 0)):
            self.gs_cmd = SAT_DEL_IMG1
            lora_tx_header = [GS_ACK, 0x00, 0x01, 0x4]
            lora_tx_payload = [self.message_ID, self.gs_cmd, 0x0]
            lora_tx_message = lora_tx_header + lora_tx_payload
            # Reset sequence counter and get new image
            self.sequence_counter = 0
            if (self.image_num < IMAGE_DEFS.IMAGE_3.value):
                self.image_num += 1
            else:
                self.image_num = IMAGE_DEFS.IMAGE_1.value
        else:
            self.target_image_CMD_ID = self.sat_images.image_1_CMD_ID
            self.target_image_UID = self.sat_images.image_1_UID
            self.target_sequence_count = self.sat_images.image_1_message_count       
            self.gs_cmd = 0x50
            lora_tx_header = [GS_ACK, 0x00, 0x01, 0x4]
            lora_tx_payload = [self.message_ID, self.gs_cmd, self.sequence_counter]
            lora_tx_message = lora_tx_header + lora_tx_payload
            # Increment sequence counter
            self.sequence_counter += 1

        return lora_tx_message

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