"""
'message_database.py'
======================
Python package containing messaging constants (IDs etc.)
Also contains packet structure definition

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

SAT_IMG1_CMD = 0x50

# TX message packet (empty header) 
# to add payload, generate payload list and add
