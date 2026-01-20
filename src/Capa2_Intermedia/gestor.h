#ifndef GESTOR_H
#define GESTOR_H

#include <stdint.h>

// Enumerado para seleccionar la lógica interna
typedef enum {
    EJERCICIO_MOLES,
    EJERCICIO_DENSIDAD,
    EJERCICIO_CINETICA
} TipoEjercicio;

// --- FUNCIONES PURAS (Sin GTK) ---

// Recibe la ruta del fichero, lo abre, lee línea a línea y llama a Capa 1
void test_automatico(TipoEjercicio tipo, const char *ruta_fichero);

// Recibe los valores numéricos crudos y muestra el resultado por consola
void test_especifico(TipoEjercicio tipo, double val1, double val2);

// Recibe arrays de datos y escribe un fichero de texto con el formato correcto
void crear_fichero(TipoEjercicio tipo, const char *nombre_fichero, double *v1, double *v2, double *res, int n);

#endif