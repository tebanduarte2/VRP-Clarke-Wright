import matplotlib.pyplot as plt
import numpy as np
import csv
from matplotlib.colors import ListedColormap
import random

def load_routes_from_csv(filename):
    """
    Carga las rutas desde el archivo CSV generado por el programa C++
    """
    routes = []
    current_route = []
    
    try:
        with open(filename, 'r') as file:
            for line in file:
                line = line.strip()
                
                # Si es un comentario de nueva ruta
                if line.startswith('# Ruta vehiculo'):
                    # Si hay una ruta anterior, la guardamos
                    if current_route:
                        routes.append(current_route)
                    current_route = []
                    continue
                
                # Si es una línea con coordenadas
                if ',' in line:
                    try:
                        parts = line.split(',')
                        if len(parts) >= 3:
                            x, y, node_id = float(parts[0]), float(parts[1]), int(parts[2])
                            current_route.append((x, y, node_id))
                        else:
                            x, y = float(parts[0]), float(parts[1])
                            current_route.append((x, y, -1))  # ID desconocido
                    except ValueError:
                        continue
            
            # Agregar la última ruta
            if current_route:
                routes.append(current_route)
                
    except FileNotFoundError:
        return []
    except Exception as e:
        return []
    
    return routes

def generate_colors(num_routes):
    """
    Genera una lista de colores distintos para las rutas
    """
    # Colores predefinidos para las primeras rutas
    base_colors = [
        '#FF0000', '#00FF00', '#0000FF', '#FFFF00', '#FF00FF', '#00FFFF',
        '#FFA500', '#800080', '#008000', '#FFC0CB', '#A52A2A', '#808080',
        '#000080', '#008080', '#800000', '#808000', '#FF6347', '#4682B4',
        '#9ACD32', '#FF1493', '#DC143C', '#00CED1', '#FF8C00', '#9932CC'
    ]
    
    colors = []
    
    # Usar colores predefinidos primero
    for i in range(min(num_routes, len(base_colors))):
        colors.append(base_colors[i])
    
    # Si necesitamos más colores, generar aleatoriamente
    for i in range(len(base_colors), num_routes):
        # Generar color hexadecimal aleatorio
        color = "#{:06x}".format(random.randint(0, 0xFFFFFF))
        colors.append(color)
    
    return colors

def plot_routes(routes, title="Rutas del Problema de Enrutamiento de Vehículos"):
    """
    Dibuja todas las rutas con diferentes colores
    """
    if not routes:
        return
    
    plt.figure(figsize=(16, 12))  # Aumentado de (12, 10) a (16, 12)
    
    # Generar colores para cada ruta
    colors = generate_colors(len(routes))
    
    # Variables para ajustar los límites del gráfico
    all_x = []
    all_y = []
    
    # Dibujar cada ruta
    for i, route in enumerate(routes):
        if len(route) < 2:
            continue
            
        # Extraer coordenadas x, y y IDs
        x_coords = [point[0] for point in route]
        y_coords = [point[1] for point in route]
        ids = [point[2] if len(point) > 2 else -1 for point in route]
        
        all_x.extend(x_coords)
        all_y.extend(y_coords)
        
        # Dibujar la ruta
        plt.plot(x_coords, y_coords, color=colors[i], linewidth=2, 
                marker='o', markersize=6, label=f'Ruta {i+1}')
        
        # Añadir etiquetas de ID a cada punto
        for j, (x, y, node_id) in enumerate(zip(x_coords, y_coords, ids)):
            if node_id != -1:
                # Offset más grande para mejor separación visual
                offset_x = 1.2  # Aumentado de 0.5 a 1.2
                offset_y = 1.2  # Aumentado de 0.5 a 1.2
                plt.annotate(str(node_id), (x, y), xytext=(x + offset_x, y + offset_y),
                           fontsize=9, fontweight='bold',  # Aumentado de fontsize=8 a 9
                           bbox=dict(boxstyle="round,pad=0.3", facecolor='white', alpha=0.8))  # Más padding
        
        # Marcar el punto de inicio (depósito) de cada ruta
        if len(route) > 0:
            plt.plot(x_coords[0], y_coords[0], color=colors[i], 
                    marker='s', markersize=10, markeredgecolor='black', markeredgewidth=1)
    
    # Marcar el depósito principal (primer punto de la primera ruta)
    if routes and len(routes[0]) > 0:
        depot_x, depot_y = routes[0][0][0], routes[0][0][1]
        plt.plot(depot_x, depot_y, color='red', marker='D', markersize=15, 
                markeredgecolor='black', markeredgewidth=2, label='Depósito')
    
    # Configurar el gráfico
    plt.xlabel('Coordenada X', fontsize=12)
    plt.ylabel('Coordenada Y', fontsize=12)
    plt.title(title, fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    # Ajustar límites del gráfico con más espacio
    if all_x and all_y:
        margin_x = (max(all_x) - min(all_x)) * 0.01 # Aumentado de 0.1 a 0.25
        margin_y = (max(all_y) - min(all_y)) * 0.01  # Aumentado de 0.1 a 0.25
        plt.xlim(min(all_x) - margin_x, max(all_x) + margin_x)
        plt.ylim(min(all_y) - margin_y, max(all_y) + margin_y)
    
    plt.tight_layout()
    return plt

def print_route_summary(routes):
    total_points = 0
    for i, route in enumerate(routes):
        # Restar 1 porque el primer punto es el depósito
        customers_in_route = len(route) - 1 if len(route) > 1 else 0
        total_points += customers_in_route

def main():
    # Nombre del archivo CSV generado por el programa C++
    csv_filename = "solucion_rutas.csv"
    
    # Cargar las rutas
    routes = load_routes_from_csv(csv_filename)
    
    if not routes:
        return
    
    # Procesar datos sin mostrar
    print_route_summary(routes)
    
    # Visualizar las rutas
    plt_obj = plot_routes(routes, "Solución CVRP - Algoritmo Clarke-Wright")
    
    if plt_obj:
        # Guardar la imagen directamente
        plt_obj.savefig('Grafica1.png', dpi=300, bbox_inches='tight')
        plt_obj.close()  # Cerrar la figura para liberar memoria

if __name__ == "__main__":
    main()