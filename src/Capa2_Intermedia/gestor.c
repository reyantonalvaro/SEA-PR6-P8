#include "gestor.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "datos.h" 

typedef void (*CalcFxFunc)(uint32_t, uint32_t, uint32_t *);

typedef struct {
    const char *nombre;
    CalcFxFunc func_calculo;
    uint8_t shift_in1;
    uint8_t shift_in2;
    uint8_t shift_out;
} ConfigEjercicio;

// Helper para cargar configuración
static ConfigEjercicio cargar_configuracion(TipoEjercicio tipo) {
    ConfigEjercicio cfg;
    uint8_t shifts[3]; 

    switch (tipo) {
        case EJERCICIO_MOLES:
            cfg.nombre = "MOLES";
            cfg.func_calculo = calc_moles_fx;
            tam_moles(shifts); 
            break;
            
        case EJERCICIO_DENSIDAD:
            cfg.nombre = "DENSIDAD";
            cfg.func_calculo = calc_density_fx;
            escalado(shifts); 
            break;
            
        case EJERCICIO_CINETICA:
            cfg.nombre = "CINETICA";
            cfg.func_calculo = calc_kinetic_energy_fx;
            escalado_k(shifts); 
            break;
    }

    cfg.shift_in1 = shifts[0];
    cfg.shift_in2 = shifts[1];
    cfg.shift_out = shifts[2];

    return cfg;
}

// ============================================================================
//  IMPLEMENTACIÓN CON FFLUSH AÑADIDO
// ============================================================================

void test_automatico(TipoEjercicio tipo, const char *ruta_fichero) {
    ConfigEjercicio cfg = cargar_configuracion(tipo);
    
    FILE *f = fopen(ruta_fichero, "r");
    if (!f) {
        printf("ERROR Capa 2: No se pudo abrir el archivo '%s'\n", ruta_fichero);
        fflush(stdout); // IMPORTANTE PARA PYTEST
        return;
    }

    printf("\n=== [Capa 2] Iniciando Test Automático: %s ===\n", cfg.nombre);
    printf("Configuración cargada de Capa 1 -> Shifts: In1=%d, In2=%d, Out=%d\n", 
           cfg.shift_in1, cfg.shift_in2, cfg.shift_out);
    
    double d1_f, d2_f, esp_f;
    uint32_t d1_fx, d2_fx, res_fx;
    int fila = 1, errores = 0;

    while (fscanf(f, "%lf %lf %lf", &d1_f, &d2_f, &esp_f) == 3) {
        // Escalado
        d1_fx = (uint32_t)round(d1_f * pow(2, cfg.shift_in1));
        d2_fx = (uint32_t)round(d2_f * pow(2, cfg.shift_in2));

        // Ejecución
        cfg.func_calculo(d1_fx, d2_fx, &res_fx);

        // Desescalado
        double calc_f = (double)res_fx / (1 << cfg.shift_out);

        // Verificación
        double err = 0.0;
        if (esp_f != 0.0) err = fabs(calc_f - esp_f) / esp_f;
        else if (fabs(calc_f) > 0.0001) err = 1.0;

        if (err > 0.05) {
            printf("[FALLO] Fila %d: In(%.3f, %.3f) -> Esp: %.5f, Obt: %.5f (Err: %.2f%%)\n", 
                   fila, d1_f, d2_f, esp_f, calc_f, err*100);
            errores++;
        } else {
            printf("[ OK ] Fila %d: Error %.2f%%\n", fila, err * 100);
        }
        fila++;
    }
    printf("--- Fin Test %s: %d errores encontrados ---\n", cfg.nombre, errores);
    fflush(stdout); // IMPORTANTE PARA PYTEST
    fclose(f);
}

void test_especifico(TipoEjercicio tipo, double val1, double val2) {
    ConfigEjercicio cfg = cargar_configuracion(tipo);
    uint32_t d1_fx, d2_fx, res_fx;

    printf("\n=== [Capa 2] Test Específico: %s ===\n", cfg.nombre);
    printf("Shifts usados: %d, %d -> %d\n", cfg.shift_in1, cfg.shift_in2, cfg.shift_out);

    d1_fx = (uint32_t)round(val1 * pow(2, cfg.shift_in1));
    d2_fx = (uint32_t)round(val2 * pow(2, cfg.shift_in2));

    cfg.func_calculo(d1_fx, d2_fx, &res_fx);

    double res_f = (double)res_fx / (1 << cfg.shift_out);

    printf("Entrada: %.4f, %.4f\n", val1, val2);
    printf("Hex Calculado: 0x%X\n", res_fx);
    printf("Resultado Final (Float): %.6f\n", res_f);
    
    fflush(stdout); // IMPORTANTE: Fuerza el volcado del buffer para que Python lo lea
}

void crear_fichero(TipoEjercicio tipo, const char *nombre_fichero, double *v1, double *v2, double *res, int n) {
    FILE *f = fopen(nombre_fichero, "w");
    if (!f) {
        printf("ERROR Capa 2: No se pudo crear el archivo '%s'\n", nombre_fichero);
        fflush(stdout);
        return;
    }
    
    for (int i = 0; i < n; i++) {
        fprintf(f, "%.6f %.6f %.6f\n", v1[i], v2[i], res[i]);
    }
    
    fclose(f);
    printf("\n=== [Capa 2] Fichero Generado ===\n");
    printf("Archivo: %s\n", nombre_fichero);
    fflush(stdout); // IMPORTANTE
}