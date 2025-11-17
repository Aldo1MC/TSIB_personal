# Actividades de Cómputo Paralelo (MPI y OpenMP)

Se presentan las soluciones a la problematica 1 (MPI en el análisis de imágenes médicas) y problematica 2 (OpenMP para análisis de registros inerciales.

**Autor:** Aldo Martínez Cruz

---

## 📂 Contenido del Repositorio

* `/problematica_1/`: Solución de procesamiento de imágenes (igualación de histograma) usando MPI y Python.
* `/problematica_2/`: Solución de análisis de IMU's usando OpenMP y C++.

---

## Problemática 1: MPI y Análisis de Imágenes Médicas

Implementación de un modelo Master-Worker con `mpi4py` para aplicar una igualación de histograma a los 3 canales (RGB) de una imagen en paralelo.

### Dependencias
* Python 3
* `mpi4py` (`pip install mpi4py`)
* `numpy` (`pip install numpy`)
* `pillow` (`pip install pillow`)
* `matplotlib` (`pip install matplotlib`)

### Cómo Ejecutar
Los scripts deben ejecutarse con `mpirun`.

```bash
# 1. Ejecutar la versión paralela (MPI)
mpirun -n 4 python3 problematica_1.py

# 2. Ejecutar la versión secuencial (para comparar)
python3 version_secuencial_p1.py
Link al video explicativo: https://drive.google.com/file/d/1712Jo_0ByY6Vt0jK1BQ1HjMgG2CXsOdZ/view?usp=sharing
##  Problemática 2: OpenMP y Análisis de Datos IMU

Implementación de un programa C++ que usa OpenMP para paralelizar el cálculo de métricas (Media, Std, RMS, Energía) sobre ventanas deslizantes de datos IMU.

### Dependencias
* Un compilador de C++
* Soporte para OpenMP

### Cómo Compilar y Ejecutar (Terminal)

El código debe compilarse con `-fopenmp`.

```bash
# Compilar el programa paralelo
g++ -fopenmp -o problema2 problema2.cpp

# Compilar el programa secuencial
g++ -o problema2_secuencial problema2_secuencial.cpp

# Cómo Ejecutar (Ejemplo en PowerShell) 

# 3. Ejecutar la versión secuencial (para obtener Ts)
.\problema2_secuencial.exe

# Ejecutar la versión paralela (ej. con 4 hilos)
$env:OMP_NUM_THREADS = "4"
.\problema2.exe
Link al video explicativo: https://drive.google.com/file/d/1C8qbXyAPR7QQcdGZe36x8_tlZlvVyEKe/view?usp=sharing