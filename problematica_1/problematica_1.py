from mpi4py import MPI
import numpy as np
from PIL import Image
import sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

#Funcion para calculo de metricas
def metricas(nombre_canal, array_original, array_procesado):
    #Esta funcion calcula e imprime suma, media y desviacion estandar
    print(f"\n Métricas para el canal {nombre_canal}")
    print(f"Métrica        | Canal Original         | Canl Procesado")
    print(f"---------------------------------------------------------")
    print(f"Suma           | {np.sum(array_original):<14.2e} | {np.sum(array_procesado):<14.2e}")
    print(f"Media          | {np.mean(array_original):<14.2f} | {np.mean(array_procesado):<14.2f}")
    print(f"Desv. Estándar | {np.std(array_original):<14.2f} | {np.std(array_procesado):<14.2f}")
#Función para comparar gráficos
def comparacion(nombre_canal, array_original, array_procesado):
    #Esta  función sirve para crear un gráfico del antes y el después
    plt.figure(figsize=(12,10))
    plt.suptitle(f"Comparación Antes/Después - Canal {nombre_canal}", fontsize=16)
    #Imagen original
    plt.subplot(2,2,1)
    plt.imshow(array_original, cmap='gray', vmin=0, vmax=255)
    plt.title("Original")
    plt.axis('off')
   #imagen procesada
    plt.subplot(2,2,2)
    plt.imshow(array_procesado, cmap='gray', vmin=0, vmax=255)
    plt.title("Procesado (Igualación)")
    plt.axis('off')
    #Histograma original
    plt.subplot(2,2,3)
    plt.hist(array_original.ravel(), 256, [0,256], color='blue', alpha=0.7)
    plt.title("Histograma original")
    plt.xlim([0,256])
    #Histograma procesado
    plt.subplot(2,2,4)
    plt.hist(array_procesado.ravel(), 256, [0,256], color='blue', alpha=0.7)
    plt.title("Histograma procesado")
    plt.xlim([0,256])
    plt.tight_layout(rect=[0,0.03,1,0.95])
    #Guardar la figura en un archivo
    archivo= f"comparacion_{nombre_canal}.png"
    plt.savefig(archivo)
    print(f"Gráfico de {nombre_canal} guardado")
    plt.close()
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

print(f"Soy el proceso {rank} de un total de {size} procesos")

