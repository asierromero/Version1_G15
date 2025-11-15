'''
import serial
import matplotlib.pyplot as plt
import numpy as np

# --- CONFIGURACIÓN DEL PUERTO SERIAL ---
device = 'COM5'  # Cambia si usas otro puerto
mySerial = serial.Serial(device, 9600, timeout=1)

# --- CONFIGURACIÓN DEL RADAR ---
plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111, polar=True)
ax.set_theta_zero_location('E')  # 0° a la derecha
ax.set_theta_direction(-1)       # Ángulos en sentido horario
ax.set_thetamin(0)
ax.set_thetamax(180)
ax.set_rmax(50)                  # Máxima distancia (ajústala a tu sensor)
ax.set_title("Radar de Ultrasonido", va='bottom')

# Línea del radar y puntos
linea, = ax.plot([], [], color='yellow')
punto, = ax.plot([], [], 'go')

angulos = []
distancias = []

try:
    while True:
        if mySerial.in_waiting > 0:
            data = mySerial.readline().decode('utf-8').strip()
            if ':' not in data:
                continue

            try:
                angulo_str, dist_str = data.split(':')
                angulo = float(angulo_str)
                distancia = float(dist_str)
            except ValueError:
                continue

            # Convertir ángulo a radianes
            theta = np.deg2rad(angulo)

            # Almacenar y mostrar
            angulos.append(theta)
            distancias.append(distancia)

            # Actualizar línea y punto
            linea.set_data(angulos, distancias)
            punto.set_data([theta], [distancia])

            plt.pause(0.01)

except KeyboardInterrupt:
    print("Programa terminado.")
    mySerial.close()
    '''

import serial
import matplotlib.pyplot as plt
import numpy as np
import time

# --- CONFIGURACIÓN DEL PUERTO SERIAL ---
device = 'COM5'  # Cambia según tu puerto
mySerial = serial.Serial(device, 9600, timeout=1)

# --- CONFIGURACIÓN DEL RADAR ---
plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111, polar=True)
ax.set_theta_zero_location('E')  # 0° a la derecha
ax.set_theta_direction(-1)       # sentido horario
ax.set_thetamin(0)
ax.set_thetamax(180)
ax.set_rmax(50)                  # rango máximo
ax.set_title("Radar de Ultrasonido", va='bottom')

# Elementos gráficos
linea_radar, = ax.plot([], [], 'g-', linewidth=2)   # barra verde
punto, = ax.plot([], [], 'go', markersize=8)        # punto verde
estela, = ax.plot([], [], 'y-', linewidth=1, alpha=0.6)  # rastro amarillo

# Datos
puntos = []  # [(theta, distancia, tiempo)]

DURACION_ESTELA = 2.0  # segundos visibles antes de borrarse

try:
    while True:
        if mySerial.in_waiting > 0:
            data = mySerial.readline().decode('utf-8').strip()
            if ':' not in data:
                continue

            try:
                angulo_str, dist_str = data.split(':')
                angulo = float(angulo_str)
                distancia = float(dist_str)
            except ValueError:
                continue

            theta = np.deg2rad(angulo)
            puntos.append((theta, distancia, time.time()))

            # --- limpiar puntos antiguos ---
            ahora = time.time()
            puntos = [(th, d, t) for (th, d, t) in puntos if ahora - t < DURACION_ESTELA]

            # --- actualizar elementos ---
            if puntos:
                # estela amarilla
                thetas, distancias, _ = zip(*puntos)
                estela.set_data(thetas, distancias)

                # barra verde hasta el punto actual
                linea_radar.set_data([0, theta], [0, distancia])
                punto.set_data([theta], [distancia])

            plt.pause(0.01)

except KeyboardInterrupt:
    print("Programa terminado.")
    mySerial.close()
