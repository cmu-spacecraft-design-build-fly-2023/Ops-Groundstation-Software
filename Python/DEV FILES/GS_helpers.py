from enum import Enum
from protocol_database import *
import time
import sys
import os
import datetime
import boto3
import RPi.GPIO as GPIO

AWS_S3_BUCKET_NAME = 'spacecraft-files'
AWS_REGION = 'us-east-2'
AWS_ACCESS_KEY = 'AKIA2UC3FSEOAUZUHPPP'
AWS_SECRET_KEY = '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'
# AWS_ACCESS_KEY = 'ASK D.J.!'
# AWS_SECRET_KEY = 'ASK D.J.!'

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
        print('Setting up AWS')
        self.s3_client = boto3.client(
            service_name='s3',
            region_name=AWS_REGION,
            aws_access_key_id=AWS_ACCESS_KEY,
            aws_secret_access_key=AWS_SECRET_KEY
        )

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
        self.image_array = []
        # RX message info
        self.rx_message_ID = 0x0
        self.rx_message_sequence_count = 0
        self.rx_message_size = 0

        # Setup groundstation GPIO
        self.rx_ctrl = 22
        self.tx_ctrl = 23

        # Set up the GPIO pin as an output pin
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.rx_ctrl, GPIO.OUT)
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.tx_ctrl, GPIO.OUT)

    '''
        Name: received_message
        Description: This function waits for a message to be received from the LoRa module
        Inputs:
            lora - Declaration of lora class
    '''
    def receive_message(self,lora):
        # Pull GS RX pin HIGH!
        GPIO.output(self.rx_ctrl, GPIO.HIGH)
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
        # Turn GS RX pin LOW!
        GPIO.output(self.rx_ctrl, GPIO.LOW)

    '''
        Name: unpack_message
        Description: This function unpacks a message based on its ID
        Inputs:
            lora - Declaration of lora class
    '''
    def unpack_message(self,lora):
        # Unpack header information - Received header, sequence count, and message size
        self.rx_message_ID, self.rx_message_sequence_count, self.rx_message_size = gs_unpack_header(lora)

        if self.rx_message_ID == SAT_HEARTBEAT:
            print("Heartbeat received!")
            self.num_commands_sent = 0
            self.new_session = True
            # If last command was an image, refetch last portion of image 
            # to make sure it was received correctly.
            if ((self.gs_cmd == self.sat_images.image_1_CMD_ID) or (self.gs_cmd == self.sat_images.image_2_CMD_ID) or (self.gs_cmd == self.sat_images.image_3_CMD_ID)):
                if (self.sequence_counter > 0):
                    self.sequence_counter -= 1
                    self.image_array.pop(self.sequence_counter)
        elif self.rx_message_ID == SAT_IMAGES:
            self.image_info_unpack(lora)
        elif ((self.rx_message_ID == SAT_IMG1_CMD) or (self.rx_message_ID == SAT_IMG2_CMD) or (self.rx_message_ID == SAT_IMG3_CMD)):
            self.image_unpack(lora)
        else:
            print("Telemetry or image received!")
            print("Lora header_to:",lora._last_payload.header_to)
            print("Lora header_from:",lora._last_payload.header_from)
            print("Lora header_id:",lora._last_payload.header_id)
            print("Lora header_flags:",lora._last_payload.header_flags)
            print("Message received header:",list(lora._last_payload.message[0:4]))
    
    '''
        Name: image_info_unpack
        Description: This function unpacks the image information from
        satellite such as command IDs, UIDs, size (KB), and message counts
        Inputs
            lora - lora - Declaration of lora class
    '''
    def image_info_unpack(self,lora):
        self.sat_images = image_meta_info(lora)
        self.image_verification()

        # Diagnostic prints
        print("Image info received!")
        print("Message received header:",list(lora._last_payload.message[0:4]))

        print("Image 1 CMD ID:",self.sat_images.image_1_CMD_ID)
        print("Image 1 UID:",self.sat_images.image_1_UID)
        print("Image 1 size:",self.sat_images.image_1_size,"KB")
        print("Image 1 message count:",self.sat_images.image_1_message_count)

        print("Image 2 CMD ID:",self.sat_images.image_2_CMD_ID)
        print("Image 2 UID:",self.sat_images.image_2_UID)
        print("Image 2 size:",self.sat_images.image_2_size,"KB")
        print("Image 2 message count:",self.sat_images.image_2_message_count)

        print("Image 3 CMD ID:",self.sat_images.image_3_CMD_ID)
        print("Image 3 UID:",self.sat_images.image_3_UID)
        print("Image 3 size:",self.sat_images.image_3_size,"KB")
        print("Image 3 message count:",self.sat_images.image_3_message_count)

    '''
        Name: image_verification
        Description: Ensures the satellite did not delete any images since the last pass.
                     Will reset the sequence counter if new image was loaded.
    '''
    def image_verification(self):
        if ((self.image_num == IMAGE_DEFS.IMAGE_2.value) and (self.target_image_UID != self.sat_images.image_2_UID)):
            self.sequence_counter = 0
            self.image_array.clear()
        elif ((self.image_num == IMAGE_DEFS.IMAGE_3.value) and (self.target_image_UID != self.sat_images.image_3_UID)):
            self.sequence_counter = 0
            self.image_array.clear()
        elif ((self.image_num == IMAGE_DEFS.IMAGE_1.value) and (self.target_image_UID != self.sat_images.image_1_UID)):
            self.sequence_counter = 0
            self.image_array.clear()

    '''
        Name: image_unpack
        Description: This function unpacks the contents of the image and stores
        the image in a file when the complete image has been received.
        Inputs
            lora - Declaration of lora class
    '''
    def image_unpack(self,lora):
        self.image_array.append(lora._last_payload.message[4:self.rx_message_size + 4])
        # Increment sequence counter
        self.sequence_counter += 1
        if self.rx_message_sequence_count == (self.target_sequence_count - 1):
            # Get the current time
            current_time = datetime.datetime.now()

            # Format the current time
            formatted_time = current_time.strftime("%Y-%m-%d_%H-%M-%S")

            # Create image name
            filename = f"earth_image_{formatted_time}.jpg"

            rec_bytes = open(filename,'wb')
            
            for i in range(self.target_sequence_count):
                rec_bytes.write(self.image_array[i])         

            rec_bytes.close()

            response = self.s3_client.upload_file(filename, AWS_S3_BUCKET_NAME, filename)
            print(f'upload_log_to_aws response: {response}')
            time.sleep(1)
            self.image_array.clear()
            os.remove(filename)

    '''
        Name: transmit_message
        Description: Ground station transmits a message via the LoRa module when the function is called.
        Inputs:
            lora - Declaration of lora class
    '''
    def transmit_message(self,lora):
        # Pull GS TX pin HIGH!
        GPIO.output(self.tx_ctrl, GPIO.HIGH)
        time.sleep(0.15)

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

        while not lora.wait_packet_sent():
            pass

        self.last_gs_cmd = self.gs_cmd

        # Set GS TX pin LOW!
        GPIO.output(self.tx_ctrl, GPIO.LOW)

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
        lora_tx_header = bytes([GS_ACK, 0x00, 0x01, 0x4])
        lora_tx_payload = (self.rx_message_ID.to_bytes(1,'big') + self.gs_cmd.to_bytes(1,'big') + (0x0).to_bytes(2,'big'))
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
            lora_tx_header = bytes([GS_ACK, 0x00, 0x01, 0x4])
            lora_tx_payload = (self.rx_message_ID.to_bytes(1,'big') + self.gs_cmd.to_bytes(1,'big') + (0x0).to_bytes(2,'big'))
            lora_tx_message = lora_tx_header + lora_tx_payload
            # Session is no longer "new" after telemetry has been retrieved
            self.new_session = False
        elif ((self.sequence_counter >= self.target_sequence_count) and (self.target_sequence_count != 0)):
            self.gs_cmd = SAT_DEL_IMG1
            lora_tx_header = bytes([GS_ACK, 0x00, 0x01, 0x4])
            lora_tx_payload = (self.rx_message_ID.to_bytes(1,'big') + self.gs_cmd.to_bytes(1,'big') + (0x0).to_bytes(2,'big'))
            lora_tx_message = lora_tx_header + lora_tx_payload
            # Reset sequence counter and get new image
            self.sequence_counter = 0
            if (self.image_num < IMAGE_DEFS.IMAGE_3.value):
                self.image_num += 1
            else:
                self.image_num = IMAGE_DEFS.IMAGE_1.value
        else:
            self.pick_image()
            lora_tx_header = bytes([GS_ACK, 0x00, 0x00, 0x4])
            lora_tx_payload = (self.rx_message_ID.to_bytes(1,'big') + self.gs_cmd.to_bytes(1,'big') + self.sequence_counter.to_bytes(2,'big'))
            lora_tx_message = lora_tx_header + lora_tx_payload

        return lora_tx_message

    '''
        Name: pick_image
        Description: Picks what image to send based on the current image number.
    '''
    def pick_image(self):
        # Fetch image #2
        if self.image_num == IMAGE_DEFS.IMAGE_2.value:
            self.target_image_CMD_ID = self.sat_images.image_2_CMD_ID
            self.target_image_UID = self.sat_images.image_2_UID
            self.target_sequence_count = self.sat_images.image_2_message_count       
            self.gs_cmd = self.sat_images.image_2_CMD_ID
        # Fetch image #3
        elif self.image_num == IMAGE_DEFS.IMAGE_3.value:
            self.target_image_CMD_ID = self.sat_images.image_3_CMD_ID
            self.target_image_UID = self.sat_images.image_3_UID
            self.target_sequence_count = self.sat_images.image_3_message_count       
            self.gs_cmd = self.sat_images.image_3_CMD_ID
        # Fetch image #1
        else:
            self.target_image_CMD_ID = self.sat_images.image_1_CMD_ID
            self.target_image_UID = self.sat_images.image_1_UID
            self.target_sequence_count = self.sat_images.image_1_message_count       
            self.gs_cmd = self.sat_images.image_1_CMD_ID

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