import pytest
import ctypes
import os

punto_fijo = True

# Obtenemos la ruta absoluta de ESTE archivo (test_moles.py)
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Construimos la ruta absoluta al .so (subiendo 2 niveles hasta src)
so_path = os.path.join(BASE_DIR, "../../src/Capa1_Datos/datos.so")
so_path = os.path.abspath(so_path) # Normaliza la ruta

class Dato(ctypes.Structure):
    _fields_ = [
        ("coma_flotante", ctypes.c_float),
        ("punto_fijo", ctypes.c_uint32),
        ("num_bits_escala", ctypes.c_uint8)
    ]

if punto_fijo:
    if not os.path.exists(so_path):
        raise FileNotFoundError(f"No se encontró el archivo {so_path}")
    
    # Cargar con RTLD_GLOBAL para compartir símbolos si fuera necesario
    lib = ctypes.CDLL(so_path, mode=ctypes.RTLD_GLOBAL)
    
    lib.calc_moles_fx.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_uint32)]
    lib.calc_moles_fx.restype = None  
    lib.tam_moles.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
    lib.tam_moles.restype = None

    tams_array = (ctypes.c_uint8 * 3)()
    lib.tam_moles(tams_array)
    tams = [tams_array[i] for i in range(3)]
else:
    if not os.path.exists(so_path):
        raise FileNotFoundError(f"No se encontró el archivo {so_path}")
    lib = ctypes.CDLL(so_path)
    lib.calc_moles.argtypes = [ctypes.c_float, ctypes.c_float]
    lib.calc_moles.restype = ctypes.c_float

def leer_datos():
    data = []
    # Usamos BASE_DIR para encontrar el txt en la misma carpeta que el test
    txt_path = os.path.join(BASE_DIR, "datos_moles.txt")
    
    with open(txt_path, "r") as f:
        for line in f:
            if line.strip() and not line.startswith("#"):
                Vp, Vt, result = map(float, line.split())
                if punto_fijo:
                    # NOTA: Idealmente usar tams[0] y tams[1] en lugar de hardcodear 28/20
                    Vp_fx = int((1 << 28) * Vp)
                    Vt_fx = int((1 << 20) * Vt)
                    result_fx = int((1 << 22) * result)
                    data.append((Vp_fx, Vt_fx, result_fx))
                else:
                    data.append((Vp, Vt, result))
    return data

@pytest.mark.parametrize("Vp_fx, Vt_fx, expected_fx",
    [pytest.param(Vp, Vt, expected, id=f"Vp={Vp}, Vt={Vt}")
     for Vp, Vt, expected in leer_datos()])
def test_calc_moles(Vp_fx, Vt_fx, expected_fx):
    if punto_fijo:
        dato_Vp = ctypes.c_uint32(Vp_fx)
        dato_Vt = ctypes.c_uint32(Vt_fx)
        resultado = ctypes.c_uint32()
        
        lib.calc_moles_fx(dato_Vp, dato_Vt, ctypes.byref(resultado))
    
        result_fx = resultado.value     
        result_float = result_fx / (1 << tams[2])
        expected_float = expected_fx / (1 << tams[2])
    else:
        result_float = lib.calc_moles(Vp_fx, Vt_fx)
        expected_float = expected_fx
    
    if abs(expected_float) > 0:
        relativo = abs(result_float - expected_float) / expected_float
    else:
        relativo = 0
    
    tolerancia = 0.05
    assert relativo < tolerancia, \
        f"Fallo: Vp={Vp_fx}, Vt={Vt_fx}, Esperado={expected_float}, Obtenido={result_float}, Error relativo={relativo:.4f}"

if __name__ == "__main__":
    if punto_fijo:
        print(f"Escalados: Vp={tams[0]}, Vt={tams[1]}, Resultado={tams[2]}")