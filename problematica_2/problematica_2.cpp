#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <omp.h>
#include <cmath>

//Struct para guardar toda la fila del archivo
struct ImuSample{
    long t_ms;
    int clip_id;
    double ax, ay, az;
    double gx, gy, gz;
    double label_vector; 
    std::string label_texto;
};

//Struct para realizar todas las ventanas
struct Ventana{
    int indice_inicio;
    int num_muestras;
};

//Struct para colocar las metricas que serán calculadas
struct MetricasVentana {
    // [ax, ay, az, gx, gy, gz]
    double media[6] = {0.0};
    double std[6] = {0.0};
    double rms[6] = {0.0};
    
    // Métrica para la energía
    double energia_accel = 0.0;
};



MetricasVentana calcular_metricas(const Ventana& v, const std::vector<ImuSample>& datos) {
    
    MetricasVentana resultado;
    int N_w = v.num_muestras; // Total de muestras en la ventana
    int inicio = v.indice_inicio;

    // Arreglos temporales para los cálculos
    double suma[6] = {0.0};
    double suma_cuadrados[6] = {0.0};
    double suma_dif_cuadradas[6] = {0.0};
    double suma_cuadrados_magnitud_accel = 0.0; // Para la Energía 

    // Calcular Sumas (para Media) y Magnitud (para Energía)
    for (int i = 0; i < N_w; ++i) {
        const ImuSample& muestra = datos[inicio + i];

        // Ponemos los 6 ejes en un array temporal para facilitar el bucle
        double ejes[6] = {muestra.ax, muestra.ay, muestra.az, muestra.gx, muestra.gy, muestra.gz};

        // Sumar para la media
        for (int j = 0; j < 6; ++j) {
            suma[j] += ejes[j];
        }

        // Calcular ||a||^2 para la Energía
        double mag_accel_cuadrada = (muestra.ax * muestra.ax) + 
                                    (muestra.ay * muestra.ay) + 
                                    (muestra.az * muestra.az);
        
        // Acumular la suma de las magnitudes al cuadrado
        suma_cuadrados_magnitud_accel += mag_accel_cuadrada;
    }

    // Calcular Medias y Energía
    for (int j = 0; j < 6; ++j) {
        resultado.media[j] = suma[j] / N_w;
    }
    // La energía es la suma total de las magnitudes al cuadrado 
    resultado.energia_accel = suma_cuadrados_magnitud_accel;

    // Calcular Sumas Cuadradas (para RMS y Std)
    for (int i = 0; i < N_w; ++i) {
        const ImuSample& muestra = datos[inicio + i];
        double ejes[6] = {muestra.ax, muestra.ay, muestra.az, muestra.gx, muestra.gy, muestra.gz};

        for (int j = 0; j < 6; ++j) {
            // Para RMS (Suma de x[n]^2) 
            suma_cuadrados[j] += (ejes[j] * ejes[j]);
            // Para Std (Suma de (x[n] - media)^2)
            suma_dif_cuadradas[j] += (ejes[j] - resultado.media[j]) * (ejes[j] - resultado.media[j]);
        }
    }

    // Calcular RMS y Std
    for (int j = 0; j < 6; ++j) {
        // Fórmula RMS
        resultado.rms[j] = std::sqrt(suma_cuadrados[j] / N_w);
        
        // Fórmula Desv. Estándar
        if (N_w > 1) {
            resultado.std[j] = std::sqrt(suma_dif_cuadradas[j] / (N_w - 1));
        } else {
            resultado.std[j] = 0.0;
        }
    }

    return resultado;
}


