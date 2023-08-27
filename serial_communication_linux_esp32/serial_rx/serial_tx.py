import serial
import time

esp32 = serial.Serial(port='/dev/ttyUSB0',baudrate=115200,timeout=.1)

def write_read(x):
	esp32.write(bytes(x, 'utf-8')) #informacion a enviar en bytes
	time.sleep(0.05) #tiempo de espera
	data = esp32.readline() #leyendo del puerto serial la informacion mandada por el arduino
	return data

while True:
	num= input("Escribe algo: ") #valor que se enviara mediante puerto serial
	value = write_read(num)
	print("valor+1: ",value) #imprimir valor del puerto serial
