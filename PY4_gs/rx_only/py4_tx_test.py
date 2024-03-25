#!/usr/bin/python3

import time,busio,board
import RPi.GPIO as GPIO
import spidev
import pycubed_rfm9x
import time

# radio1 - STOCK ADAFRUIT BONNET https://www.adafruit.com/product/4074
GPIO.setmode(GPIO.BCM)
CS1    = GPIO.setup(8, GPIO.OUT)
RESET1 = GPIO.setup(12, GPIO.OUT)
IRQ1   = GPIO.setup(19, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

cfg = {
    'r1b':(1,7,62500,1),  # default radio1 beacon config (CRC,SF,BW,LDRO) symb=2ms
}

# dummy 60-byte beacon packet to send
dummy_packet = b'IL\x00\x00\x05\x00\x00\x00\x808mI\xf3\x11R\x00\n@\t\x00\x00\x00\xb1\x00R\x01\xa4\x00\x01\t\x00Q\xfe\xb5\xfe\xdd\xff\x02\x00\x1f\x00\x18\xc6\x00x\x00H\x01\x98\x04\x13\x0e\xd8\x00\x00\x00\x00\x00\x00I'

spi = None
radio1=pycubed_rfm9x.RFM9x(spi, CS1, RESET1, 915.6, code_rate=8, baudrate=5_000_000)
radio1.tx_power = 5
radio1.dio0 = IRQ1
radio1.node = dummy_packet[1]
radio1.ack_delay= 0.2
radio1.ack_wait = 2
radio1.set_params(cfg['r1b']) # default CRC True, SF7, BW62500
radio1._write_u8(0x11,0b00110111) # IRQ RxTimeout,RxDone,TxDone
radio1.listen()

i = 0
while (1):
    # set the node id to the one used in the packet
    # set the destination byte and transmit the packet
    radio1.send(dummy_packet[4:],
        # set the 4-byte RadioHead header from the first four bytes of the dummy packet
        destination=dummy_packet[0],node=dummy_packet[1],identifier=dummy_packet[2],flags=dummy_packet[3],
        keep_listening=True)

    print(f'Sent test message #{i}!')
    i = i + 1
    time.sleep(0.5)
