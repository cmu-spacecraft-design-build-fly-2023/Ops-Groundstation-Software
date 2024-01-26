from raspi_lora import LoRa, ModemConfig
import message_database
import time
import signal

# This is our callback function that runs when a message is received
def on_recv(payload):
    print(payload.message.decode("utf-8")) 
    print("From:", payload.header_from)
    print("Received image")
    print("RSSI: {}; SNR: {}".format(payload.rssi, payload.snr))
    global received_success 
    received_success = True

# Use chip select 0. GPIO pin 17 will be used for interrupts
# The address of this device will be set to 2
lora = LoRa(0, 19, 10, modem_config=ModemConfig.Bw125Cr45Sf128, acks=False)
lora.on_recv = on_recv

# LoRa header
lora_tx_message = [GS_ACK, 0x00, 0x01, 0x04]

# Generate LoRa payload for dummy GS acknowledgement
ack_payload = [0x01, 0x01, 0x00, 0x01]
lora_tx_message += ack_payload

signal.signal(signal.SIGINT, lora.close)

received_success = False

while True:
    received_success = False
    lora.set_mode_rx()

    while received_success == False:
        time.sleep(1)

    lora.set_mode_tx()
    time.sleep(1)

    lora.send(lora_tx_message, 2)

lora.close()