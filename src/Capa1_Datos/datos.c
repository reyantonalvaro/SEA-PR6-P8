// Funciones en punto fijo
// gcc puntofijo.c -o puntofijo -L. -lm
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "datos.h"

#define FX_MUL(a,b,c) (((uint64_t)(a) * (uint64_t)(b)) >> c)
#define FX_DIV(a,b,c) (((uint64_t)(a) << c) / (uint64_t)(b))
#define FX_SQRT(x,c) punto_fijo_sqrt(x,c)

uint32_t punto_fijo_sqrt(uint32_t valor,uint8_t escalado) {
    if (valor <= 0) return 0;  
    uint32_t resultado = valor / 2;
    uint32_t anterior = 0;    
    while (resultado != anterior) {
        anterior = resultado;
        resultado = (anterior + FX_DIV(valor, anterior,escalado)) / 2;
    }    
    return resultado;
}
//Ejercicio 1
void tam_moles(uint8_t * tams){
  tams[0]=28;
  tams[1]=20;
  tams[2]=22;
}
/* Número de Moles con Ley de Gases Ideales */
void calc_moles_fx(uint32_t Vp_fx,uint32_t Vt_fx,uint32_t *resultado) {

    uint8_t tams[3];
    tam_moles(tams);
    uint32_t esc_Vp=tams[0];
    uint32_t esc_Vt=tams[1];

    //Generamos las variables de entrada para los cálculos en coma flotante  
    float Vp=(float)Vp_fx/(1<<esc_Vp);
    float Vt=(float)Vt_fx/(1<<esc_Vt);

    //Cálculos en coma flotante (copiados desde comaflotante.c)
    const float R = 8.314;    // J/(mol·K)
    const float V = 0.0025;   // m^3 (2.5 L)
    float Vp_offset = (Vp - 0.2);
    float ratio = 700.0 / 4.5;
    float P_kPa = Vp_offset * ratio;
    float P = P_kPa*1000; 
    float Tc = Vt/10;
    float T = Tc+273.15;
    float n = P*V/(R*T);

    //Cálculos en punto fijo 
    const uint32_t R_fx = round(8.314 * (1<<22)); //R_fx 22 parte decimal
    const uint32_t V_fx = round(0.0025 * ((uint64_t)1<<(uint64_t)(22))); //V_fx 22 parte decimal   
    const uint32_t K_ADJ = round(273.15 * (1<<22));  //K_ADJ 22 parte decimal
    
    uint32_t Vp_offset_fx = Vp_fx-round(0.2*(1<<28));//Vp_offset_fx 28 parte decimal
    uint32_t ratio_fx = round(700/4.5 * (1<<22));//ratio_fx 22 parte decimal xxxx
    uint32_t P_kPa_fx = FX_MUL(ratio_fx, Vp_offset_fx, 28); //p_kPa_fx 22 parte decimal
    uint32_t P_fx = FX_MUL(P_kPa_fx,1000,10);//P_fx 12 parte decimal
    uint32_t Tc_fx = FX_DIV(Vt_fx,10,2);//Tc_fx 22 parte decimal
    uint32_t T_fx = Tc_fx +K_ADJ; //T_fx 22 parte decimal
    uint32_t n_fx = FX_DIV(FX_MUL(P_kPa_fx,V_fx,22)*1000,FX_MUL(R_fx,T_fx,22),22);
  
    printf("Escalado Vp: %d     Escalado Vt: %d\n",esc_Vp,esc_Vt);
    printf("Vp:        %f    (guarda %.2f bits)\n",Vp,32-log2(Vp_fx));
    printf("Vt:        %f    (guarda %.2f bits)\n",Vt,32-log2(Vt_fx));    
    printf("Vp_offset: %f,%f (guarda %.2f bits)\n",Vp_offset,(float)Vp_offset_fx/(1<<28),32-log2(Vp_offset_fx));    
    printf("ratio:     %f,%f (guarda %.2f bits)\n",ratio,(float)ratio_fx/(1<<22),32-log2(ratio_fx));    
    printf("P_kPa:     %f,%f (guarda %.2f bits)\n",P_kPa,(float)P_kPa_fx/(1<<22),32-log2(P_kPa_fx));    
    printf("P:         %f,%f (guarda %.2f bits)\n",P,(float)P_fx/(1<<12),32-log2(P_fx));
    printf("Tc:        %f,%f (guarda %.2f bits)\n",Tc,(float)Tc_fx/(1<<22),32-log2(Tc_fx));
    printf("T:         %f,%f (guarda %.2f bits)\n",T,(float)T_fx/(1<<22),32-log2(T_fx));    
    printf("n:         %f,%f (guarda %.2f bits)\n",n,(float)n_fx/(1<<22),32-log2(n_fx));
        
    *resultado = n_fx;        
}

