 #include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <chrono>

// Structs (Sin cambios) 
struct ImuSample{
    long t_ms;
    int clip_id;
    double ax, ay, az;
    double gx, gy, gz;
    double label_vector;
    std::string label_texto;
};

struct Ventana{
    int indice_inicio;
    int num_muestras;
};

struct MetricasVentana {
    double media[6] = {0.0};
    double std[6] = {0.0};
    double rms[6] = {0.0};
    double energia_accel = 0.0;
};

// Función de cálculo (Sin cambios) 
MetricasVentana calcular_metricas(const Ventana& v, const std::vector<ImuSample>& datos) {
    MetricasVentana resultado;
    int N_w = v.num_muestras;
    int inicio = v.indice_inicio;

    double suma[6] = {0.0};
    double suma_cuadrados[6] = {0.0};
    double suma_dif_cuadradas[6] = {0.0};
    double suma_cuadrados_magnitud_accel = 0.0;

    for (int i = 0; i < N_w; ++i) {
        const ImuSample& muestra = datos[inicio + i];
        double ejes[6] = {muestra.ax, muestra.ay, muestra.az, muestra.gx, muestra.gy, muestra.gz};
        for (int j = 0; j < 6; ++j) {
            suma[j] += ejes[j];
        }
        double mag_accel_cuadrada = (muestra.ax * muestra.ax) + (muestra.ay * muestra.ay) + (muestra.az * muestra.az);
        suma_cuadrados_magnitud_accel += mag_accel_cuadrada;
    }

    // Medias y Energía
    for (int j = 0; j < 6; ++j) {
        resultado.media[j] = suma[j] / N_w;
    }
    resultado.energia_accel = suma_cuadrados_magnitud_accel;

    for (int i = 0; i < N_w; ++i) {
        const ImuSample& muestra = datos[inicio + i];
        double ejes[6] = {muestra.ax, muestra.ay, muestra.az, muestra.gx, muestra.gy, muestra.gz};
        for (int j = 0; j < 6; ++j) {
            suma_cuadrados[j] += (ejes[j] * ejes[j]);
            suma_dif_cuadradas[j] += (ejes[j] - resultado.media[j]) * (ejes[j] - resultado.media[j]);
        }
    }

    // RMS y Std
    for (int j = 0; j < 6; ++j) {
        resultado.rms[j] = std::sqrt(suma_cuadrados[j] / N_w);
        if (N_w > 1) {
            resultado.std[j] = std::sqrt(suma_dif_cuadradas[j] / (N_w - 1));
        } else {
            resultado.std[j] = 0.0;
        }
    }
    return resultado;
}


int main(){
    // Lectura secuencial de archivo (sin cambios)
    std::vector<ImuSample> datos;
    std::string nombre_archivo = "imu_data.csv";
    std::ifstream archivo(nombre_archivo);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << nombre_archivo << std::endl;
        return 1;
    }
    std::string linea;
    std::getline(archivo, linea);
    long numero_linea = 1;
    while(std::getline(archivo, linea)){
        numero_linea++;
        if (linea.empty() || linea.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
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
            size_t inicio_texto = label_completa.find_first_not_of(" \t,");
            if (inicio_texto != std::string::npos) {
                label_completa = label_completa.substr(inicio_texto);
            }
            std::stringstream ss_label(label_completa);
            std::string label_parte_num;
            ss_label >> label_parte_num;
            try {
                muestra_actual.label_vector = std::stod(label_parte_num);
                std::getline(ss_label, muestra_actual.label_texto);
                size_t inicio_resto = muestra_actual.label_texto.find_first_not_of(" \t");
                if (inicio_resto != std::string::npos) {
                    muestra_actual.label_texto = muestra_actual.label_texto.substr(inicio_resto);
                }
            } catch (const std::invalid_argument& e_label) {
                muestra_actual.label_vector = -1.0; 
                muestra_actual.label_texto = label_completa;
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error en la linea #" << numero_linea << ": " << e.what() << std::endl;
            return 1;
        }
        datos.push_back(muestra_actual);
    }
    archivo.close();
    std::cout << "--- Lectura de CSV completada (SECUENCIAL) ---" << std::endl;
    std::cout << "Total de muestras leidas: " << datos.size() << std::endl;

    // construccion de ventanas (Sin cambios) 
    const int MUESTRAS_POR_VENTANA = 200;
    const int MUESTRAS_STRIDE = 100;
    std::vector<Ventana> ventanas;
    std::cout << "\n--- Construyendo ventanas (SECUENCIAL) ---" << std::endl;
    std::cout << "Parametros: Tamano=200 muestras (2s), Stride=100 muestras (1s)" << std::endl;
    for (int i = 0; i <= datos.size() - MUESTRAS_POR_VENTANA; i += MUESTRAS_STRIDE) {
        ventanas.push_back({i, MUESTRAS_POR_VENTANA});
    }
    std::cout << "\n--- Verificacion de ventanas ---" << std::endl;
    std::cout << "Total de ventanas creadas: " << ventanas.size() << std::endl;

    // CÁLCULO DE MÉTRICAS (SECUENCIAL)
    
    std::vector<MetricasVentana> resultados(ventanas.size());
    // no hay omp_set_num_threads

    std::cout << "\n--- Iniciando Paso 3: Calculo de Metricas (SECUENCIAL) ---" << std::endl;
    
    // cronometro
    auto tiempo_inicio = std::chrono::high_resolution_clock::now();

    // no hay #pragma omp parallel for
    for (int i = 0; i < ventanas.size(); ++i) {
        const Ventana& v = ventanas[i];
        MetricasVentana m = calcular_metricas(v, datos);
        resultados[i] = m;
    }
    
    // se detiene el cronómetro
    auto tiempo_fin = std::chrono::high_resolution_clock::now();
    // Calcular la duración
    std::chrono::duration<double> duracion = tiempo_fin - tiempo_inicio;
    double tiempo_total_secuencial = duracion.count(); // Tiempo en segundos

    std::cout << "Calculo secuencial completado." << std::endl;

    // verificacion
    std::cout << "\n--- Verificacion de resultados ---" << std::endl;
    std::cout << "Tiempo total (SECUENCIAL): " << tiempo_total_secuencial << " segundos" << std::endl;
    
    // (Imprimimos solo la Ventana 0 para verificar, no las 3)
    std::cout << "\n--- Resultados de la Ventana 0 (para verificar consistencia) ---" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::string etiquetas_ejes[6] = {"ax", "ay", "az", "gx", "gy", "gz"};
    
    if (!resultados.empty()) {
        std::cout << "--- Ventana 0 ---" << std::endl;
        for (int j = 0; j < 3; ++j) {
            std::cout << "  " << etiquetas_ejes[j] << " (Media/Std/RMS): " 
                      << std::setw(8) << resultados[0].media[j] << " / " 
                      << std::setw(8) << resultados[0].std[j] << " / " 
                      << std::setw(8) << resultados[0].rms[j] << std::endl;
        }
        std::cout << "  Energia Accel (E_||a||): " << resultados[0].energia_accel << std::endl;
    }

    return 0;
}