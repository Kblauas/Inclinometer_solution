import serial
import time

# Configura a porta RS-485 (ajuste o nome da porta conforme o seu sistema)
rs485_port = 'COM5'  
baudrate = 115200

with serial.Serial(rs485_port, baudrate, timeout=1) as ser:
    print("Enviando: ping")
    ser.write(b'R\n')
    
    time.sleep(0.5)
    
    if ser.in_waiting:
        response = ser.readline().decode().strip()
        print(f"Recebido: {response}")
    else:
        print("Nenhuma resposta recebida.")
