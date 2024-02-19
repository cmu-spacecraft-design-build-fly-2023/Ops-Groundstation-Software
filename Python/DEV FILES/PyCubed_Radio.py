from protocol_database import *
from PyCubed_Radio_helpers import *
from pycubed import cubesat

SAT = SATELLITE()

## ---------- MAIN CODE STARTS HERE! ---------- ##

while True:
    if cubesat.hardware['Radio1']:
        SAT.transmit_message()

    if cubesat.hardware['Radio1']:
        SAT.receive_message()