int main(){
    //Lectura secuencial del archivo
    std::vector<ImuSample> datos;
    std::string nombre_archivo = "imu_data.csv";
    std::ifstream archivo(nombre_archivo);

    if (!archivo.is_open()) { // verificación de error
        std::cerr << "Error: No se pudo abrir el archivo " << nombre_archivo << std::endl;
        return 1;
    }

    std::string linea;
    std::getline(archivo, linea);
    long numero_linea = 1; // Para saber qué línea falla

    while(std::getline(archivo, linea)){
        numero_linea++; 

        // Ignorar líneas vacías
        if (linea.empty() || linea.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue; // Saltar a la siguiente línea
        }

        std::stringstream ss(linea);
        std::string campo;
        ImuSample muestra_actual;
        
        try {
            
            std::getline(ss, campo, ','); muestra_actual.t_ms = std::stol(campo);
            std::getline(ss, campo, ','); muestra_actual.clip_id = std::stoi(campo);
            std::getline(ss, campo, ','); muestra_actual.ax = std::stod(campo);
            std::getline(ss, campo, ','); muestra_actual.ay = std::stod(campo);
            std::getline(ss, campo, ','); muestra_actual.az = std::stod(campo);
            std::getline(ss, campo, ','); muestra_actual.gx = std::stod(campo);
            std::getline(ss, campo, ','); muestra_actual.gy = std::stod(campo);
            std::getline(ss, campo, ','); muestra_actual.gz = std::stod(campo);


            std::string label_completa;
            std::getline(ss, label_completa); 
            
            // Quitar espacios en blanco del inicio (si los hay)
            size_t inicio_texto = label_completa.find_first_not_of(" \t,");
            if (inicio_texto != std::string::npos) {
                label_completa = label_completa.substr(inicio_texto);
            }

            // 3. Crear un "partidor" solo para el label
            std::stringstream ss_label(label_completa);
            std::string label_parte_num;
            
            ss_label >> label_parte_num; // Intenta leer la primera "palabra"
            
            try {
                // intenta convertir esa primera palabra a número
                muestra_actual.label_vector = std::stod(label_parte_num);
                // Si tuvo éxito, lee el resto como texto
                std::getline(ss_label, muestra_actual.label_texto);
                // Quitar espacio en blanco inicial del texto
                size_t inicio_resto = muestra_actual.label_texto.find_first_not_of(" \t");
                if (inicio_resto != std::string::npos) {
                    muestra_actual.label_texto = muestra_actual.label_texto.substr(inicio_resto);
                }

            } catch (const std::invalid_argument& e_label) {
                // Si FALLA
                // Asigna un valor default (como -1)
                muestra_actual.label_vector = -1.0; 
                // Y guarda la palabra completa como el texto
                muestra_actual.label_texto = label_completa;
            }
        
            
            datos.push_back(muestra_actual);

        } catch (const std::invalid_argument& e) {
            // catch para errores en (ax, ay, az...) ---
            std::cerr << "---------------------------------------------" << std::endl;
            std::cerr << "¡ERROR DE LECTURA! El programa se detendra." << std::endl;
            std::cerr << "Error en la linea #" << numero_linea << " del CSV." << std::endl;
            std::cerr << "Contenido de la linea: " << linea << std::endl;
            std::cerr << "El campo que fallo era: \"" << campo << "\"" << std::endl;
            std::cerr << "Error C++: " << e.what() << std::endl;
            std::cerr << "---------------------------------------------" << std::endl;
            archivo.close(); 
            return 1; 
        }
    } 
    archivo.close();
    std::cout << "--- Lectura de CSV completada (SECUENCIAL) ---" << std::endl;
    std::cout << "Total de muestras leidas: " << datos.size() << std::endl;

    //Construimos las ventanas
    const int MUESTRAS_POR_VENTANA = 200; // 100 Hz * 2 segundos
    const int MUESTRAS_STRIDE = 100;      // 50% de solapamiento
    std::vector<Ventana> ventanas;
    std::cout << "\n--- Construyendo ventanas (SECUENCIAL) ---" << std::endl;
    std::cout << "Parametros: Tamano=200 muestras (2s), Stride=100 muestras (1s)" << std::endl;

    for (int i = 0; i <= datos.size() - MUESTRAS_POR_VENTANA; i += MUESTRAS_STRIDE) {
        Ventana v;
        v.indice_inicio = i;
        v.num_muestras = MUESTRAS_POR_VENTANA;
        ventanas.push_back(v);
    }

    // verificacion
    std::cout << "\n--- Verificacion de ventanas ---" << std::endl;
    std::cout << "Total de ventanas creadas: " << ventanas.size() << std::endl;

    //calculo de metricas en paralelo
    
    // Crear el "buffer de salida"
    std::vector<MetricasVentana> resultados(ventanas.size());

    std::cout << "\n--- Iniciando Paso 3: Calculo de Metricas (Paralelo) ---" << std::endl;
    std::cout << "Usando " << omp_get_max_threads() << " hilos..." << std::endl;
    
    // Iniciar cronómetro
    double tiempo_inicio = omp_get_wtime();

    // El bucle PARALELO
    #pragma omp parallel for
    for (int i = 0; i < ventanas.size(); ++i) {
        const Ventana& v = ventanas[i];
        MetricasVentana m = calcular_metricas(v, datos);
        resultados[i] = m; 
    }

    // Detener cronómetro
    double tiempo_fin = omp_get_wtime();
    double tiempo_total_paralelo = tiempo_fin - tiempo_inicio;
    std::cout << "Calculo paralelo completado." << std::endl;

    // VERIFICACIÓN 
    std::cout << "\n--- Verificacion de resultados ---" << std::endl;
    std::cout << "Tiempo total (paralelo): " << tiempo_total_paralelo << " segundos" << std::endl;
    
    std::cout << "\n--- Resultados de las primeras 3 ventanas ---" << std::endl;
    std::cout << std::fixed << std::setprecision(4); // Formatear a 4 decimales
    
    // Definimos etiquetas para los ejes
    std::string etiquetas_ejes[6] = {"ax", "ay", "az", "gx", "gy", "gz"};

    for (int i = 0; i < 3 && i < resultados.size(); ++i) {
        std::cout << "--- Ventana " << i << " ---" << std::endl;
        
        // Imprimir métricas para los 3 ejes del acelerómetro
        for (int j = 0; j < 3; ++j) { // Solo ax, ay, az
            std::cout << "  " << etiquetas_ejes[j] << " (Media/Std/RMS): " 
                      << std::setw(8) << resultados[i].media[j] << " / " 
                      << std::setw(8) << resultados[i].std[j] << " / " 
                      << std::setw(8) << resultados[i].rms[j] << std::endl;
        }
        
        // Imprimir la Energía (E_||a||)
        std::cout << "  Energia Accel (E_||a||): " << resultados[i].energia_accel << std::endl;
    }

    return 0;
}