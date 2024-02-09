"""
Used to convert settings files to binary and transfer them to the BMS. Currently
only supports transfer over UART

:author: Matthew Magee
"""
import argparse
import os
import sys
from common import BQSetting
from convert import load_from_ti
import pathlib
from typing import List
import serial


def ti_to_uart(file_path: str, port_name: str) -> None:
    """
    Load a list of the BQSettings from a TI file, and write the settings
    to the BMS over the given serial port.

    :param file_path: Path to the TI file to parse.
    :param port_name: Port that the BMS is connected to.
    """
    settings = load_from_ti(file_path)

    stm = serial.Serial(port_name)
    stm.write(len(settings).to_bytes(2, 'little'))

    for setting in settings:
        val = stm.read()
        if val != b'\0':
            print(val)
            print("STM failure")
            return

        byte_arr = setting.to_binary()
        stm.write(byte_arr)
        print(byte_arr)
    print("Complete")


# TODO: Add support for CANopen transfer
def convert_transfer(args: argparse.Namespace):
    """
    Entry point into the convert_transfer logic. Handles running the proper
    logic based on the provided arguments. The arguments are the parsed
    command line arguments that are handled in `run.py`.

    Expected Arguments
    ==================
    From `run.py`, the args will include

    * input (required): The input file in TI format
    * serial_port (required): The name of the serial port to send data on
    """
    # Validate the input file exists
    if not os.path.exists(args.input):
        print('Input file: {}, does not exist'.format(args.input),
              file=sys.stderr)
        exit(1)

    # Check the conversion type (csv or binary) and execute the conversion
    ti_to_uart(args.input, args.port)
