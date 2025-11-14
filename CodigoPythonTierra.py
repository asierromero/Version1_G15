from tkinter import *
from tkinter import messagebox
import tkinter as tk
import serial
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading
import time

# Variable global para controlar la recepción
recepcion_activa = True
mySerial = None

def INICIARClick():
    global mySerial, recepcion_activa
    device = 'COM8'
    try:
        mySerial = serial.Serial(device, 9600, timeout=1)
    except serial.SerialException:
        messagebox.showerror("Error", "No se pudo abrir el puerto COM7")
        return

    recepcion_activa = True  # activamos la recepción

    # Crear figura
    fig = Figure(figsize=(6, 4), dpi=100)
    ax = fig.add_subplot(111)
    ax.set_xlabel("Muestras")
    ax.set_ylabel("Temperatura (°C)")
    ax.set_title("TEMPERATURA DEL SATÉLITE EN TIEMPO REAL")

    temperaturas = []
    eje_x = []
    linea, = ax.plot([], [], 'r-')

    # Integrar figura en Tkinter
    canvas = FigureCanvasTkAgg(fig, master=picture_frame)
    canvas.draw()
    canvas_widget = canvas.get_tk_widget()
    canvas_widget.config(width=600, height=400)
    canvas_widget.grid(row=0, column=0, padx=5, pady=5, sticky=N+E+W+S)

    i = 0

    # Función que se ejecuta en hilo para recibir datos
    def recepcion():
        nonlocal i
        global recepcion_activa
        while True:
            if mySerial.in_waiting > 0 and recepcion_activa:
                linea_serial = mySerial.readline().decode('utf-8').strip()
                try:
                    trozos = linea_serial.split(':')
                    temperatura = float(trozos[1])  # T:xx:H:yy
                except (IndexError, ValueError):
                    continue

                eje_x.append(i)
                temperaturas.append(temperatura)
                linea.set_data(eje_x, temperaturas)
                ax.set_xlim(0, max(100, i))
                ax.set_ylim(min(temperaturas)-1, max(temperaturas)+1)
                canvas.draw()
                i += 1
            time.sleep(0.1)  # evita saturar CPU

    # Crear hilo
    hilo = threading.Thread(target=recepcion, daemon=True)
    hilo.start()

def PAUSARClick():
    global mySerial 
    if mySerial:
        mensaje = "Parar\n"
        mySerial.write(mensaje.encode('utf-8'))
    print("Has pulsado PARAR")

def REANUDARClick():
    global mySerial 
    if mySerial:
        mensaje = "Reanudar\n"
        mySerial.write(mensaje.encode('utf-8'))
    print("Has pulsado REANUDAR")

def SALIRClick():
    global mySerial
    if mySerial:
        mySerial.close()
    window.destroy()

# --- INTERFAZ ---
window = Tk()
window.geometry("1000x600")
window.rowconfigure(0, weight=1)
window.rowconfigure(1, weight=1)
window.rowconfigure(2, weight=1)
window.rowconfigure(3, weight=1)
window.rowconfigure(4, weight=1)
window.rowconfigure(4, weight=1)
window.columnconfigure(0, weight=1)
window.columnconfigure(1, weight=4)

tituloLabel = Label(window, text="Mi programa", font=("Courier", 20, "italic"))
tituloLabel.grid(row=0, column=0, columnspan=1, padx=5, pady=5, sticky=N + S + E + W)

botones_frame = tk.LabelFrame(window, text='Botones')
botones_frame.grid(row=1, column=0, padx=5, pady=5, sticky=tk.N + tk.S + tk.E + tk.W)

AButton = Button(window, text="Grafica de Temperatura", bg='red', fg="white", command=INICIARClick)
AButton.grid(row=1, column=0, padx=5, pady=5, sticky=N + S + E + W)

BButton = Button(window, text="PARAR", bg='yellow', fg="black", command=PAUSARClick)
BButton.grid(row=2, column=0, padx=5, pady=5, sticky=N + S + E + W)

CButton = Button(window, text="REANUDAR", bg='blue', fg="white", command=REANUDARClick)
CButton.grid(row=3, column=0, padx=5, pady=5, sticky=N + S + E + W)

DButton = Button(window, text="SALIR", bg='orange', fg="black", command=SALIRClick)
DButton.grid(row=4, column=0, padx=5, pady=5, sticky=N + S + E + W)

#RadarButton = Button(window, text="Radar", bg='green', fg="black", command=RADARClick)
#RadarButton.grid(row=4, column=0, padx=5, pady=5, sticky=N + S + E + W)

# Frame para la gráfica
picture_frame = tk.LabelFrame(window, text="Gráficos")
picture_frame.grid(row=1, column=1, rowspan=4, padx=5, pady=5, sticky=tk.N + tk.S + tk.E + tk.W)

window.mainloop()
