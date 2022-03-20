"""
Utility for transfering the binary representation of the setting over to the
BMS. The binary transfer utility assumes a serial communication with an
Arudino that is running the provided script.

The binary file can also be sent over to the BMS via other methods including
using the SDO file transfer logic of the Vector CAN.

:author: Collin Bolles
"""
import argparse
import os
import sys
from common import BQSetting
import canopen
import struct


def get_num_settings(binary_file: str) -> int:
    """
    Determine the number of settings contained in the binary file. It is
    understood that the binary file only contains the settings and that the
    settings are each 7 bytes in size.

    :param binary_file: The file containing the settings in binary form
    """
    return int(os.path.getsize(binary_file) / BQSetting.SETTING_SIZE)


def transfer(args: argparse.Namespace):
    """
    Handles the logic of transfering the binary data over a serial connection
    to the Arduino running the provided code.

    Expected Arguments
    ==================
    From `run.py`, the args will include

    * input (required): The binary file to transfer
    * port (required): The serial port of the Arduino
    """
    if not os.path.exists(args.input):
        print('Input file: {}, does not exist'.format(args.input),
              file=sys.stderr)
        exit(1)

    # Connect to CANopen network
    network = canopen.Network()
    network.connect(channel=args.port, bustype='slcan')

    # Make an SDO client for communicating with the BMS
    # TODO: Once we have EDS for the BMS, this None will be replaced with
    #       a path to the EDS
    node = network.add_node(5, None)
    client = node.sdo

    # Read in all bytes of the data
    with open(args.input, 'rb') as input_file:
        settings_bin = input_file.read()

    # Transfer the number of settings over
    client.download(0x2100, 0x0,
                    struct.pack('<H', get_num_settings(args.input)))

    # Transfer the settings themselves
    client.download(0x2100, 0x1, settings_bin)
