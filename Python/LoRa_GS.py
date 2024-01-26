from raspi_lora import LoRa, ModemConfig
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

signal.signal(signal.SIGINT, lora.close)

received_success = False

print('I am running...')

while True:
    received_success = False
    lora.set_mode_rx()

    while received_success == False:
        time.sleep(1)

    lora.set_mode_tx()
    time.sleep(1)
    
    ack = "GS is sending acknowledgement!"
    status = lora.send(ack, 2)

    if status is True:
        print(ack)
    else:
        print("No acknowledgment from recipient")

lora.close()