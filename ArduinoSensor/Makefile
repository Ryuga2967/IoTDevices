PORT=/dev/cu.usbmodem11201
#PORT=/dev/cu.usbserial-AI03QAR6
compile:
	arduino-cli compile --fqbn esp8266:esp8266:generic

upload:
	arduino-cli upload --fqbn esp8266:esp8266:generic -p ${PORT}

monitor:
	arduino-cli monitor -p ${PORT}

all: compile upload monitor
