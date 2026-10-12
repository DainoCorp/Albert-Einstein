#include <iostream>
#include <cstdio>
#include <cstring>
#include "buffet.h"

using namespace std;

// Misma cuenta que en normalizacion.cpp (corrimiento de K), si no el login no coincide
void encriptarPassword(char passOriginal[], char passDestino[], int k) {
    int i = 0;
    while (passOriginal[i] != '\0') {
        passDestino[i] = passOriginal[i] + k;
        i++;
    }
    passDestino[i] = '\0';
}

// Lee un numero y si escriben letras vuelve a pedir
int leerNumero() {
    int n;
    while (!(cin >> n)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Ingrese un numero valido: ";
    }
    return n;
}

// Pide la fecha una sola vez, formato DD-MM-AAAA
void pedirFecha(char fecha[]) {
    char texto[50];
    bool ok = false;
    while (!ok) {
        cout << "Fecha del dia (DD-MM-AAAA): ";
        cin >> texto;
        if (strlen(texto) == 10 && texto[2] == '-' && texto[5] == '-') {
            strcpy(fecha, texto);
            ok = true;
        } else {
            cout << "Formato incorrecto, ejemplo: 03-06-2025" << endl;
        }
    }
}

// Si la planilla del dia no existe la crea (con "ab" si ya existe no se pisa nada)
void crearPlanillaSiNoExiste(char nombreArchivo[]) {
    FILE* f = fopen(nombreArchivo, "ab");
    if (!f) {
        cout << "ERROR: no se pudo abrir ni crear " << nombreArchivo << endl;
        return;
    }
    fclose(f);
}

// Devuelve el idMozo si el mozo existe y la clave coincide, sino devuelve 0
int loginMozo() {
    int id;
    char clave[50];          // mas grande que el password por si tipean de mas
    char claveCifrada[50];
    Mozo m;

    cout << "Numero de mozo: ";
    id = leerNumero();

    FILE* fMozos = fopen("datos/mozos.dat", "rb");
    if (!fMozos) {
        cout << "ERROR: No se pudo abrir mozos.dat" << endl;
        return 0;
    }

    // Los ids son correlativos desde 1, asi que el mozo N esta en la posicion N-1
    if (id < 1 || fseek(fMozos, (long)(id - 1) * sizeof(Mozo), SEEK_SET) != 0
        || fread(&m, sizeof(Mozo), 1, fMozos) != 1 || m.idMozo != id) {
        cout << "El mozo " << id << " no existe." << endl;
        fclose(fMozos);
        return 0;
    }
    fclose(fMozos);

    cout << "Clave: ";
    cin >> clave;
    encriptarPassword(clave, claveCifrada, K_ENCRIPTACION);

    if (strcmp(m.password, claveCifrada) != 0) {
        cout << "Clave incorrecta." << endl;
        return 0;
    }

    cout << "Bienvenido " << m.nombre << endl;
    return id;
}

// El inventario esta ordenado por codigo (pero con huecos), asi que uso busqueda binaria
// Devuelve la posicion del producto en el archivo o -1 si no esta
int buscarProducto(FILE* fInventario, int codigo, Producto& prod) {
    fseek(fInventario, 0, SEEK_END);
    int cantidad = ftell(fInventario) / sizeof(Producto);

    int ini = 0;
    int fin = cantidad - 1;
    while (ini <= fin) {
        int medio = (ini + fin) / 2;
        fseek(fInventario, (long)medio * sizeof(Producto), SEEK_SET);
        fread(&prod, sizeof(Producto), 1, fInventario);
        if (prod.codigo == codigo) {
            return medio;
        } else if (prod.codigo < codigo) {
            ini = medio + 1;
        } else {
            fin = medio - 1;
        }
    }
    return -1;
}

void cargarVenta(char nombreArchivo[]) {
    int idMozo = loginMozo();
    if (idMozo == 0) {
        cout << "No se puede cargar la venta." << endl;
        return;
    }

    cout << "Codigo de producto: ";
    int codigo = leerNumero();
    cout << "Cantidad: ";
    int cantidad = leerNumero();
    if (cantidad <= 0) {
        cout << "La cantidad tiene que ser mayor a 0." << endl;
        return;
    }

    FILE* fInventario = fopen("datos/inventario.dat", "rb+");
    if (!fInventario) {
        cout << "ERROR: No se pudo abrir inventario.dat" << endl;
        return;
    }

    Producto prod;
    int pos = buscarProducto(fInventario, codigo, prod);
    if (pos == -1) {
        cout << "El producto " << codigo << " no existe." << endl;
        fclose(fInventario);
        return;
    }
    if (prod.stockActual < cantidad) {
        cout << "Stock insuficiente de " << prod.descripcion
             << " (quedan " << prod.stockActual << ")." << endl;
        fclose(fInventario);
        return;
    }

    Comanda venta;
    venta.idMozo = idMozo;
    venta.codigoProducto = codigo;
    venta.cantidad = cantidad;
    venta.comision = prod.precio * cantidad * TASA_COMISION;

    FILE* fDia = fopen(nombreArchivo, "ab");
    if (!fDia) {
        cout << "ERROR: No se pudo abrir " << nombreArchivo << endl;
        fclose(fInventario);
        return;
    }
    fwrite(&venta, sizeof(Comanda), 1, fDia);
    fclose(fDia);

    // Descuento lo vendido del stock, sin rehacer el archivo
    prod.stockActual -= cantidad;
    fseek(fInventario, (long)pos * sizeof(Producto), SEEK_SET);
    fwrite(&prod, sizeof(Producto), 1, fInventario);
    fclose(fInventario);

    cout << "Venta cargada. Comision: $" << venta.comision << endl;
}

// Ordena la planilla del dia por idMozo (burbujeo directo sobre el archivo)
void ordenarPorMozo(char nombreArchivo[]) {
    FILE* f = fopen(nombreArchivo, "rb+");
    if (!f) {
        cout << "ERROR: No se pudo abrir " << nombreArchivo << " para ordenar" << endl;
        return;
    }

    fseek(f, 0, SEEK_END);
    int cant = ftell(f) / sizeof(Comanda);

    Comanda a, b;
    for (int i = 0; i < cant - 1; i++) {
        for (int j = 0; j < cant - 1 - i; j++) {
            fseek(f, (long)j * sizeof(Comanda), SEEK_SET);
            fread(&a, sizeof(Comanda), 1, f);
            fread(&b, sizeof(Comanda), 1, f);
            if (a.idMozo > b.idMozo) {
                fseek(f, (long)j * sizeof(Comanda), SEEK_SET);
                fwrite(&b, sizeof(Comanda), 1, f);
                fwrite(&a, sizeof(Comanda), 1, f);
            }
        }
    }
    fclose(f);
    cout << "[Ventas] " << nombreArchivo << " ordenado por mozo." << endl;
}

int main() {
    char fecha[11];
    char nombreArchivo[60];

    pedirFecha(fecha);
    sprintf(nombreArchivo, "datos/comandas_%s.dat", fecha);
    crearPlanillaSiNoExiste(nombreArchivo);

    int opcion = -1;
    while (opcion != 0) {
        cout << endl << "--- VENTAS DEL DIA " << fecha << " ---" << endl;
        cout << "1. Cargar venta" << endl;
        cout << "0. Terminar" << endl;
        cout << "Opcion: ";
        opcion = leerNumero();

        if (opcion == 1) {
            cargarVenta(nombreArchivo);
        } else if (opcion != 0) {
            cout << "Opcion invalida." << endl;
        }
    }

    ordenarPorMozo(nombreArchivo);
    return 0;
}