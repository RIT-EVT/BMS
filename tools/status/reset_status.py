import canopen
import struct

network = canopen.Network()
network.connect(channel='/dev/cu.usbmodem143201', bustype='slcan')

node = network.add_node(5, None)
client = node.sdo

print(client.download(0x2102, 0x0, struct.pack('<I', 0x4)))

network.disconnect()
