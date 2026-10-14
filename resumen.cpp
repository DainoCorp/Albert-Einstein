#include <iostream>
#include <cstdio>
#include <cstring>
#include "buffet.h"
using namespace std;

void armar_ruta_archivo(char rutaDestino[], char semana[]) {
    strcpy(rutaDestino, "datos/comandas_semana_");
    strcat(rutaDestino, semana);
    strcat(rutaDestino, ".dat");
}

void corte_control(FILE* fSemana) {
    Comanda com;
    int leido = fread(&com, sizeof(Comanda), 1, fSemana);
    if (leido == 1) {
        int TotalProductos = 0;
        cout << "--- RESUMEN SEMANAL DEL BUFFET ---" << endl;
        while (leido == 1) {
            int mozoActual = com.idMozo;
            int totalProductosMozo = 0;
            float totalComisionMozo = 0.0f;
            while (leido == 1 && com.idMozo == mozoActual) {
                totalProductosMozo += com.cantidad;
                totalComisionMozo += com.comision;
                TotalProductos += com.cantidad;
                leido = fread(&com, sizeof(Comanda), 1, fSemana);
            }
            cout << "Mozo ID " << mozoActual << " -> Vendio: " << totalProductosMozo
                 << " prod. | Comision a pagar: $" << totalComisionMozo << endl;
        }
        cout << "----------------------------------" << endl;
        cout << "TOTAL PRODUCTOS VENDIDOS EN EL BUFFET: " << TotalProductos << endl;
    } else {
        cout << "El archivo semanal esta vacio." << endl;
    }
}

int main() {
    char semana[50];
    char rutaCompleta[100];    
    cout << "Ingrese la semana (ej: s1-10): ";
    cin >> semana;
    armar_ruta_archivo(rutaCompleta, semana);
    FILE* fSemana = fopen(rutaCompleta, "rb");
    if (!fSemana) {
        cout << "Error: No se pudo encontrar el archivo en " << rutaCompleta << endl;
    } else {
        corte_control(fSemana);
        fclose(fSemana);
    }
    cout << "Escriba cualquier letra y presione Enter para salir: ";
    char pausa;
    cin >> pausa;
    return 0;
}