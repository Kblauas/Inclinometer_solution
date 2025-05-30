import serial
import time

ser = serial.Serial('COM5', 115200, timeout=1)

ser.write(b'R')  # Envia comando para o ESP
time.sleep(0.2)  # Aguarda resposta

data = ser.read_all()
print("Recebido:", data)

ser.close()
