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
import serial
from common import BQSetting
import time


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

    with serial.Serial(args.port, 9600) as ser:
        num_settings = get_num_settings(args.input)
        print(num_settings)

        ser.write(num_settings.to_bytes(2, 'little'))

        total_settings_read = 0
        with open(args.input, 'rb') as input_file:
            while total_settings_read < num_settings:
                raw_setting = input_file.read(BQSetting.SETTING_SIZE)
                ser.write(raw_setting)
                total_settings_read += 1
                time.sleep(0.01)

        print(total_settings_read)
