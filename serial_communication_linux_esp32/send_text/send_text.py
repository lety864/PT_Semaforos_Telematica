import serial
import time

esp32 = serial.Serial(port='/dev/ttyUSB0',baudrate=9600,timeout=1)

def write_read(x):
	esp32.write(bytes(x, 'utf-8')) #informacion a enviar en bytes


while True:
	text= input("Escribe algo: ") #valor que se enviara mediante puerto serial
	write_read(text) #Envia la informacion
	
