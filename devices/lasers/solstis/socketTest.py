import socket
import json

## Note: The IP address needs to be the address of the SolsTis.
### The ip_address sent in the startMessage must be the local IP, and must match one of the remote interfaces listed in the SolsTiS config.
### The TCP_PORT must match the port listed in the SolsTiS configuration (confusing: the "port" field in the config must be
### the port that the solistis will listen to, not any port associated with the remote interface.

TCP_IP = '192.168.1.222'
TCP_PORT = 39933
buffer_size = 1024

localAddress = "sr-optics"

localAddress = (localAddress + '.') if localAddress.find('.') == -1 else localAddress
localIPaddr = socket.gethostbyname(localAddress)

#startMess = b'{\"message\":{ \"transmission_id\":[0],\"op\":\"start_link\",\"parameters\":{\"ip_address\":\"192.168.1.235\"}}}'
startMess = str.encode('{\"message\":{ \"transmission_id\":[0],\"op\":\"start_link\",\"parameters\":{\"ip_address\":\"' + localIPaddr + '\"}}}')

pingMess = b'{\"message\":{ \"transmission_id\":[1],\"op\":\"ping\",\"parameters\":{\"text_in\":\"testText\"}}}'
statusMess = b'{\"message\":{ \"transmission_id\":[2],\"op\":\"get_status\"}}'

#print(startMess)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect((TCP_IP, TCP_PORT))


s.send(startMess)
data = s.recv(buffer_size)
print(data)

s.send(pingMess)
data = s.recv(buffer_size)
print(data)

s.send(statusMess)
data = s.recv(buffer_size)
print(data)

# parse JSON
status = json.loads(data)
print("Status:")
print("Output PD: ", status["message"]["parameters"]["output_monitor"][0])

s.close()


