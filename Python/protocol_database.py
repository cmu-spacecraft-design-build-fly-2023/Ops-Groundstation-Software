"""
'protocol_database.py'
======================
Python package containing protocol constants (IDs etc.). 
Also contains functions for constructing/deconstructing 
protocol messages. 

Each message has the following header: 
MESSAGE_ID : 1 byte 
SEQ_COUNT  : 2 bytes
LENGTH     : 1 byte  

Authors: Akshat Sahay, DJ Morvay
"""

# message ID definitions 
SAT_HEARTBEAT = 0x01

GS_ACK  = 0x08
SAT_ACK = 0x09

SAT_BATT_INFO = 0x10
SAT_GPS_INFO  = 0x11
SAT_IMU_INFO  = 0x12
SAT_SUN_INFO  = 0x13

SAT_IMAGES   = 0x21
SAT_DEL_IMG1 = 0x22
SAT_DEL_IMG2 = 0x23
SAT_DEL_IMG3 = 0x24

# function definitions 
def deconstruct_message(lora_rx_message):
    """
    :param lora_rx_message: Received LoRa message
    :return: None

    Deconstructs RX message based on message ID
    """
    # check RX message ID 
    if(lora_rx_message[0] == SAT_HEARTBEAT):
        # received satellite heartbeat, deconstruct header 
        print("GS: Received SAT heartbeat!")
        seq_count = (lora_rx_message[1] << 8) + lora_rx_message[2]
        print("GS: Sequence Count:", seq_count)
        print("GS: Message Length:", lora_rx_message[3])

        # deconstruct message contents 
        print("GS: Satellite system status:", lora_rx_message[4], lora_rx_message[5])
        print("GS: Satellite battery SOC: " + str(lora_rx_message[6]) + "%")
        print("GS: Satellite temperature: " + str(lora_rx_message[7]) + "*C")


def construct_message(lora_tx_message_ID):
    """
    :param lora_tx_message_ID: LoRa message ID
    :return: lora_tx_message

    Constructs TX message based on message ID
    """
    # LoRa header
    lora_tx_message = [0x00, 0x00, 0x00, 0x00] 

    if(lora_tx_message_ID == SAT_HEARTBEAT):
        # construct satellite heartbeat 
        lora_tx_message = [SAT_HEARTBEAT, 0x00, 0x01, 0x04] 

        # Generate LoRa payload for dummy heartbeat 
        # All systems ok, bits 0-12 are 1
        sat_system_status = [0x1F, 0xFF]
        lora_tx_message += sat_system_status

        # Battery at 80% charge 
        batt_soc = 80
        lora_tx_message.append(batt_soc)

        # Satellite temperature is 32 degrees C
        sat_temperature = 16
        lora_tx_message.append(sat_temperature)
    
    return lora_tx_message