from tkinter import *

import serial
import matplotlib.pyplot as plt


def EntrarClick ():
    if fraseEntry.get()=='1'or fraseEntry.get()=='Be'or fraseEntry.get()=='be':
        print("M'alegro que estiguis be!!")
    elif fraseEntry.get()=="0" or fraseEntry.get()=='Malament'or fraseEntry.get()=='malament':
        print("OHHH, em sap greu, espero que et posis content en breus")
    else:
        #print ('Has introducido la frase --- ' + fraseEntry.get() + ' --- y has pulsado el botón entrar')
        print ('HOLA, COM ESTÀS?')

def AClick ():
    device = 'COM7'
    mySerial = serial.Serial(device, 9600, timeout=1)

    plt.ion()
    fig, ax = plt.subplots()

    ax.set_xlabel("Muestras")
    ax.set_ylabel("Temperatura (°C)")
    ax.set_title("TEMPERATURA DEL SATÉLITE EN TIEMPO REAL")

    temperaturas = []
    eje_x = []

    # Crear línea vacía al principio
    linea, = ax.plot([], [], 'r-')  # Línea roja sin puntos

    i = 0

    try:
        while True:
            if mySerial.in_waiting > 0:
                linea_serial = mySerial.readline().decode('utf-8').strip()

                try:
                    trozos = linea_serial.split(':')
                    temperatura = float(trozos[1])
                except (IndexError, ValueError):
                    continue

                eje_x.append(i)
                temperaturas.append(temperatura)

                # Actualizar los datos de la línea
                linea.set_data(eje_x, temperaturas)

                # Ajustar los límites de los ejes
                ax.set_xlim(0, max(100, i))
                ax.set_ylim(min(temperaturas) - 1, max(temperaturas) + 1)

                plt.draw()
                plt.pause(0.1)
                i += 1

    except KeyboardInterrupt:
        print("Saliendo...")

    finally:
        mySerial.close()

def BClick ():
    print ('Has pulsado el boton PARAR')

def CClick ():
    print ('Has pulsado el boton REANUDAR')

def DClick ():
    print ('Has pulsado el boton SALIR')

window = Tk()
window.geometry("400x400")
window.rowconfigure(0, weight=1)
window.rowconfigure(1, weight=1)
window.rowconfigure(2, weight=1)
window.columnconfigure(0, weight=1)
window.columnconfigure(1, weight=1)
window.columnconfigure(2, weight=1)
window.columnconfigure(3, weight=1)

tituloLabel = Label(window, text = "Mi programa", font=("Courier", 20, "italic"))
tituloLabel.grid(row=0, column=0, columnspan=5, padx=5, pady=5, sticky=N + S + E + W)

fraseEntry = Entry(window)
fraseEntry.grid(row=1, column=0, columnspan = 3, padx=5, pady=5, sticky=N + S + E + W)

EntrarButton = Button(window, text="Entrar", bg='red', fg="white",command=EntrarClick)
EntrarButton.grid(row=1, column=3, padx=5, pady=5, sticky=N + S + E + W)

AButton = Button(window, text="INICIAR", bg='red', fg="white",command=AClick)
AButton.grid(row=2, column=0, padx=5, pady=5, sticky=N + S + E + W)
BButton = Button(window, text="PARAR", bg='yellow', fg="black",command=BClick)
BButton.grid(row=2, column=1, padx=5, pady=5, sticky=N + S + E + W)
CButton = Button(window, text="REANUDAR", bg='blue', fg="white",command=CClick)
CButton.grid(row=2, column=2, padx=5, pady=5, sticky=N + S + E + W)
DButton = Button(window, text="SALIR", bg='orange', fg="black",command=DClick)
DButton.grid(row=2, column=3, padx=5, pady=5, sticky=N + S + E + W)

window.mainloop()
