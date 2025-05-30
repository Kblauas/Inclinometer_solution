import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import askretrycancel, showinfo
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

# Definições de variaveis
SERIAL_PORT = 'COM4' # porta usada
BAUD_RATE = 115200
HEADER_BYTE = 0XFF
PACKET_SIZE = 9 # de 0 a 12 bytes
MAX_DATA_POINTS = 1000
SCALE_FACTOR = 100.0 # usado para converter os valores recebidos para float
SAVE_FOLDER = os.path.expanduser("~/Desktop/dados_gravados")

class SerialReader: 
    def __init__(self, port, baud_rate):
        self.port = port
        self.baud_rate = baud_rate
        self.ser = None
        self.active = False
        self.connect()

    def set_active(self, active):
        self.active = active

    def connect(self):
        try:
            self.ser = serial.Serial(self.port, self.baud_rate, timeout=1)
            time.sleep(1)  # Aguarda um segundo para garantir que a conexão seja estável
            print(f"Serial aberto: {self.ser.is_open}")
            print(f"Porta: {self.ser.port}")
            print("Porta aberta e OK")  # Para teste
        except SerialException as e:
            print(f"Erro ao abrir a porta {self.port}: {e}")
            self.ser = None
        
    def read_data(self):
        if not self.active: # para não ler se não estiver ativo
            return None
        
        start = time.time()
        while self.ser.in_waiting < PACKET_SIZE:
            if time.time() - start > 1:  # Timeout de 1 segundo
                return None
        time.sleep(0.01)
    # Lê exatamente o pacote inteiro
        dados = self.ser.read(PACKET_SIZE)
    
    # Verifica se o primeiro byte é o cabeçalho esperado
        if dados[0] != HEADER_BYTE:
            print("Cabeçalho inválido, tentando sincronizar...")
        # Se não for o cabeçalho, descarte um byte e tente novamente
            self.ser.read(1)
            return None

    # Desempacota os dados (usando os 8 bytes restantes)
        try:
            roll, pitch, yaw, dev = struct.unpack('>hhhh', dados[1:])
            return [v / SCALE_FACTOR for v in (roll, pitch, yaw, dev)]
        except struct.error as e:
            print(f"Erro no desempacotamento: {e}")
            return None

class InitialScreen(tk.Frame):
    def __init__(self, root, start_callback):
        super().__init__(root)
        self.start_callback = start_callback

        self.depth_var = tk.StringVar()
        self.side_var  = tk.StringVar()

        innerFrame = tk.Frame(self)
        innerFrame.pack(expand=True)

        innerFrame.grid_columnconfigure(0, weight=1)
        innerFrame.grid_columnconfigure(1, weight=1)

        ttk.Label(innerFrame, text="Digite a profundidade (em metros): \n(Para número com virgula, use .)").grid(row=1, column=0, sticky="w", padx=5, pady=5)
        self.depth_entry = ttk.Entry(innerFrame, textvariable=self.depth_var, width=20)
        self.depth_entry.grid(row=1, column=1, padx=5, pady=5)

        ttk.Label(innerFrame, text="Digite a face do mergulho: \n(NOTA: isso será usado no nome do arquivo)").grid(row=2, column=0, sticky="n", padx=5, pady=5)
        self.side_entry = ttk.Entry(innerFrame, textvariable=self.side_var, width=20)
        self.side_entry.grid(row=2, column=1, padx=5, pady=5)

        tk.Button(innerFrame, text="Confirmar e continuar", command=self.on_continue).grid(row=3, column=0, columnspan=2, pady=10)

        self.pack(expand=True)

    def on_continue(self):
        try:
            depth = float(self.depth_var.get())
            side  = self.side_var.get().strip()
            if not side:
                answer = askretrycancel("AVISO!", "A face de mergulho não pode ser vazia, tente novamente")
                if answer:
                    self.side_entry.delete(0, tk.END)
                return
            
            try:
                ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
                ser.close()
            except SerialException:
                answer = showinfo("ERRO!", f"Não foi possivel acessar a porta {SERIAL_PORT}")
                self.master.destroy()
                return
            self.destroy()
            self.start_callback(depth, side)

        except ValueError: # erro no valor de profundidade
            answer = askretrycancel("AVISO!", "Profundiade inválida, tente novamente")
            if answer:
                self.depth_entry.delete(0, tk.END)
                self.side_entry.delete(0, tk.END)