//Ejercicio 2
void escalado (uint8_t * escal){
  escal[0]=0;
  escal[1]=10;
  escal[2]=19;
}
/* Densidad del Aire con Presión y Temperatura */
void calc_density_fx(uint32_t dato_P, uint32_t dato_Vt,uint32_t *resultado) {
    
    //Escalado entrada para coma flotante
    uint8_t escal[3];
    escalado(escal);
    uint32_t esc_dato_P=escal[0];
    uint32_t esc_dato_Vt=escal[1];
    uint32_t esc_salida=escal[2];
    float P_raw=(float)dato_P/(1<<esc_dato_P);
    float Vt=(float)dato_Vt/(1<<esc_dato_Vt);

    //Calculos coma flotante
    const float C0 = 50000.0;
    const float C1 = 7500.0;
    const float R = 287.0;    // J/(kg·K)
    float P = (P_raw-C0)/C1;
    float Tc =(Vt-500)/10;
    float T = Tc+273.15;
    float rho = P/(R*T);
    
    //Cálculos en punto fijo
    const uint32_t C0_fx = 50000; //C0_fx 0 parte decimal
    const uint32_t C1_fx = 7500;  //C1_fx 0 parte decimal
    const uint32_t R_fx = 287; //R_fx 0 parte decimal
    uint32_t P_fx = FX_DIV((dato_P-(C0_fx<<esc_dato_P)), C1_fx, 19); //20 bits decimales
    int32_t Tc_fx;
    if(Vt< 500){
      int32_t aux = ~(((500<<esc_dato_Vt)-dato_Vt)/10)+1;
      Tc_fx= aux;
    }else{
      Tc_fx = FX_DIV((dato_Vt-(500<<esc_dato_Vt)),10, 0); //Esc_dato_vt bits decimales
    }
    uint32_t T_fx = Tc_fx + round((273.15)*(1<<esc_dato_Vt)); //Esc_dato_vt bits decimales
    uint32_t rho_fx = FX_DIV(P_fx,FX_MUL(R_fx,T_fx,0), esc_dato_Vt);//20 bits decimales
    

    printf("Escalado P: %d     Escalado Vt: %d\n",esc_dato_P,esc_dato_Vt);
    printf("P_raw:        %f    (guarda %.2f bits)\n",P_raw,32-log2(P_raw));
    printf("Vt:        %f    (guarda %.2f bits)\n",Vt,32-log2(dato_Vt));    
    printf("C0: %f,%f (guarda %.2f bits)\n",C0,(float)C0_fx/(1<<0),32-log2(C0_fx));    
    printf("C1:     %f,%f (guarda %.2f bits)\n",C1,(float)C1_fx/(1<<0),32-log2(C1_fx));    
    printf("R:     %f,%f (guarda %.2f bits)\n",R,(float)R_fx/(1<<0),32-log2(R_fx));    
    printf("P:         %f,%f (guarda %.2f bits)\n",P,(float)P_fx/(1<<19),32-log2(P_fx));
    printf("Tc:        %f,%f (guarda %.2f bits)\n",Tc,(float)Tc_fx/(1<<esc_dato_Vt),32-log2((uint32_t)Tc_fx));
    printf("T:         %f,%f (guarda %.2f bits)\n",T,(float)T_fx/(1<<esc_dato_Vt),32-log2(T_fx));   
    printf("rho:         %f,%f (guarda %.2f bits)\n",rho,(float)rho_fx/(1<<esc_salida),32-log2(rho_fx));
        
  *resultado = rho_fx;
}

  //ADXL345: 10 bits complemento a dos
  //HX711: 24 bits complemento a dos
