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
SAT_HEARTBEAT = const(0x1)

GS_ACK  = const(0x8)
SAT_ACK = const(0x9)

SAT_BATT_INFO = const(0x10)
SAT_GPS_INFO  = const(0x11)
SAT_IMU_INFO  = const(0x12)
SAT_SUN_INFO  = const(0x13)

SAT_IMAGES   = const(0x21)
SAT_DEL_IMG1 = const(0x22)
SAT_DEL_IMG2 = const(0x23)
SAT_DEL_IMG3 = const(0x24)

# TX message packet (empty header) 
# to add payload, generate payload list and add