class DynamicPlotApp(tk.Frame):
    def __init__(self, root, depth, side):
        super().__init__(root)
        self.root = root
        self.depth_initial = depth
        self.side = side
        label = tk.Label(self, text="Tela de Graficos")
        label.grid(row=0, column=0)
        self.serial_reader = SerialReader(SERIAL_PORT, BAUD_RATE)

        self.stop_event = threading.Event()
        self.lock = threading.Lock()

        # inicializa as variáveis
        self.setup_variables()
        self.create_widgets()
        self.setup_plots()

    def create_widgets(self): # para criar os elementos em tela
        top_frame = ttk.Frame(self.root)
        top_frame.grid(row=0, column=0, columnspan=4, sticky="ew", padx=5, pady=5)

        top_frame.columnconfigure(0, weight=1)
        top_frame.columnconfigure(1, weight=0)
        top_frame.columnconfigure(2, weight=1)
        top_frame.columnconfigure(3, weight=1)

        self.start_read_button = ttk.Button(top_frame, text="Iniciar Leitura e Gravação", command=self.read_and_record)
        self.start_read_button.grid(row=0, column=0, padx=5, pady=5, sticky="w")

        self.repeat_button = ttk.Button(top_frame, text="Repetir Última gravação", command=self.repeat)
        self.repeat_button.grid(row=0, column=1, padx=5, pady=5, sticky="e")

        self.depth_label = ttk.Label(top_frame, text=f'Medindo: {self.depth[self.indice]:.1f} metros')
        self.depth_label.grid(row=0, column=2, padx=5, pady=5, sticky="w")

        self.status_label = ttk.Label(top_frame, text="Pronto")
        self.status_label.grid(row=0, column=3, padx=5, pady=5)

        self.close_button = ttk.Button(top_frame, text="Fechar programa", command=self.on_close)
        self.close_button.grid(row=0, column=4, padx=20, pady=5, sticky="e")

    def setup_variables(self):
        self.indice = 0
        self.recording = False
        self.paused = False
        self.active_colect = False
        self.plotting_active = False
        self.save_on_exit = True
        self.collected_data = []
        self.all_data = []
        self.depth = [self.depth_initial - i*0.5 for i in range(int(self.depth_initial / 0.5) + 1)] # criar floats de DEPTH até 0

        self.depth_data  = deque(maxlen=MAX_DATA_POINTS) # eixo x dos gráficos
        self.roll_data  = deque(maxlen=MAX_DATA_POINTS) # eixo x
        self.pitch_data = deque(maxlen=MAX_DATA_POINTS) # eixo y
        self.yaw_data   = deque(maxlen=MAX_DATA_POINTS) # eixo z
        self.dev_data   = deque(maxlen=MAX_DATA_POINTS) # desvio/desviation total

    def setup_plots(self): # para configurar e ajustar os plots
        self.fig, self.axes = plt.subplots(2, 2, figsize=(10,8))
        (self.ax_roll, self.ax_pitch, self.ax_yaw, self.ax_dev) = self.axes.flatten()
        for ax in [self.ax_roll, self.ax_pitch, self.ax_yaw, self.ax_dev]:
            ax.grid(True)
            ax.set_xlabel('Profundidade (m)')
            ax.set_xlim(self.depth_initial, 0) # Profunidade máxima

        self.ax_roll.set_ylabel ('Ângulo (°)')
        self.ax_pitch.set_ylabel('Ângulo (°)')
        self.ax_yaw.set_ylabel  ('Ângulo (°)')
        self.ax_dev.set_ylabel  ('Desvio (°)')

        self.ax_roll.set_title ('Dados dos Ângulos de X')
        self.ax_pitch.set_title('Dados dos Ângulos do Y')
        self.ax_yaw.set_title  ('Dados dos Ângulos do Z')
        self.ax_dev.set_title  ('Desvio dos Ângulos')

        self.line_roll  = self.ax_roll.plot ( [], [], color="blue",    marker="D")[0]
        self.line_pitch = self.ax_pitch.plot( [], [], color="red",     marker="D")[0]
        self.line_yaw   = self.ax_yaw.plot  ( [], [], color="green",   marker="D")[0]
        self.line_dev   = self.ax_dev.plot  ( [], [], color="magenta", marker="D")[0]

        self.fig.tight_layout(pad=3.0)

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().grid(row=1, column=0, columnspan=2)

    def read_and_record(self):
        if not self.active_colect:
            self.active_colect = True
            self.serial_reader.set_active(True) # ativa a leitura serial

            self.collected_data = []  # Limpa a lista de dados coletados
            self.serial_reader.ser.write(b"S")  # Envia o comando para o dispositivo

            self.start_read_button.config(text="Coletando dados...", state=tk.DISABLED)
            self.repeat_button.config(state=tk.DISABLED)
            threading.Thread(target=self._thread_colect, daemon=True).start()

    def _thread_colect(self):
        try:
            received_data = []
            current_depth = float(self.depth[self.indice])
            timeout = time.time() + 2
            attempts = 0

            self.serial_reader.set_active(True)
            self.serial_reader.ser.reset_input_buffer()

            while len(received_data) < 11 and time.time() < timeout:
                pacote = self.serial_reader.read_data()
                attempts += 1 # para contar a quantidade caso de erro
                if pacote:
                    received_data.append(pacote)
                    self.all_data.append((current_depth, *pacote))

                    # Atualiza status na interface
                    self.root.after(0, lambda c=len(received_data): self.status_label.config(text=f'Coletado {c}/11'))
                    time.sleep(0.01) # para evitar sobrecarga
            if len(received_data) == 11:
                last_packet = received_data[-1] # pra pegar o ultimo pacote
                with self.lock:
                    self.depth_data.append(current_depth)
                    self.roll_data.append(last_packet[0])
                    self.pitch_data.append(last_packet[1])
                    self.yaw_data.append(last_packet[2])
                    self.dev_data.append(last_packet[3])

                self.root.after(0, self._update_plots)
                self.indice += 1 # se deu certo, vai para a próxima DEPTH 

                if current_depth == 0: # chegou no topo
                    self.root.after(5000, self.collectd_done)
                    return
            else:
                self.root.after(0, lambda: self.status_label.config(text=f"Coleta interrompida {len(received_data)}/11"))
                self.all_data = [data for data in self.all_data if data[0] != current_depth]
            time.sleep(3) # para ver a mensagem
            self.root.after(0, lambda: self.status_label.config(text="Pronto"))
        finally:
            if current_depth != 0: # só vai para a próxima se não chegou em 0m
                self.serial_reader.set_active(False)
                self.active_colect = False
                self.start_read_button.config(text="Iniciar Leitura e Gravação", state=tk.NORMAL) # para voltar o texto original
                self.repeat_button.config(state=tk.NORMAL) # para ativar novamente
                self.depth_label.config(text=f"Medindo: {self.depth[self.indice]:.1f} metros") # atualiza o self.depth

    def repeat(self):
        if self.indice == 0: # se apertar sem rodar antes
            self.status_label.config(text="Nada para repetir ainda")
            self.start_read_button.config(text="Nenhum dado salvo", state=tk.DISABLED) # para travar a gravação
            self.repeat_button.config(state=tk.DISABLED) # para travar a gravação
            self.root.after(3000, self._reset_status_labels) # para esperar 3 segundos e depois atualizar os textos
            return

        if self.depth_data:
            self.status_label.config(text="Apagando dados")
            removed_depth = self.depth_data.pop()
            self.all_data = [d for d in self.all_data if d[0] != removed_depth]
            self.roll_data.pop()
            self.pitch_data.pop()
            self.yaw_data.pop()
            self.dev_data.pop()

            self.current_depth = removed_depth # atualiza a profundidade para manter
            self.indice -= 1

            self._update_plots()
            self.depth_label.config(text=f"Medindo {self.depth[self.indice]:.1f} metros")
            self.status_label.config(text="Pronto para repetir última medição")

    def _reset_status_labels(self):
        self.start_read_button.config(text="Iniciar Leitura e Gravação", state=tk.ACTIVE)
        self.repeat_button.config(state=tk.ACTIVE)
        self.status_label.config(text="Pronto")
                
    def _update_plots(self):
        try:
            if not self.depth_data or not self.roll_data:
                return
            
            depth = list(self.depth_data)
            roll  = list(self.roll_data)
            pitch = list(self.pitch_data)
            yaw   = list(self.yaw_data)
            dev   = list(self.dev_data)

            self.line_roll.set_data (depth, roll)
            self.line_pitch.set_data(depth, pitch)
            self.line_yaw.set_data  (depth, yaw)

            self.line_dev.set_data(depth, dev)

            self.ax_roll.relim()
            self.ax_pitch.relim()
            self.ax_yaw.relim()
            self.ax_dev.relim()
            self.ax_roll.autoscale_view (True, True, True)
            self.ax_pitch.autoscale_view(True, True, True)
            self.ax_yaw.autoscale_view  (True, True, True)
            self.ax_dev.autoscale_view  (True, True, True)

            self.canvas.draw_idle() # para atualizar o canvas
        except Exception as e:
            print(f"Erro ao atulizar os plots: {e}")

    def _update_plots_thread(self):
        print("Thread de plot iniciada!") 
        while not self.stop_event.is_set():
            if self.plotting_active and not self.paused:
                if hasattr(self.serial_reader, 'active') and self.serial_reader.active:
                    data = self.serial_reader.read_data()
                    if data:
                        with self.lock:
                            self.depth_data.append(float(self.depth[self.indice]))
                            self.roll_data.append(data[0])
                            self.pitch_data.append(data[1])
                            self.yaw_data.append(data[2])
                            self.dev_data.append(data[3])

                        self.root.after(0, self._update_plots)
                time.sleep(0.05)

    def collectd_done(self):
        self.serial_reader.set_active(False)
        self.active_colect = False
        self.start_read_button.config(text="Coleta Finalizada", state=tk.DISABLED)
        self.depth_label.config(text=f"Terminou os {self.depth_initial}m")

        if self.all_data:
            self.save_on_exit = False
            self.save_data()

        self.root.after(5000, self.end_all) # fecha após 5 segundos

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
                filename = os.path.join(SAVE_FOLDER, f"dados_{datetime.now().strftime('%Y%m%d_%H%M')}_{self.side}.csv")
            with open(filename, 'w', newline='', encoding='utf-8') as file:
                writer = csv.writer(file)
                writer.writerow(['DEPTH', 'Roll', 'Pitch', 'Yaw', 'Deviation'])
                for i, (depth, roll, pitch, yaw, dev) in enumerate(self.all_data):
                    writer.writerow([depth, roll, pitch, yaw, dev])
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
        self.serial_reader.set_active(False)

        if self.save_on_exit and self.all_data:
            self.save_on_exit = False
            self.save_data()
        else:
            print("Nenhum dado para ser salvo")
            self.end_all() # fecha a tela se não houver dados

class MainApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Sistema de Monitoramento do Inclinometro")

        container = tk.Frame(self)
        container.pack(fill="both", expand=True)

        self.screen = {}

        self.screen["inicio"] = InitialScreen(container, lambda: self.show_screen("graficos"))
        self.screen["inicio"].grid(row=0, column=0, sticky="nsew")

        self.screen["graficos"] = DynamicPlotApp(container)
        self.screen["graficos"].grid(row=0, column=0, sticky="nsew")

        self.show_screen("inicio")

    def show_screen(self, screen_name):
        screen = self.screen.get(screen_name)
        if screen:
            screen.tkraise()

if __name__ == "__main__":
    def start_main_app(depth, side):
        for widget in root.winfo_children():
            widget.destroy()
        app = DynamicPlotApp(root, depth, side)
        app.grid(row=0, column=0)

        # Caso seja fechado clicando no "X"
        root.protocol("WM_DELETE_WINDOW", app.on_close)
    
    root = tk.Tk()
    root.title("Leitura Serial com Gráficos")

    initial_screen = InitialScreen(root, start_main_app)
    initial_screen.grid(row=0, column=0)   

    root.mainloop()