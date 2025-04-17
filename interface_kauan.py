import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading
import os
import serial
import struct
from collections import deque

# Configuração da porta serial

ser = serial.Serial('COM5', 115200, timeout=1)
print(f"Serial aberto: {ser.is_open}")
print(f"Porta: {ser.port}")



def ler_dados():
    print("Tentando ler dados...")  # DEBUG
    if ser.in_waiting >= 1:
        dados = ser.read(9)
        print(f"Raw: {[hex(b) for b in dados]}")
        print(f"Dados brutos recebidos: {dados}")
        if dados[0] == 0xFF:
            valores = struct.unpack('>hhhh', dados[1:])
            print(f"Valores convertidos: {valores}")
            return [v / 100.0 for v in valores]
    return None


class DynamicPlotApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Gráficos Dinâmicos")
        self.recording = False
        self.data = []
        self.create_widgets()
        tamanho_maximo = 100000
        self.x_data = deque(maxlen=tamanho_maximo)
        self.roll_data = deque(maxlen=tamanho_maximo)
        self.pitch_data = deque(maxlen=tamanho_maximo)
        self.yaw_data = deque(maxlen=tamanho_maximo)
        self.deviation_data = deque(maxlen=tamanho_maximo)
        self.lock = threading.Lock()

    def create_widgets(self):
        self.start_read_button = ttk.Button(self.root, text="Iniciar Leitura", command=self.start_read)
        self.start_read_button.grid(row=0, column=0, padx=5, pady=5)

        self.stop_button = ttk.Button(self.root, text="Parar", command=self.stop_recording)
        self.stop_button.grid(row=0, column=2, padx=5, pady=5)

        self.fig, self.axes = plt.subplots(4, 1, figsize=(10, 8))
        self.fig.tight_layout(pad=3.0)
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().grid(row=1, column=0, columnspan=3)

        self.ax1, self.ax2, self.ax3, self.ax4 = self.axes.flatten()
        self.ax1.set_title("Roll")
        self.ax2.set_title("Pitch")
        self.ax3.set_title("Yaw")
        self.ax4.set_title("Deviation")

    def start_read(self):
        if not self.recording:  # Prevent multiple threads
            ser.write(b"S")
            print("Comando S enviado ao ESP32")
            ler_dados()
            self.recording = True
            threading.Thread(target=self.update_data, daemon=True).start()
       

    def stop_recording(self):
        self.recording = False
        self.save_data()

    def update_data(self):
        self.contador = 0  
        while self.recording:
            leitura = None  # Garante que a variável exista
            if ser.in_waiting >=1:
                leitura = ler_dados()
            if leitura:
                print("Leitura recebida com sucesso:", leitura)
            if leitura:
                roll, pitch, yaw, deviation = leitura
                with self.lock:
                    self.x_data.append(self.contador)
                    self.roll_data.append(roll)
                    self.pitch_data.append(pitch)
                    self.yaw_data.append(yaw)
                    self.deviation_data.append(deviation)
                    self.contador += 1
                self.update_plots()

            
    def update_plots(self):
        with self.lock:
            self.ax1.cla()
            self.ax1.plot(self.x_data, self.roll_data, label="Roll", color="blue")
            self.ax1.set_title("Roll")
            self.ax1.legend()

            self.ax2.cla()
            self.ax2.plot(self.x_data, self.pitch_data, label="Pitch", color="red")
            self.ax2.set_title("Pitch")
            self.ax2.legend()

            self.ax3.cla()
            self.ax3.plot(self.x_data, self.yaw_data, label="Yaw", color="green")
            self.ax3.set_title("Yaw")
            self.ax3.legend()

            self.ax4.cla()
            self.ax4.plot(self.x_data, self.deviation_data, label="Deviation", color="cyan")
            self.ax4.set_title("Deviation")
            self.ax4.legend()

            self.canvas.draw()


    def save_data(self):
        folder_path = "C:\\Users\\Kauan\\Documents\\Trabalho\\Data"
        if not os.path.exists(folder_path):
            os.makedirs(folder_path)
        file_path = os.path.join(folder_path, "dados_gravados.txt")
        with open(file_path, "w") as file:
            file.write("Timestamp, Roll, Pitch, Yaw, Deviation\n")
            for t, r, p, y, d in zip(self.x_data, self.roll_data, self.pitch_data, self.yaw_data, self.deviation_data):
                file.write(f"{t}, {r:.2f}, {p:.2f}, {y:.2f}, {d:.2f}\n")
        print(f"Dados salvos em {file_path}")

root = tk.Tk()
app = DynamicPlotApp(root)
root.mainloop()
