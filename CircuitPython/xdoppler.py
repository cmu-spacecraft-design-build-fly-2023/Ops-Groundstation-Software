
from raspi_lora import LoRa, ModemConfig
import time

# This is our callback function that runs when a message is received
def on_recv(payload):
    print("From:", payload.header_from)
    print("Received:", payload.message)
    print("RSSI: {}; SNR: {}".format(payload.rssi, payload.snr))

# Use chip select 0. GPIO pin 17 will be used for interrupts
# The address of this device will be set to 2

center_freq = 915
step = 1
n_above = 10
freqs = range(
    center_freq - n_above * step, 
    center_freq + (n_above + 1) * step, step)
lora = LoRa(
        0, 19, 2, 
        modem_config=ModemConfig.Bw125Cr45Sf128, 
        tx_power=3, 
        acks=True)
for freq in freqs:
    # Send a message to a recipient device with address 10
    # Retry sending the message twice if we don't get an  acknowledgment from the recipient
    lora.freq = freq
    lora.on_recv = on_recv
    lora.set_mode_tx()
    message = "Tx " + str(freq) + "MHz Rx" + str(center_freq) + "MHz"
    status = lora.send(message, 10)
    if status is True:
        print("Sent: " + message)
    else:
        print("No ack")
    time.sleep(0.5)
lora.close()
    

