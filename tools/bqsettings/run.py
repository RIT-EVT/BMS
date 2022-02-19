"""
Provides a CLI for converting the TI settings file into a binary file and
for setting that binary file over to the BMS. The user has the ability to
select the file to convert to binary form, and has the ability to stop during
intermidate steps in order to perform edits to the converted format.

:author: Collin Bolles
"""
from argparse import ArgumentParser
from convert import convert
from transfer import transfer


def main():
    # Top level arguments
    argparser = ArgumentParser(description='''Utility for converting BQ
                               settings to binary and sending the settings over
                               to the BMS''')
    subparsers = argparser.add_subparsers(dest='command', required=True)

    # Arguments for the convert command
    convert_parser = subparsers.add_parser('convert', help='''Command to for
                                           handling the logic of converting
                                           the file into different formats''')
    convert_parser.add_argument('input', action='store', help='''The file that
                                is going to be converted into either binary or
                                the intermediate CSV format. This can either be
                                the TI provided file, or the intermediate CSV
                                format itself.''')
    convert_parser.add_argument('output', action='store', help='''The
                                destination to save either the binary format or
                                the CSV data''')
    convert_parser.add_argument('--target', action='store', type=str,
                                help='''The target format, either binary or
                                CSV, defaults to binary''', default='binary',
                                choices=['binary', 'csv'])

    # Arguments for the transfer command
    transfer_parser = subparsers.add_parser('transfer')
    transfer_parser.add_argument('input', action='store', help='''The binary
                                 file containing the settings to transfer''')
    transfer_parser.add_argument('port', action='store', help='''Port of the
                                 SLcan device to send settings to''')
    transfer_parser.add_argument('bms_node', action='store', type=int,
                                 help='The CANopen node of the BMS')

    args = argparser.parse_args()

    if args.command == 'convert':
        convert(args)
    else:
        transfer(args)


if __name__ == '__main__':
    main()
