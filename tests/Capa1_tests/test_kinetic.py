import pytest
import ctypes
import os

punto_fijo = True

# Ruta dinámica
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
so_path = os.path.abspath(os.path.join(BASE_DIR, "../../src/Capa1_Datos/datos.so"))

class Dato(ctypes.Structure):
    _fields_ = [
        ("coma_flotante", ctypes.c_float),
        ("punto_fijo", ctypes.c_uint32),
        ("num_bits_escala", ctypes.c_uint8)
    ]

if punto_fijo:
    if not os.path.exists(so_path):
        raise FileNotFoundError(f"No se encontró el archivo {so_path}")
    
    lib = ctypes.CDLL(so_path, mode=ctypes.RTLD_GLOBAL)
    
    lib.calc_kinetic_energy_fx.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_uint32)]
    lib.calc_kinetic_energy_fx.restype = None

    lib.escalado_k.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
    lib.escalado_k.restype = None

    escalado_array = (ctypes.c_uint8 * 3)()
    lib.escalado_k(escalado_array)
    escalado_k = [escalado_array[i] for i in range(3)]
else:
    if not os.path.exists(so_path):
        raise FileNotFoundError(f"No se encontró el archivo {so_path}")
    lib = ctypes.CDLL(so_path)
    lib.calc_kinetic_energy.argtypes = [ctypes.c_float, ctypes.c_float]
    lib.calc_kinetic_energy.restype = ctypes.c_float

def leer_datos():
    data = []
    # Ruta dinámica para el txt
    txt_path = os.path.join(BASE_DIR, "datos_cinetica.txt")
    
    with open(txt_path, "r") as f:
        for line in f:
            if line.strip() and not line.startswith("#"):
                A_raw, W_raw, result = map(float, line.split())
                if punto_fijo:
                    A_raw_fx = int((1 << escalado_k[0]) * A_raw)
                    W_raw_fx = int((1 << escalado_k[1]) * W_raw)
                    result_fx = int((1 << escalado_k[2]) * result)
                    data.append((A_raw_fx, W_raw_fx, result_fx))
                else:
                    data.append((A_raw, W_raw, result))
    return data

@pytest.mark.parametrize("A_raw_fx, W_raw_fx, expected_fx",
    [pytest.param(A_raw, W_raw, expected, id=f"A_raw={A_raw}, W_raw={W_raw}")
     for A_raw, W_raw, expected in leer_datos()])
def test_calc_kinetic(A_raw_fx, W_raw_fx, expected_fx):
    if punto_fijo:
        dato_A_raw_fx = ctypes.c_uint32(A_raw_fx)
        dato_W_raw_fx = ctypes.c_uint32(W_raw_fx)
        resultado = ctypes.c_uint32()
        
        lib.calc_kinetic_energy_fx(dato_A_raw_fx, dato_W_raw_fx, ctypes.byref(resultado))
    
        result_fx = resultado.value        
        result_float = result_fx / (1 << escalado_k[2])
        expected_float = expected_fx / (1 << escalado_k[2])
    else:
        result_float = lib.calc_kinetic_energy(A_raw_fx, W_raw_fx)
        expected_float = expected_fx
    
    if abs(expected_float) > 0:
        relativo = abs(result_float - expected_float) / expected_float
    else:
        relativo = 0
    
    tolerancia = 0.05
    assert relativo < tolerancia, \
        f"Fallo: A_raw={A_raw_fx}, W_raw={W_raw_fx}, Esperado={expected_float}, Obtenido={result_float}, Error relativo={relativo:.4f}"

if __name__ == "__main__":
    if punto_fijo:
        print(f"Escalados: A_raw={escalado_k[0]}, W_raw={escalado_k[1]}, Resultado={escalado_k[2]}")