"""
Script to reach a specific cell from the BMS
"""
import canopen
from argparse import ArgumentParser
import struct


def main():
    argparser = ArgumentParser(description='''Utility to get the current state
                               of the BMS''')
    argparser.add_argument('port', action='store', type=str, help='''The
                           port of the can device to interface with the
                           BMS''')
    argparser.add_argument('node', action='store', type=int, help='''The
                           CANopen node ID of the BMS''')
    argparser.add_argument('cell', action='store', type=int, help='''The
                           cell index to read the voltage of, 0 for total
                           voltage''')
    args = argparser.parse_args()

    network = canopen.Network()
    network.connect(channel=args.port, bustype='slcan')

    node = network.add_node(args.node, None)
    client = node.sdo

    if args.cell == 0:
        cell_voltage = struct.unpack('<I', client.upload(0x2101, args.cell))[0]
    else:
        cell_voltage = struct.unpack('<H', client.upload(0x2101, args.cell))[0]

    print('{}V'.format(cell_voltage / 1000.0))

    network.disconnect()


if __name__ == '__main__':
    main()
