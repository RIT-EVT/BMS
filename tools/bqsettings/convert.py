"""
Used to convert the TI produced settings file into a binary file. The binary
file containing the list of BQ settings in the format documented at
<TODO ADD LINK>. The script also contains the logic to save as an intermediate
format which is a CSV of just the settings.

:author: Collin Bolles
"""
import argparse
import os
import sys
from common import BQSetting
import pathlib
from typing import List
import serial


def ti_to_uart(file_path: str, port_name: str) -> None :
    """
    Load a list of the BQSettings from a TI file, and write the settings
    to the BMS over the given serial port.

    :param file_path: Path to the TI file to parse.
    :param port_name: Port that the BMS is connected to.
    """
    settings = []
    with open(file_path, 'r') as ti_file:
        for line in ti_file:
            # Check for, and ignore, comments
            if line[0] == '*':
                continue
            settings.append(BQSetting.from_ti(line))

    stm = serial.Serial(port_name)
    stm.write(len(settings).to_bytes(2, 'little'))

    for setting in settings :
        val = stm.read()
        if val != b'\0':
            print(val)
            print("STM failure")
            return

        byteArr = setting.to_binary()
        stm.write(byteArr)
        print(byteArr)
    print("Complete")
        


def load_from_ti(file_path: str) -> List[BQSetting]:
    """
    Load a list of the BQSettings from a TI file. Note, the TI file itself
    only contains RAM settings so all settings will be of type RAM.

    :param file_path: Path to the TI file to parse.
    :return: List of parsed BQSettings
    """
    settings = []
    with open(file_path, 'r') as ti_file:
        for line in ti_file:
            # Check for, and ignore, comments
            if line[0] == '*':
                continue
            settings.append(BQSetting.from_ti(line))
    return settings


def load_from_csv(file_path: str) -> List[BQSetting]:
    """
    Load a list of the BQSettings from a CSV file.

    :param file_path: Path to the CSV file to parse
    :return: List of the parsed BQSettings
    """
    settings = []
    with open(file_path, 'r') as csv_file:
        for line in csv_file:
            # Check for, and ignore, comments
            if line[0] == '*':
                continue
            settings.append(BQSetting.from_csv(line))
    return settings


def save_to_csv(file_path: str, settings: List[BQSetting]) -> None:
    """
    Save the given settings into a CSV.
    """
    with open(file_path, 'w') as csv_file:
        # Write out header
        csv_file.write('* Setting Type,Number of Bytes,Address,Data\n')
        for setting in settings:
            csv_file.write(setting.to_csv() + '\n')


def save_to_binary(file_path: str, settings: List[BQSetting]) -> None:
    """
    Save the provided settings into a binary format. The data will have all
    the settings listed back-to-back. See TODO: Add URL for more details

    :param file_path: The path to the file to write out the binary data
    :param settings: The list of settings to convert
    """
    with open(file_path, 'wb') as binary_file:
        for setting in settings:
            binary_file.write(setting.to_binary())


def is_ti_file(file_path: str) -> bool:
    """
    Return true if the given file path is for a TI settings file, TI
    settings files should be names with ".gg.csv" as the extension

    :param file_path: The file to check the type of
    :return: True if the file has the extension ".gg.csv"
    """
    return pathlib.Path(file_path).name.endswith('.gg.csv')


def convert_to_binary(args: argparse.Namespace):
    """
    Converts the provided input file to a binary format.

    See the `convert` function for the expected arguments within args.
    """
    settings = []
    if is_ti_file(args.input):
        settings = load_from_ti(args.input)
    else:
        settings = load_from_csv(args.input)
    save_to_binary(args.output, settings)


def convert_to_csv(args: argparse.Namespace):
    """
    Convert the provided input file into the intermediate CSV format.

    See the `convert` function for the expected arguments with args.
    """
    settings = []
    if is_ti_file(args.input):
        settings = load_from_ti(args.input)
    else:
        return
    save_to_csv(args.output, settings)


def convert(args: argparse.Namespace):
    """
    Entry point into the convert logic. Handles running the proper convert
    logic based on the provided arguments. The arguments are the parsed
    command line arguments that are handled in `run.py`.

    Expected Arguments
    ==================
    From `run.py`, the args will include

    * input (required): The input file, can either be in the TI format, or the
      intermediate CSV format
    * output (required): The output file, where to store either the
      intermediate CSV file or the final binary file
    * target (defaults to binary): The target output format, either binary
      or CSV
    """
    # Validate the input file exists
    if not os.path.exists(args.input):
        print('Input file: {}, does not exist'.format(args.input),
              file=sys.stderr)
        exit(1)

    # Check the conversion type (csv or binary) and execute the conversion
    if args.target == 'binary':
        convert_to_binary(args)
    else:
        convert_to_csv(args)
