# Replace RPi libraries by fake ones
import sys
try:
	import fake_rpi
	sys.modules['RPi'] = fake_rpi.RPi # Fake RPi
	sys.modules['RPi.GPIO'] = fake_rpi.RPi.GPIO # Fake GPIO
	import RPi.GPIO as GPIO
except ImportError:
	import RPi.GPIO as GPIO
import datetime
import socket
import re
from datetime import datetime
import signal
  
UDP_IP = "::" # = 0.0.0.0 u IPv4
UDP_PORT = 5678
SENSOR_IP = "fd00::212:4b00:f8e:2584"
BORDER_ROUTER_IP = "fd00::212:4b00:f19:b001"
SENSOR_UTC = datetime.utcnow()
BORDER_ROUTER_UTC = datetime.utcnow()


SENSOR_GPIO = 5 
BORDER_ROUTER_GPIO = 6

def signal_handler(sig, frame):
	GPIO.cleanup()
	sys.exit(0)

def sensor_sent_callback(channel):
	print("sensor pin pulled low")

def border_router_sent_callback(channel):
	print("border router pin pulled low")

def main():
	sock = socket.socket(socket.AF_INET6, # Internet
						socket.SOCK_DGRAM) # UDP
	sock.bind((UDP_IP, UDP_PORT))
	
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(SENSOR_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(BORDER_ROUTER_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.add_event_detect(SENSOR_GPIO, GPIO.RISING, callback=sensor_sent_callback)
	GPIO.add_event_detect(BORDER_ROUTER_GPIO, GPIO.RISING, callback=border_router_sent_callback)

	signal.signal(signal.SIGINT, signal_handler)
	#signal.pause()
	

	while True:
		data, addr = sock.recvfrom(1024) # Receive data from socket, buffersize 1024 bytes
		data_split = re.split(': |, ', data.decode('utf-8')) # Interpret data
		asn = (int(data_split[1]) << 32) + int(data_split[3]) # Bitshift msb 4 bytes to the left and add with lsb to get full ASN
		if(addr[0] == SENSOR_IP):
			SENSOR_UTC = datetime.utcnow()
			print("Sensor ASN:", asn, "Timestamp:", SENSOR_UTC)
		elif(addr[0] == BORDER_ROUTER_IP):
			BORDER_ROUTER_UTC = datetime.utcnow()
			print("Border Router ASN:", asn, "Timestamp:", BORDER_ROUTER_UTC)


if __name__ == "__main__":
	main()