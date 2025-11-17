import numpy as np
from PIL import Image
import time
#Funcion para aplicar la igualacion a un solo canal
def igualacion(array, alto, ancho):
    #Calculamos el histograma
    hist, bins = np.histogram(array.ravel(), 256, [0,256])
    #Calcular CDF
    cdf = hist.cumsum()
    #Calcular LUT
    cdf_mask = np.ma.masked_equal(cdf,0)
    cdf_min=cdf_mask.min()
    total_pixeles = alto*ancho

    lut=np.round(((cdf - cdf_min) / (total_pixeles - cdf_min))*255)
    lut=np.clip(lut,0,255).astype(np.uint8)
    #Apliquemos el lut
    canal_procesado = lut[array]
    return canal_procesado

print("Inicia el proceso secuencial")
tiempo_inicio = time.time()
try:
    #cargar imagen
    img_original = Image.open("imagen_prueba.jpg")
    img_rgb = img_original.convert('RGB')
    ancho, alto = img_rgb.size
    print(f"Imagen cargada: {ancho}x{alto}")
    #Separar en arrays
    canal_r, canal_g, canal_b = img_rgb.split()
    array_r = np.array(canal_r, dtype=np.uint8)
    array_g = np.array(canal_g, dtype=np.uint8)
    array_b = np.array(canal_b, dtype=np.uint8)
    print("Canales separados en arrays")
    #Procesar cada canal
    array_r_procesado = igualacion(array_r, alto, ancho)
    array_g_procesado = igualacion(array_g, alto, ancho)
    array_b_procesado = igualacion(array_b, alto, ancho)
    print("Todos los canales procesados")
    #Reconstruir la imagen y guardar
    img_r = Image.fromarray(array_r_procesado, 'L')
    img_g = Image.fromarray(array_g_procesado, 'L')
    img_b = Image.fromarray(array_b_procesado, 'L')
    img_final= Image.merge('RGB', (img_r, img_g, img_b))
    img_final.save('imagen_procesada_secuencial.png')
    print("Imagen guardada")
except FileNotFoundError:
    print("No se encontró el archivo")
#Tiempo final
tiempo_final = time.time()
tiempo_total_secuencial = tiempo_final - tiempo_inicio
print(f"TIempo del proceso secuencial: {tiempo_total_secuencial:.6f} segundos")
