#include <iostream>
#include <cstdio> 
#include <cstring> // Para strcmp

// Estructura oficial para las planillas diarias y semanales
struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

// Función para ordenar el archivo semanal en disco por idMozo (Método Burbuja)
void ordenarArchivoSemanal(const char* rutaSemanal) {
    FILE* arch = fopen(rutaSemanal, "rb+"); 
    if (arch == NULL) return;

    // 1. Calcular la cantidad total de registros en el archivo de forma segura
    fseek(arch, 0, SEEK_END);
    
    // Dejamos tamanoTotal con signo para capturar el -1 si hay error
    long tamanoTotal = ftell(arch); 
    
    // Control de seguridad por si el archivo no se leyó bien
    if (tamanoTotal == -1) {
        fclose(arch);
        return;
    }

    // Convertimos de forma segura a int común el total de registros
    int totalRegistros = (int)(tamanoTotal / (long)sizeof(Comanda));//int totalRegistros = (int)(tamanoTotal / sizeof(Comanda));

    Comanda c1, c2;

    // 2. Algoritmo de la burbuja usando INT comunes para evitar mezclas con unsigned
    for (int i = 0; i < totalRegistros - 1; i++) {
        for (int j = 0; j < totalRegistros - 1 - i; j++) {
            
            // Leer registro J
            // Forzamos a que el salto del fseek sea un long para que no se queje en 64 bits
            fseek(arch, (long)j * (long)sizeof(Comanda), SEEK_SET);
            fread(&c1, sizeof(Comanda), 1, arch);

            // Leer registro J + 1
            fread(&c2, sizeof(Comanda), 1, arch);

            // Si están desordenados, los intercambiamos de lugar
            if (c1.idMozo > c2.idMozo) {
                fseek(arch, (long)j * (long)sizeof(Comanda), SEEK_SET);
                fwrite(&c2, sizeof(Comanda), 1, arch);
                fwrite(&c1, sizeof(Comanda), 1, arch);
            }
        }
    }
    fclose(arch);
}

int main() {
    char nombreDiario[50];
    char nombreSemanal[50];
    
    // 1. Pedir el nombre del archivo semanal de salida
    std::cout << "Ingrese el nombre para el archivo semanal (ej: comandas_semana_s1-06.dat): ";
    std::cin >> nombreSemanal;

    // 2. Abrir el archivo semanal en modo escritura
    FILE* archSemanal = fopen(nombreSemanal, "wb");
    if (archSemanal == NULL) {
        std::cout << "Error al crear el archivo semanal." << std::endl;
        return 1;
    }

    Comanda registroAux;
    int archivosProcesados = 0;

    // 3. Bucle para pedir y procesar archivos diarios hasta que el usuario escriba "fin"
    while (true) {
        std::cout << "Ingrese el nombre de un archivo diario a incluir (o 'fin' para terminar): ";
        std::cin >> nombreDiario;

        // Si el usuario escribe "fin", salimos del bucle
        if (strcmp(nombreDiario, "fin") == 0) {
            break;
        }

        // Intentar abrir el archivo diario ingresado
        FILE* archDiario = fopen(nombreDiario, "rb");
        
        if (archDiario != NULL) {
            // Copiar los registros al archivo semanal
            while (fread(&registroAux, sizeof(Comanda), 1, archDiario) == 1) {
                fwrite(&registroAux, sizeof(Comanda), 1, archSemanal);
            }
            fclose(archDiario);
            archivosProcesados++;
            std::cout << "-> Archivo '" << nombreDiario << "' acoplado con exito." << std::endl;
        } else {
            std::cout << "[ERROR] El archivo '" << nombreDiario << "' no existe. Intente de nuevo." << std::endl;
        }
    }

    // Cerrar el archivo semanal para impactar los cambios en el disco
    fclose(archSemanal);

    // 4. Si se procesó al menos un archivo, ordenamos el resultado
    if (archivosProcesados > 0) {
        std::cout << "\nOrdenando la planilla semanal por ID de Mozo..." << std::endl;
        ordenarArchivoSemanal(nombreSemanal);
        std::cout << "¡Proceso terminado! Se unificaron " << archivosProcesados << " archivos." << std::endl;
    } else {
        std::cout << "No se proceso ningun archivo diario." << std::endl;
    }

    return 0;
} 