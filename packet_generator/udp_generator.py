from socket import *
import sys
import time
import random

# First mac : 00:0c:bf:26:c1:1d
# Second mac : 00:0c:bf:26:c1:1e
first_mac = "00:0c:bf:26:c1:1d"
sec_mac = "00:0c:bf:26:c1:1e"

# Create a UDP socket
sock = socket(AF_INET, SOCK_DGRAM)
sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
sock.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
sock.settimeout(5)

server_address = ('255.255.255.255', 12345)

# Messages
first_val = 1
sec_val = 2

# Send data
while True:
  print("Sending data...")

  first_message = 'VRADA;'+str(first_mac)+';speed:'+str(round(random.random()*100,2))+';rpm:'+str(round(random.random()*100,2))+';distance:0;calories:1;hf:0'+';power:'+str(round(random.random()*100,2))+';level:1;'
  sec_message = 'VRADA;'+str(sec_mac)+';speed:'+str(round(random.random()*100,2))+';rpm:'+str(round(random.random()*100,2))+';distance:0;calories:1;hf:0'+';power:'+str(round(random.random()*100,2))+';level:1;'

  sent = sock.sendto(first_message.encode(), server_address)
  sent = sock.sendto(sec_message.encode(), server_address)
  time.sleep(1.5)
