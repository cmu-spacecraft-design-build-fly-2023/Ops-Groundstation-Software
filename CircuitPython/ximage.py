
from raspi_lora import LoRa, ModemConfig
import time

# This is our callback function that runs when a message is received
def on_recv(payload):
    print("From:", payload.header_from)
    print("Received")
    print("RSSI: {}; SNR: {}".format(payload.rssi, payload.snr))

# Use chip select 0. GPIO pin 17 will be used for interrupts
# The address of this device will be set to 2

lora = LoRa(
        0, 19, 2, 
        modem_config=ModemConfig.Bw125Cr45Sf128, 
        tx_power=3, 
        acks=True)
# Send a message to a recipient device with address 10
# Retry sending the message twice if we don't get an  acknowledgment from the recipient
lora.on_recv = on_recv
lora.set_mode_tx()
with open("tinyimage.jpg", mode="rb") as file:
    data = file.read()

ranges = list(range(0, len(data), 128))
if ranges[-1] != len(data):
    ranges += [len(data)]
time.sleep(1)
for i in range(len(ranges) - 1):
    message = data[ranges[i]: ranges[i+1]]
    status = lora.send(message, 10)
    if status is True:
        print("Sent image at bit " + str(i))
    else:
        print("No ack")
    time.sleep(1)

lora.close()
    

