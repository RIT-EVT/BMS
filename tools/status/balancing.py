"""
Script to interact with the balancing functionality of the BMS. Has the ability
to get the current state of balancing and set balancing on sepcific cells.
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
                           cell index to check/set balancing of with 1 indexing
                           ''')

    subparsers = argparser.add_subparsers(dest='cmd', required=True)

    poll_parser = subparsers.add_parser('poll', help='''Poll the state of
                                        balancing of a cell''')

    set_parser = subparsers.add_parser('set', help='''Set the state of
                                        balancing of a cell''')
    set_parser.add_argument('state', action='store', type=int, help='''0
                            for disabling balancing, 1 for balancing''')

    args = argparser.parse_args()

    network = canopen.Network()
    network.connect(channel=args.port, bustype='slcan')

    node = network.add_node(args.node, None)
    client = node.sdo


    if args.cmd == 'poll':
        state = struct.unpack('<B', client.upload(0x2103, args.cell))[0]
        print('Cell {} is balancing: {}'.format(args.cell, state))
    else:
        client.download(0x2103, args.cell, struct.pack('<B', args.state))
        print('Cell {} balancin set to: {}'.format(args.cell, args.state))

    network.disconnect()


if __name__ == '__main__':
    main()
