"""
Simple script that checks the status of the BMS. This includes checking the
current state of the state machine
"""
import canopen
from argparse import ArgumentParser


def main():
    argparser = ArgumentParser(description='''Utility to get the current state
                               of the BMS''')
    argparser.add_argument('port', action='store', type=str, help='''The
                           port of the can device to interface with the
                           BMS''')
    argparser.add_argument('node', action='store', type=int, help='''The
                           CANopen node ID of the BMS''')
    args = argparser.parse_args()

    network = canopen.Network()
    network.connect(channel=args.port, bustype='slcan')

    node = network.add_node(args.node, None)
    client = node.sdo

    print(client.upload(0x2102, 0x0))

    network.disconnect()


if __name__ == '__main__':
    main()
