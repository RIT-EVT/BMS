"""
Utility to reset the state of the BMS.
"""
import canopen
import struct
from argparse import ArgumentParser


def main():
    argparser = ArgumentParser(description='''Utility to reset the state of
                               the BMS''')
    argparser.add_argument('port', action='store', type=str, help='''The
                           port of the can device to interface with the
                           BMS''')
    argparser.add_argument('node', action='store', type=int, help='''The
                           CANopen node ID of the BMS''')
    argparser.add_argument('state', action='store', type=int, help='''The
                           target state of the BMS. See `BMS::State` in
                           `BMS.hpp` for the states and their corresponding
                           numbers''')
    args = argparser.parse_args()

    network = canopen.Network()
    network.connect(channel=args.port, bustype='slcan')

    node = network.add_node(args.node, None)
    client = node.sdo

    client.download(0x2102, 0x0, struct.pack('<I', args.state))

    network.disconnect()


if __name__ == '__main__':
    main()
