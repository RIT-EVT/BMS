import canopen
import struct

# Connect to CANopen network
network = canopen.Network()
network.connect(channel='/dev/ttyACM1', bustype='slcan')

# Make an SDO client for communicating with the BMS
# TODO: Once we have EDS for the BMS, this None will be replaced with
#       a path to the EDS
node = network.add_node(5, None)
client = node.sdo

client.download(0x2100, 0x0,
                struct.pack('<H',
                            0))
