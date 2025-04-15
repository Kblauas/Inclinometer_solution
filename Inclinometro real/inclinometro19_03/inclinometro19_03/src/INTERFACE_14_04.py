import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading
import os
from collections import deque
import serial
from serial.serialutil import SerialException
import struct
import time
import csv
from datetime import datetime
import random

# Definições de variaveis
SERIAL_PORT = 'COM42' # porta usada #COM42 pra teste sem estar conectado
BAUD_RATE = 115200
HEADER_BYTE = 0XAA
PACKET_SIZE = 13 # de 0 a 12 bytes
MAX_DATA_POINTS = 1000
SCALE_FACTOR = 100.0 # usado para converter os valores recebidos para float
SAVE_FOLDER = os.path.expanduser("~/Desktop/dados_gravados")

class SerialReader: 
    def __init__(self, port, baud_rate):
        self.port = port
        self.baud_rate = baud_rate
        self.ser = None
        self.simulated_data = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.connect()

    def connect(self): #testar a leitura da porta serial
        try:
            self.ser = serial.Serial(self.port, self.baud_rate, timeout=1)
        except SerialException as e:
            print(f"Erro ao abrir a porta {self.port}: {e}")
            self.ser = None
        
    def read_data(self):
        if not self.ser or not self.ser.is_open:
            print("Porta serial não está disponível")
            self.connect()  # Tenta reconectar
            return None #
            #return self._simulate_data() # só para testes com a porta serial desconectada

        try:
            if self.ser.in_waiting >= PACKET_SIZE:
                dados = self.ser.read(PACKET_SIZE)

                if dados[0] == HEADER_BYTE: # cabeçalho válido
                    roll, pitch, yaw, dev_r, dev_p, dev_y = struct.unpack('>hhhhhh', dados[1:])
                    return [v / SCALE_FACTOR for v in (roll, pitch, yaw, dev_r, dev_p, dev_y)]
                else:
                    self.ser.reset_input_buffer()
        except SerialException as e:
            print(f"Erro na leitura serial: {e}")
            self.ser.close()
            self.ser = None
            return None
        except Exception as e:
            print(f"Erro inesperado na leitura: {e}")
            return None
    
    def _simulate_data(self): # só para testes sem a serial conectada
        self.simulated_data = [x + random.uniform(-1,1) for x in self.simulated_data]
        return self.simulated_data

class DynamicPlotApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Monitor do Inclinômetro")
        self.serial_reader = SerialReader(SERIAL_PORT, BAUD_RATE)

        self.serial_reader = None # ambos usados só para testes sem estar conectado
        self.USE_SIMULATED_DATA = True

        self.stop_event = threading.Event()
        self.lock = threading.Lock()

        # inicializa as variáveis
        self.setup_variables()
        self.create_widgets()
        self.setup_plots()   
        self.start_plot_updater()    
    
    def start_plot_updater(self):
        if not hasattr(self, 'plot_update_thread'):
            self.plot_update_thread = threading.Thread(target=self._update_plots_thread, daemon=True)
            self.plot_update_thread.start()

    def create_widgets(self): # para criar os elementos em tela
        top_frame = ttk.Frame(self.root)
        top_frame.grid(row=0, column=0, columnspan=4, sticky="ew", padx=5, pady=5)

        top_frame.columnconfigure(0, weight=1)
        top_frame.columnconfigure(1, weight=0)
        top_frame.columnconfigure(2, weight=1)
        top_frame.columnconfigure(3, weight=1)

        self.start_read_button = ttk.Button(top_frame, text="Iniciar Leitura e Gravação", command=self.read_and_record)
        self.start_read_button.grid(row=0, column=0, padx=5, pady=5, sticky="w")

        self.depth_label = ttk.Label(top_frame, text=f'Medindo: {self.depth[self.indice]} metros')
        self.depth_label.grid(row=0, column=1, padx=5, pady=5, sticky="w")

        self.status_label = ttk.Label(top_frame, text="Pronto")
        self.status_label.grid(row=0, column=2, padx=5, pady=5)

        self.close_button = ttk.Button(top_frame, text="Fechar programa", command=self.on_close)
        self.close_button.grid(row=0, column=3, padx=20, pady=5, sticky="e")

    def setup_variables(self):
        self.indice = 0
        self.recording = False
        self.paused = False
        self.active_colect = False
        self.plotting_active = False
        self.save_on_exit = True
        self.collected_data = []
        self.all_data = []
        self.depth = [str(x) for x in [0.5 + i*0.5 for i in range(40) if 0.5 + i*0.5 <= 20.0]] # criar valores a cada 0.5 até 20.0

        self.time_data  = deque(maxlen=MAX_DATA_POINTS) # eixo x dos gráficos

        self.roll_data  = deque(maxlen=MAX_DATA_POINTS) # eixo x
        self.pitch_data = deque(maxlen=MAX_DATA_POINTS) # eixo y
        self.yaw_data   = deque(maxlen=MAX_DATA_POINTS) # eixo z

        self.devR_data   = deque(maxlen=MAX_DATA_POINTS) # desvio/desviation em Roll
        self.devP_data   = deque(maxlen=MAX_DATA_POINTS) # desvio/desviation em Pitch
        self.devY_data   = deque(maxlen=MAX_DATA_POINTS) # desvio/desviation em Yaw

    def setup_plots(self): # para configurar e ajustar os plots
        self.fig, self.axes = plt.subplots(2, 1, figsize=(10,8))
        (self.ax_angle, self.ax_dev) = self.axes.flatten()
        for ax in [self.ax_angle, self.ax_dev]:
            ax.grid(True)
            ax.set_xlabel('Tempo (s)')

        self.ax_angle.set_ylabel('Ângulo (°)')
        self.ax_dev.set_ylabel('Desvio (°)')

        self.ax_angle.set_title('Dados dos Ângulos')
        self.ax_dev.set_title('Desvio dos Ângulos')

        self.line_roll  = self.ax_angle.plot([], [], label="Roll",  color="blue")[0]
        self.line_pitch = self.ax_angle.plot([], [], label="Pitch", color="red")[0]
        self.line_yaw   = self.ax_angle.plot([], [], label="Yaw",   color="green")[0]

        self.line_devR = self.ax_dev.plot([], [], label="Dev Roll",  color="cyan")[0]
        self.line_devP = self.ax_dev.plot([], [], label="Dev Pitch", color="magenta")[0]
        self.line_devY = self.ax_dev.plot([], [], label="Dev Yaw"  , color="orange")[0]

        self.ax_angle.legend(loc='upper right') # deixa as legendas em um lugar fixo
        self.ax_dev.legend(loc='upper right')
        self.fig.tight_layout(pad=3.0)

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().grid(row=1, column=0, columnspan=2)

    def read_and_record(self):
        if not self.active_colect:
            self.active_colect = True
            self.plotting_active = True

            self.root.after(0, self.clear_plots_display) # limpa os gráficos antes de mostrar novos dados

            self.collected_data = [] # para limpar a lista
            self.start_read_button.config(text="Coletando dados...", state=tk.DISABLED)
            threading.Thread(target=self._thread_colect, daemon=True).start()

    def _thread_colect(self):
        try:
            for _ in range(10):
                if not self.active_colect: 
                    break

                with self.lock:
                    self.data = self.serial_reader.read_data()
                
                if self.data:
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
                    self.collected_data.append((timestamp, *self.data))
                    self.all_data.append((self.depth[self.indice], *self.data))

                    # pra atualiar a interface a cada leitura
                    self.root.after(0, lambda: self.status_label.config(text=f'Coletado {len(self.collected_data)}/10'))

                time.sleep(.1)
            self.plotting_active = False
        except Exception as e:
            print(f"Erro na coleta: {e}")
        finally:
            self.active_colect = False
            self.root.after(0, lambda: self.status_label.config(text="Pronto" if len(self.collected_data) == 10 else "Coleta interrompida"))
            self.start_read_button.config(text="Iniciar Leitura e Gravação", state=tk.NORMAL)
            self.indice+= 1
            self.depth_label.config(text=f"Medindo: {self.depth[self.indice]} metros")

    def _update_plots(self):
        try:
            self.line_roll.set_data(self.time_data, self.roll_data)
            self.line_pitch.set_data(self.time_data, self.pitch_data)
            self.line_yaw.set_data(self.time_data, self.yaw_data)

            self.line_devR.set_data(self.time_data, self.devR_data)
            self.line_devP.set_data(self.time_data, self.devP_data)
            self.line_devY.set_data(self.time_data, self.devY_data)

            self.ax_angle.relim()
            self.ax_angle.autoscale_view(True, True, True)
            self.ax_dev.relim()
            self.ax_dev.autoscale_view(True, True, True)

            self.canvas.draw_idle() # para atualizar o canvas
        except Exception as e:
            print(f"Erro ao atulizar os plots: {e}")

    def _update_plots_thread(self):
        while not self.stop_event.is_set():
            if self.plotting_active and not self.paused:
                data = self.serial_reader.read_data()
                if data:
                    with self.lock:
                        current_time = time.time()
                        self.time_data.append(current_time)
                        self.roll_data.append(data[0])
                        self.pitch_data.append(data[1])
                        self.yaw_data.append(data[2])
                        self.devR_data.append(data[3])
                        self.devP_data.append(data[4])
                        self.devY_data.append(data[5])

                self.root.after(0, self._update_plots)
            #time.sleep(0.05)
            time.sleep(2) # testar 

    def clear_plots_display(self):
        self.time_data.clear()
        self.roll_data.clear()
        self.pitch_data.clear()
        self.yaw_data.clear()
        self.devR_data.clear()
        self.devP_data.clear()
        self.devY_data.clear()
        
        self.line_roll.set_data([], [])
        self.line_pitch.set_data([], [])
        self.line_yaw.set_data([], [])
        self.line_devR.set_data([], [])
        self.line_devP.set_data([], [])
        self.line_devY.set_data([], [])
        
        self.canvas.draw_idle()

    def tela_popup(self, nome_local):
        popup = tk.Toplevel(self.root)
        popup.title("Local salvo")
        popup.geometry("600x300")  # Define o tamanho da janela
        popup.protocol("WM_DELETE_WINDOW", lambda:self.end_all())

        popup.transient(self.root)
        popup.grab_set()

        main_frame = ttk.Frame(popup, padding=10)
        main_frame.pack(fill=tk.BOTH, expand=True)

        ttk.Label(main_frame, text=f'Salvo em:\n{nome_local}', font=('Arial', 10)).pack(pady=(0, 5))
        ttk.Button(main_frame, text="OK", command=self.end_all).pack(pady=10)

        popup.update_idletasks() # para ajustar o popup no meio da tela
        x = self.root.winfo_x() + (self.root.winfo_width()  - popup.winfo_width())  // 2
        y = self.root.winfo_y() + (self.root.winfo_height() - popup.winfo_height()) // 2
        popup.geometry(f"+{x}+{y}")

        self.root.wait_window(popup) # para esperar o popup fechar

    def save_data(self, filename=None):
        if not self.all_data:
            return False
        try:
            os.makedirs(SAVE_FOLDER, exist_ok=True)
            if not filename:
                filename = os.path.join(SAVE_FOLDER, f"dados_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv")
            with open(filename, 'w', newline='', encoding='utf-8') as file:
                writer = csv.writer(file)
                writer.writerow(['Profundidade', 'Roll', 'Pitch', 'Yaw', 'DeviationR', 'DeviationP', 'DeviationY'])
                writer.writerows(self.all_data)            
            self.tela_popup(filename)  # ativa a tela de popup
            print(f'Dados salvos em {filename}')
            return True
        except Exception as e:
            self.status_label.config(text=f"Erro ao salvar: {str(e)}")
            print(f'Erro ao salvar {str(e)}')
            return False

    def end_all(self):
        self.stop_event.set()
        self.plotting_active = False

        if self.serial_reader.ser and self.serial_reader.ser.is_open:
            self.serial_reader.ser.close()
            print("Porta serial fechada.") 

        self.root.destroy()

        os._exit(0)

    def on_close(self):
        print("Fechando a aplicação...")

        if self.save_on_exit and self.all_data:
            self.save_on_exit = False
            self.save_data()
        else:
            print("Nenhum dado para ser salvo")
            self.end_all() # fecha a tela se não houver dados
    
if __name__ == "__main__":
    print(f"Os arquivos salvos em {SAVE_FOLDER}")
    root = tk.Tk()
    app = DynamicPlotApp(root)

    # Caso seja fechado clicando no "X"
    root.protocol("WM_DELETE_WINDOW", app.on_close)

    try:
        root.mainloop()
    finally:
        app.stop_event.set()
        if hasattr(app, 'serial_reader'):
            ser = app.serial_reader.ser
            if ser and ser.is_open:
                ser.close()