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


def convert_to_binary(args: argparse.Namespace):
    """
    Converts the provided input file to a binary format.

    See the `convert` function for the expected arguments within args.
    """
    with open(args.input, 'r') as input_file:
        print(type(input_file))


def convert_to_csv(args: argparse.Namespace):
    """
    Convert the provided input file into the intermediate CSV format.

    See the `convert` function for the expected arguments with args.
    """
    pass


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
