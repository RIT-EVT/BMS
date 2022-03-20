import canopen

network = canopen.Network()
network.connect(channel='/dev/ttyACM1', bustype='slcan')

node = network.add_node(5, None)
client = node.sdo

print(client.upload(0x2102, 0x0))

network.disconnect()
