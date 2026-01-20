#ifndef PUNTOFIJO_H
#define PUNTOFIJO_H

#include <stdint.h>

void tam_moles(uint8_t * tams);
void calc_moles_fx(uint32_t Vp_fx,uint32_t Vt_fx,uint32_t *resultado);

void escalado (uint8_t * escal);
void calc_density_fx(uint32_t dato_P, uint32_t dato_Vt,uint32_t *resultado);

void escalado_k (uint8_t * escal_k);
void calc_kinetic_energy_fx(uint32_t dato_A, uint32_t dato_W,uint32_t *resultado);

float calc_moles(float Vp, float Vt);
float calc_density(float P_raw, float Vt);
float calc_kinetic_energy(float A_raw, float W_raw);
#endif
