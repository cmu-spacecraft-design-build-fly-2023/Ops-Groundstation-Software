from raspi_lora import LoRa, ModemConfig
from message_database import *
from declarations import *
import time
import signal

## ---------- MAIN CODE STARTS HERE! ---------- ##
# LoRa module setup
# Use chip select 0. GPIO pin 19 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 2, modem_config=ModemConfig.Bw125Cr45Sf128, tx_power=3, acks=False)
lora.on_recv = on_recv

# Setup interrupt
signal.signal(signal.SIGINT, lora.close)

# LoRa header
lora_tx_message = [SAT_HEARTBEAT, 0x00, 0x01, 0x04]

# Generate LoRa payload for dummy heartbeat 
# All systems ok, bits 0-12 are 1
sat_system_status = [0x1F, 0xFF]
lora_tx_message += sat_system_status

# Battery at 80% charge 
batt_soc = 80
lora_tx_message.append(batt_soc)

# Satellite temperature is 32 degrees C
sat_temperature = 32
lora_tx_message.append(sat_temperature)

while True:
    sat_transmit_message(lora, lora_tx_message)
    receive_message(lora)

# And remember to call this as your program exits...
lora.close()
