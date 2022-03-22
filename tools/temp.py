import can
import time

with can.interface.Bus(bustype='slcan', channel='/dev/cu.usbmodem143201') as bus:
    msg = can.Message(arbitration_id=0x716, is_extended_id=False)
    bus.send(msg)

time.sleep(1)
