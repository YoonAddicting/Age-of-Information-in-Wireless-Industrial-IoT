import sys #To import the fake_rpi module and exit.
try: #If run on MacOS
	import fake_rpi
	sys.modules['RPi'] = fake_rpi.RPi # Fake RPi
	sys.modules['RPi.GPIO'] = fake_rpi.RPi.GPIO # Fake GPIO
	import RPi.GPIO as GPIO
except ModuleNotFoundError: #If run on RPi
	import RPi.GPIO as GPIO
import datetime # For getting the system time
import socket # To open a network connection
import re # To regex split the incomming data
from datetime import date, datetime # For getting the system time
import time # High resolution time since epoch
import signal # To properly handle SIGINT (ctrl+c)
  
UDP_IP = "::" # Receive from all IPs
UDP_PORT = 5678 # Port to listen on
SENSOR_IP = "fd00::212:4b00:f8e:2584" # IPv6 address of the sensor
BORDER_ROUTER_IP = "fd00::212:4b00:f19:b001" # IPv6 address of the border router
TIME_BORDER_ROUTER_INT = time.time_ns() # Store the time of the received interupt from border router
TIME_BORDER_ROUTER_REC = time.time_ns() # Store the time of the received package from border router
TIME_SENSOR_INT = time.time_ns() # Store the time of the received interupt from snesor
TIME_SENSOR_REC = time.time_ns() # Store the time of the received package from sensor
ASN_BORDER_ROUTER = 0 # Store the last received ASN of the border router
ASN_SENSOR = 0 # Store the last received ASN of the sensor
SLOT_DURATION = 10 ** 6 # TSCH Slot duration in ns

SENSOR_GPIO = 5 # GPIO pin to listen on for the sensor
BORDER_ROUTER_GPIO = 6 # GPIO pin to listen on for the border router

err_bench = open(f"{date.today().year:02}" + f"{date.today().month:02}" + f"{date.today().day:02}" + "_" +
            f"{datetime.now().hour:02}" + f"{datetime.now().minute:02}" + f"{datetime.now().second:02}" + "_bench.txt",'w')
err_pred = open(f"{date.today().year:02}" + f"{date.today().month:02}" + f"{date.today().day:02}" + "_" +
            f"{datetime.now().hour:02}" + f"{datetime.now().minute:02}" + f"{datetime.now().second:02}" + "_pred.txt",'w')

def sigint_handler(sig, frame):
	global err_bench
	global err_pred
	GPIO.remove_event_detect(SENSOR_GPIO)
	GPIO.remove_event_detect(BORDER_ROUTER_GPIO)
	GPIO.cleanup()
	# Close the files properly
	err_bench.close()
	err_pred.close()
	sys.exit(0)

def int_sensor_callback(channel):
	# Update the time for the last interupt received
	global TIME_SENSOR_INT
	TIME_SENSOR_INT = time.time_ns()

def int_border_router_callback(channel):
	# Update the time for the last interupt received
	global TIME_BORDER_ROUTER_INT 
	TIME_BORDER_ROUTER_INT = time.time_ns()

def main():
	# Access the global variables 
	global TIME_BORDER_ROUTER_INT
	global TIME_BORDER_ROUTER_REC
	global TIME_SENSOR_INT
	global TIME_SENSOR_REC
	global ASN_BORDER_ROUTER
	global ASN_SENSOR

	sock = socket.socket(socket.AF_INET6, # IPv6
						socket.SOCK_DGRAM) # UDP
	sock.bind((UDP_IP, UDP_PORT)) # Open the connection
	
	GPIO.setmode(GPIO.BCM) # Access pins by the Broadcom SOC channel
	GPIO.setup(SENSOR_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Set to expect to be pulled low
	GPIO.setup(BORDER_ROUTER_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Set to expect to be pulled low
	GPIO.add_event_detect(SENSOR_GPIO, GPIO.FALLING, callback=int_sensor_callback, bouncetime=200) # Detect interupt event when the GPIO pin of the sensor is falling, and call int_sensor_callback
	GPIO.add_event_detect(BORDER_ROUTER_GPIO, GPIO.FALLING, callback=int_border_router_callback, bouncetime=200) # Detect interupt event when the GPIO pin of the border router is falling, and call int_border_router_callback

	signal.signal(signal.SIGINT, sigint_handler) # Handle if a SIGINT (ctrl+c) is received

	# TODO: Do different stuff depending on mode
	# TODO: Implement way to stop after x samples
	
	while True:
		data, addr = sock.recvfrom(1024) # Receive data from socket, buffersize 1024 bytes
		data_split = re.split(': |, ', data.decode('utf-8')) # Split data into array by RegEx
		if(addr[0] == SENSOR_IP): # If sent from sensor
			TIME_SENSOR_REC = time.time_ns() # Update the time of received ASN
			ASN_SENSOR = (int(data_split[1]) << 32) + int(data_split[3]) # Bitshift msb 4 bytes to the left and add with lsb to get full ASN
			err_bench_data = (TIME_SENSOR_REC - TIME_SENSOR_INT) / (10 ** 6) # Convert time_ns result into floating point milliseconds
			err_bench.write(f"{err_bench_data:.5f}" + "\n")
			time_sensor_pred = TIME_BORDER_ROUTER_INT + (ASN_SENSOR - ASN_BORDER_ROUTER) * SLOT_DURATION # Predicted time the package were sent from the sensor
			err_pred_data = (time_sensor_pred - TIME_SENSOR_INT) / (10 ** 6)
			err_pred.write(f"{err_pred_data:.5f}" + "\n")
			print("Sensor ASN: {}\nTimestamp: {}\nError benchmark: {:.5f}\nError of prediction: {:.5f}".format(ASN_SENSOR, TIME_SENSOR_REC, err_bench_data, err_pred_data))
		elif(addr[0] == BORDER_ROUTER_IP): # If sent from border router 
			TIME_BORDER_ROUTER_REC = time.time_ns() # Update the time of the received ASN
			ASN_BORDER_ROUTER = (int(data_split[1]) << 32) + int(data_split[3]) # Bitshift msb 4 bytes to the left and add with lsb to get full ASN
			print("Border Router ASN: {}\nTimestamp: {}".format(ASN_BORDER_ROUTER, TIME_BORDER_ROUTER_REC))


if __name__ == "__main__":
	main()