//Ejercicio 3
void escalado_k (uint8_t * escal_k){
  escal_k[0]=16;
  escal_k[1]=0;
  escal_k[2]=20;
}
/* Energía Cinética con Aceleración y Peso */
void calc_kinetic_energy_fx(uint32_t dato_A, uint32_t dato_W,uint32_t *resultado) {
    //Escalado entrada para coma flotante
    uint8_t escal_k[3];
    escalado_k(escal_k);
    uint32_t esc_dato_A=escal_k[0];
    uint32_t esc_dato_W=escal_k[1];
    uint32_t esc_salida=escal_k[2];
    float A_raw=(float)dato_A/(1<<esc_dato_A);
    float W_raw=(float)dato_W/(1<<esc_dato_W);

    //Calculos coma flotante
    const float t = 1.0;      // Tiempo fijo en s
    float A_g =(A_raw*4)/1024;
    float A = A_g * 9.81;   // Aceleración en m/s²
    float v = A*t;          // Velocidad en m/s
    float W_g = (W_raw-8388608)/16777;
    float m = W_g / 1000;
    float Ek = (m*(v*v))/2;

    //Cálculos en punto fijo
    uint32_t A_g_fx = dato_A>>8; //esc_dato_A bits decimales
    uint32_t A_fx = (A_g_fx*981)/100; //esc_dato_A bits decimales; 
    uint32_t v_fx = A_fx; //esc_dato_A bits decimales

    uint32_t W_g_fx= FX_DIV((dato_W-8388608),16777,12); //12 bits decimales
    uint32_t m_fx = FX_DIV(W_g_fx,1000,8); //20 bits decimales
 
    uint32_t Ek_fx = FX_MUL(m_fx,FX_MUL(v_fx,v_fx,esc_dato_A),esc_dato_A+1);//20 bits decimales

    printf("Escalado A: %d     Escalado W: %d\n",esc_dato_A,esc_dato_W);
    printf("A_raw:        %f    (guarda %.2f bits)\n",A_raw,32-log2(A_raw));
    printf("W_raw:        %f    (guarda %.2f bits)\n",W_raw,32-log2(W_raw));    
    printf("t: %f,%f (guarda %.2f bits)\n",t,(float)1,32-log2(1));    
    printf("A_g:     %f,%f (guarda %.2f bits)\n",A_g,(float)A_g_fx/(1<<esc_dato_A),32-log2(A_g_fx));    
    printf("A:     %f,%f (guarda %.2f bits)\n",A,(float)A_fx/(1<<esc_dato_A),32-log2(A_fx));    
    printf("v:         %f,%f (guarda %.2f bits)\n",v,(float)v_fx/(1<<esc_dato_A),32-log2(v_fx));
    printf("W_g:        %f,%f (guarda %.2f bits)\n",W_g,(float)W_g_fx/(1<<(12)),32-log2(W_g_fx));    
    printf("m:         %f,%f (guarda %.2f bits)\n",m,(float)m_fx/(1<<(20)),32-log2(m_fx));   
    printf("Ek:         %f,%f (guarda %.2f bits)\n",Ek,(float)Ek_fx/(1<<esc_salida),32-log2(Ek_fx));
        
  *resultado = Ek_fx;

}  


//Implementación en coma flotante
//gcc comaflotante.c -o comaflotante -lm

/* Número de Moles con Ley de Gases Ideales */
float calc_moles(float Vp, float Vt) {
    const float R = 8.314;    // J/(mol·K)
    const float V = 0.0025;   // m^3 (2.5 L)

    float Vp_offset = (Vp - 0.2);
    float ratio = 700.0 / 4.5;
    float P_kPa = Vp_offset * ratio;
    float P = P_kPa*1000; 

    float Tc = Vt/10;
    float T = Tc+273.15;

    float n = P*V/(R*T);
    return n;
}

/* Densidad del Aire con Presión y Temperatura */
float calc_density(float P_raw, float Vt) {
    const float C0 = 50000.0;
    const float C1 = 7500.0;
    const float R = 287.0;    // J/(kg·K)

    float P = (P_raw-C0)/C1;

    float Tc =(Vt-500)/10;
    float T = Tc+273.15;

    float rho = P/(R*T);
    return rho;
}

/* Energía Cinética con Aceleración y Peso */
float calc_kinetic_energy(float A_raw, float W_raw) {
    const float t = 1.0;      // Tiempo fijo en s

    float A_g =(A_raw*4)/1024;
    float A = A_g * 9.81;   // Aceleración en m/s²
    float v = A*t;          // Velocidad en m/s

    float W_g = (W_raw-8388608)/16777;
    float m = W_g / 1000;

    float Ek = (m*(v*v))/2;
    return Ek;
}
