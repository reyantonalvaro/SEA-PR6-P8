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
    
    lib.calc_density_fx.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_uint32)]
    lib.calc_density_fx.restype = None
    lib.escalado.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
    lib.escalado.restype = None

    esc_array = (ctypes.c_uint8 * 3)()
    lib.escalado(esc_array)
    escal = [esc_array[i] for i in range(3)]
else:
    if not os.path.exists(so_path):
        raise FileNotFoundError(f"No se encontró el archivo {so_path}")
    lib = ctypes.CDLL(so_path)
    lib.calc_density.argtypes = [ctypes.c_float, ctypes.c_float]
    lib.calc_density.restype = ctypes.c_float

def leer_datos():
    data = []
    # Ruta dinámica para el txt
    txt_path = os.path.join(BASE_DIR, "datos_densidad.txt")
    
    with open(txt_path, "r") as f:
        for line in f:
            if line.strip() and not line.startswith("#"):
                P, Vt, result = map(float, line.split())
                if punto_fijo:
                    dato_P = int((1 << escal[0]) * P)
                    dato_Vt = int((1 << escal[1]) * Vt)
                    result_fx = int((1 << escal[2]) * result)
                    data.append((dato_P, dato_Vt, result_fx))
                else:
                    data.append((P, Vt, result))
    return data

@pytest.mark.parametrize("dato_P, dato_Vt, expected_fx",
    [pytest.param(dato_P, dato_Vt, expected, id=f"P={dato_P}, Vt={dato_Vt}")
     for dato_P, dato_Vt, expected in leer_datos()])
def test_calc_density(dato_P, dato_Vt, expected_fx):
    if punto_fijo:
        dato_P = ctypes.c_uint32(dato_P)
        dato_Vt = ctypes.c_uint32(dato_Vt)
        resultado = ctypes.c_uint32()
        
        lib.calc_density_fx(dato_P, dato_Vt, ctypes.byref(resultado))
    
        result_fx = resultado.value      
        result_float = result_fx / (1 << escal[2])
        expected_float = expected_fx / (1 << escal[2])
    else:
        result_float = lib.calc_density(dato_P, dato_Vt)
        expected_float = expected_fx
    
    if abs(expected_float) > 0:
        relativo = abs(result_float - expected_float) / expected_float
    else:
        relativo = 0
    
    tolerancia = 0.05
    assert relativo < tolerancia, \
        f"Fallo: P={dato_P}, Vt={dato_Vt}, Esperado={expected_float}, Obtenido={result_float}, Error relativo={relativo:.4f}"

if __name__ == "__main__":
    if punto_fijo:
        print(f"Escalados: P={escal[0]}, Vt={escal[1]}, Resultado={escal[2]}")