import canopen
import struct

network = canopen.Network()
network.connect(channel='/dev/ttyACM1', bustype='slcan')

node = network.add_node(5, None)
client = node.sdo

print(client.download(0x2102, 0x0, struct.pack('<I', 0x0)))

network.disconnect()
