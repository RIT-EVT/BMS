"""
Small utility which can be used to send out a simulated heartbeat out once.
The user provides the device that is being simulated using a CANopen node,
and the result is the associated heartbeat.

For example, for device node 3, this script will write out 0x703 over the
network
"""
import can
import time
from argparse import ArgumentParser

def main():
    argparser = ArgumentParser(description='''Utility to simulate a heartbeat''')
    argparser.add_argument('port', action='store', type=str, help='''The
                           port of the can device to interface with the
                           BMS''')
    argparser.add_argument('device', action='store', type=int, help='''The
                           CANopen node ID to simulate the heartbeat of''')
    args = argparser.parse_args()

    with can.interface.Bus(bustype='slcan', channel=args.port) as bus:
        msg = can.Message(arbitration_id=0x700 + args.device, is_extended_id=False)
        bus.send(msg)
    time.sleep(0.1)


if __name__ == '__main__':
    main()
