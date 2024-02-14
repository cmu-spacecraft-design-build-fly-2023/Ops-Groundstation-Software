from message_database_dev import *
from PyCubed_Radio_helpers import *
import signal
from pycubed import cubesat

SAT = SATELLITE()

## ---------- MAIN CODE STARTS HERE! ---------- ##
# Setup interrupt
signal.signal(signal.SIGINT, lambda signum, frame: hard_exit(signum, frame))

while True:
    if cubesat.hardware['Radio1']:
        SAT.transmit_message()

    if cubesat.hardware['Radio1']:
        SAT.receive_message()
