#include <iostream>
#include <cstdio>
#include <cstring>
#include "buffet.h"
using namespace std;

void mostrarMozos() {
    FILE* f = fopen("datos/mozos.dat", "rb");
    if (!f) {
        cout << "No se pudo abrir mozos.dat" << endl;
        return;
    }

    Mozo m;
    cout << "--- LISTADO DE MOZOS NORMALIZADOS ---" << endl;
    while (fread(&m, sizeof(Mozo), 1, f) == 1) {
        cout << "ID: " << m.idMozo 
             << " | Nombre: " << m.nombre 
             << " | Clave cifrada: " << m.password 
             << " | Total Comision: $" << m.totalComision << endl;
    }
    fclose(f);
}

void encriptarPassword(const char* passOriginal, char* passDestino, int k) {
    int i = 0;
    while (passOriginal[i] != '\0') {
        passDestino[i] = passOriginal[i] + k;
        i++;
    }
    passDestino[i] = '\0';
}

void actualizarStock() {
    FILE* fHistoricas = fopen("datos/comandas_historicas.dat", "rb");
    if (!fHistoricas) {
        cout << "ERROR: No se pudo abrir comandas_historicas.dat" << endl;
        return;
    }

    FILE* fInventario = fopen("datos/inventario.dat", "rb+");
    if (!fInventario) {
        cout << "ERROR: No se pudo abrir inventario.dat" << endl;
        fclose(fHistoricas);
        return;
    }

    ComandaHistorica comanda;
    Producto prod;

    while (fread(&comanda, sizeof(ComandaHistorica), 1, fHistoricas) == 1) {
        rewind(fInventario);

        while (fread(&prod, sizeof(Producto), 1, fInventario) == 1) {
            if (prod.codigo == comanda.codigoProducto) {
                prod.stockActual -= comanda.cantidad;

                fseek(fInventario, -sizeof(Producto), SEEK_CUR);
                fwrite(&prod, sizeof(Producto), 1, fInventario);
                break;
            }
        }
    }

    fclose(fHistoricas);
    fclose(fInventario);
    cout << "[Stock] inventario.dat actualizado correctamente." << endl;
}

int buscarMozoPorNombre(const Mozo mozos[], int cantMozos, const char* nombre) {
    for (int i = 0; i < cantMozos; i++) {
        if (strcmp(mozos[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

void ordenarComandasPorMozo(Comanda v[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (v[j].idMozo > v[j + 1].idMozo) {
                Comanda aux = v[j];
                v[j] = v[j + 1];
                v[j + 1] = aux;
            }
        }
    }
}

int generarArchivoMozos(Mozo mozos[], int maxMozos) {
    FILE* fHistoricas = fopen("datos/comandas_historicas.dat", "rb");
    if (!fHistoricas) {
        cout << "ERROR: No se pudo abrir comandas_historicas.dat" << endl;
        return 0;
    } 

    int cantMozos = 0;
    ComandaHistorica reg;

    while (fread(&reg, sizeof(ComandaHistorica), 1, fHistoricas) == 1) {
        int idx = buscarMozoPorNombre(mozos, cantMozos, reg.nombreMozo);
        if (idx == -1) {
            if (cantMozos < maxMozos) {
                mozos[cantMozos].idMozo = cantMozos + 1;
                strcpy(mozos[cantMozos].nombre, reg.nombreMozo);
                mozos[cantMozos].totalComision = reg.comision;

                char clavePlana[20];
                sprintf(clavePlana, "%d", mozos[cantMozos].idMozo);
                encriptarPassword(clavePlana, mozos[cantMozos].password, K_ENCRIPTACION);

                cantMozos++;
            }
        } else {
            mozos[idx].totalComision += reg.comision;
        }
    }
    fclose(fHistoricas);
    
    FILE* fMozos = fopen("datos/mozos.dat", "wb");
    if (!fMozos) {
        cout << "Error: no se pudo crear mozos.dat" << endl;
        return 0;
    }
    fwrite(mozos, sizeof(Mozo), cantMozos, fMozos);
    fclose(fMozos);

    cout << "[Mozo] mozos.dat generado con exito. Total: " << cantMozos << " mozos." << endl;
    return cantMozos;
}

void generarPlanillasDiarias(const Mozo mozos[], int cantMozos) {
    FILE* fHistoricas = fopen("datos/comandas_historicas.dat", "rb");
    if (!fHistoricas) return;

    ComandaHistorica todas[500];
    int total = 0;
    while (fread(&todas[total], sizeof(ComandaHistorica), 1, fHistoricas) == 1) {
        total++;
    }
    fclose(fHistoricas);

    bool procesadas[500] = {false};

    for (int i = 0; i < total; i++) {
        if (procesadas[i]) continue;

        char fechaActual[11];
        strcpy(fechaActual, todas[i].fecha);

        Comanda ventasDia[100];
        int cantVentasDia = 0;

        for (int j = i; j < total; j++) {
            if (strcmp(todas[j].fecha, fechaActual) == 0) {
                procesadas[j] = true;
                int idxMozo = buscarMozoPorNombre(mozos, cantMozos, todas[j].nombreMozo);
                if (idxMozo != -1) {
                    ventasDia[cantVentasDia].idMozo = mozos[idxMozo].idMozo;
                    ventasDia[cantVentasDia].codigoProducto = todas[j].codigoProducto;
                    ventasDia[cantVentasDia].cantidad = todas[j].cantidad;
                    ventasDia[cantVentasDia].comision = todas[j].comision;
                    cantVentasDia++;
                }
            }
        }

        ordenarComandasPorMozo(ventasDia, cantVentasDia);

        char nombreArchivo[50];
        sprintf(nombreArchivo, "datos/comandas_%s.dat", fechaActual);

        FILE* fDia = fopen(nombreArchivo, "wb");
        if (fDia) {
            fwrite(ventasDia, sizeof(Comanda), cantVentasDia, fDia);
            fclose(fDia);
            cout << "[Planilla Diaria] Creada: " << nombreArchivo << endl;
        }
    }
}

int main() {
    Mozo listaMozos[100];
    int cantMozos = generarArchivoMozos(listaMozos, 100);
    if (cantMozos > 0) {
        mostrarMozos();
        actualizarStock();
        generarPlanillasDiarias(listaMozos, cantMozos);
    }   
    return 0;
}