if rank == 0:
    print(f"Rank {rank}: Hola. Soy el master")
    if size !=4:
        print(f"Se esperaban 4 procesos. Recibí {size}")
        comm.Abort()
    try:
        #Cargar la imagen
        img_original = Image.open("imagen_prueba.jpg")
        print(f"Master con Rank {rank}: Imagen 'imagen_prueba.jpg' cargada")
        #Poner la imagen en RGB
        img_rgb = img_original.convert('RGB')
        #Obtener los metadatos (dimensiones)
        ancho, alto = img_rgb.size
        print(f"Master con Rank {rank}: Dimensiones de la imagen: {ancho}x{alto}")
        #Separar en 3 canales
        canal_red, canal_green, canal_blue = img_rgb.split()
        print(f"Master con Rank {rank}: Canales R, G ,B separados")
        #Convertimos cada canal en un array de tipo uint8
        array_r = np.array(canal_red, dtype=np.uint8)
        array_g = np.array(canal_green, dtype=np.uint8)
        array_b = np.array(canal_blue, dtype=np.uint8)
        tiempo_inicio=MPI.Wtime()
 
        print(f"Master con Rank {rank}: Canales convertidos a arrays")
        print(f"Master con Rank {rank}: Array del canal R: {array_r.shape}")
        print(f"Master con Rank {rank}: Array del canal G: {array_g.shape}")
        print(f"Master con Rank {rank}: Array del canal B: {array_b.shape}")
        #crear metadatos
        metadatos = {'alto': alto, 'ancho':ancho}
        #Enviar los metadatos a los workers
        comm.send(metadatos, dest=1)
        comm.send(metadatos, dest=2)
        comm.send(metadatos, dest=3)
        print("Master: Metadatos enviados a los workers")
        print("Master: Enviando canal R a worker 1...")
        comm.send(array_r, dest=1)
        print("Master: Enviando canal G a worker 2...")
        comm.send(array_g, dest=2)
        print("Master: Enviando canala B a worker 3...")
        comm.send(array_b, dest=3)
        print("Master: Todos los arrays enviados")
        #Recolectar los datos de los workers
        print(f"Master: Esperando los arrays procesados")
        canal_r_procesado = comm.recv(source=1)
        print("Master: Canal R procesado recibido")
        canal_g_procesado = comm.recv(source=2)
        print("Master: Canal G procesado recibido")
        canal_b_procesado = comm.recv(source=3)
        print("Master: Canal B procesado recibido")
        print("Master: Todos los canales procesados recibidos")
        tiempo_final=MPI.Wtime()
        #Transformar los canales a imagenes
        img_r_procesada= Image.fromarray(canal_r_procesado, 'L')
        img_g_procesada= Image.fromarray(canal_g_procesado, 'L')
        img_b_procesada= Image.fromarray(canal_b_procesado, 'L')
        print("Master: Canales convertidos a imagen")
        #Unir los 3 canales en una sola imagen
        img_final= Image.merge('RGB', (img_r_procesada, img_g_procesada, img_b_procesada))
        print("Master: Canales fusionados en una sola imagen")
        #Guardar la imagen final
        img_final.save('imagen_procesada.png')
        print("Master: La imagen procesada ha sido guardada")
        #Calcular tiempo
        tiempo_total_paralelo = tiempo_final - tiempo_inicio
        print(f"Tiempo total del proceso en paralelo: {tiempo_total_paralelo:.6f} segundos")
        #Verificación
        #Imprimir métricas
        metricas("Rojo", array_r, canal_r_procesado)
        metricas("Verde", array_g, canal_g_procesado)
        metricas("Azul", array_b, canal_b_procesado)
        #Antes y despues de los histogramas
        print("\nMaster: Generando gráficos de comparación")
        comparacion("Rojo", array_r, canal_r_procesado)
        comparacion("Verde", array_g, canal_g_procesado)
        comparacion("Azul", array_b, canal_b_procesado)
        #Comprobar dimensiones
        if img_final.size == img_rgb.size and img_final.mode == 'RGB':
            print(f"Master: Verificación de dimensiones y modo con éxito")
            print(f"Original: {img_rgb.size}, Modo: {img_rgb.mode}")
            print(f"Final: {img_final.size}, Modo: {img_final.mode}")
        else:
            print("\nMaster: Las dimensiones o el modo no coinciden")
        print("Verificacion completada")
    except FileNotFoundError:
        print(f"Master con Rank {rank}: No se encontró el archivo")
        comm.Abort()
else:
    print(f"Soy el worker con Rank {rank}")
    #Recibir los metadatos por parte del master
    metadatos = comm.recv(source=0)
    print(f"Worker {rank}: Metadatos recibidos (Alto: {metadatos['alto']})")
    #recibir el array
    array_recibido = comm.recv(source=0)
    print(f"Worker {rank}: Forma del array recibido: {array_recibido.shape}")
    print(f"Worker {rank}: Iniciando cálculo de histograma...")
    
    #Calcular histograma. Con .ravel() reducimos de dos dimensiones a una
    hist, bins = np.histogram(array_recibido.ravel(), 256, [0,256])
    #Calcular CDF
    cdf = hist.cumsum()
    #Calcular LUT. Para esto encontremos CDF min, y con np.ma ignoramos los valores cero
    cdf_mask = np.ma.masked_equal(cdf,0)
    cdf_min = cdf_mask.min()
    #total de pixeles
    total_pixeles = metadatos['alto']*metadatos['ancho']
    #Aplicar LUT
    lut = np.round(((cdf-cdf_min) / (total_pixeles - cdf_min))*255)
    #Fijemos los datos entre 0 y 255, y que sean enteros
    lut=np.clip(lut, 0, 255).astype(np.uint8)
    #Apliquemos el LUT al array del canal
    array_procesado = lut[array_recibido]
    print(f"Worker {rank}: Cálculo de igualación completado")
    #Enviar el resultado al master
    comm.send(array_procesado, dest=0)
    print(f"Worker {rank}: Array procesado enviado al master")


comm.barrier()
if rank ==0:
    print("\nMaster: Paso 3 completado")
    print(f"Master: Todos los workers recibieron sus datos")
