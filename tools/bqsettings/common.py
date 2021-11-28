"""
Contains tools and utilities used both by the convert logic and the transfer
logic.
"""
from enum import Enum
from typing import List


class BQSetting:
    """
    Representation of the BQSetting as defined in TODO: Add URL. The setting
    has the ability to convert to CSV and the binary format as well as having
    the ability to be parsed in from CSV.
    """

    class BQSettingType(Enum):
        """
        BQSetting type, as defined in TODO: Add URL
        """
        Direct = 0
        Subcommand = 1
        RAM = 2

    def __init__(self, setting_type: BQSettingType, num_bytes: int,
                 address: int, data: int):
        """
        Create an instance of the BQSetting

        :param setting_type: The type of the setting
        :param num_bytes: The number of bytes of data (can be 0)
        :param address: The address of the setting
        :param data: The data that for the setting (can be 0 when num_bytes is
                     0)
        """
        self.setting_type = setting_type
        self.num_bytes = num_bytes
        self.address = address
        self.data = data

    def to_csv(self) -> str:
        """
        Convert the BQSetting into a single CSV line in the format
        BQSettingType,NumBytes,Address,Data

        :return: CSV line of the data
        """
        result = ''

        # Add in setting type
        if self.command_type == BQSetting.BQSettingType.Direct:
            result += 'Direct,'
        elif self.command_type == BQSetting.BQSettingType.Subcommand:
            result += 'Subcommand,'
        else:
            result += 'RAM,'

        # Add in number of bytes
        result += '{},'.format(self.num_bytes)

        # Add in address
        result += '{},'.format(self.address)

        # Add in data
        result += '{}'.format(self.data)

        return result

    @staticmethod
    def from_csv(string: str):
        """
        Convert the given CSV line into a BQSetting. Performs the opposite
        logic as `to_csv`.

        :param string: The CSV line to parse
        :return: The parsed BQSetting
        """
        line = string.split(',')

        setting_type = BQSetting.BQSettingType.Direct

        # Parse setting type
        if line[0] == 'Subcommand':
            setting_type = BQSetting.BQSettingType.Subcommand
        elif line[0] == 'RAM':
            setting_type = BQSetting.BQSettingType.RAM

        # Parse number of bytes
        num_bytes = int(line[1])

        # Parse address
        address = int(line[2])

        # Parse data
        data = int(line[3])

        return BQSetting(setting_type, num_bytes, address, data)

    @staticmethod
    def from_ti(string: str):
        """
        Convert the given TI format line (really just a CSV) into a BQSetting.
        This also handles the logic of converting the data from the "user
        friendly format" into a format that can be transfered.

        For example, this function will convert fractions into the packed
        IEEE format that is specified in "Section 3.3 Data Formats" of the
        BQ Technical Reference Manual.
        """
        content = string.split(',')

        data_type = content[4]
        address = int(content[6], 16)
        unconverted_data = int(content[7])
        conversion_string = content[12]

    def to_binary(self) -> bytearray:
        """
        Convert the BQSetting into a bytearray containing the binary contents
        of the setting in the format described TODO: Add URL

        :return: Byte array of the data in the fomat described in url above
        """
        result = bytearray()

        # Add in setting type and number of bytes
        command_byte = self.num_bytes << 2 & self.setting_type.value
        result.append(command_byte.to_bytes(1, 'little'))

        # Add in address
        result.append(self.address.to_bytes(2, 'little'))

        # Add in data
        result.append(self.data.to_bytes(4, 'little'))

        return result


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
