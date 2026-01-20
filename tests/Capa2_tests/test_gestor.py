import pytest
import ctypes
import os
import sys

# Definición del Enum igual que en C
EJERCICIO_MOLES = 0
EJERCICIO_DENSIDAD = 1
EJERCICIO_CINETICA = 2

# Rutas a las librerías compartidas
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../src"))
LIB_DATOS_PATH = os.path.join(BASE_DIR, "Capa1_Datos/datos.so")
LIB_GESTOR_PATH = os.path.join(BASE_DIR, "Capa2_Intermedia/gestor.so")

@pytest.fixture(scope="module")
def lib_gestor():
    """Carga la librería gestor.so vinculando primero datos.so"""
    print(f"\n[PY-SETUP] Buscando librerías en: {BASE_DIR}")
    if not os.path.exists(LIB_DATOS_PATH):
        pytest.fail(f"No se encontró datos.so en {LIB_DATOS_PATH}")
    if not os.path.exists(LIB_GESTOR_PATH):
        pytest.fail(f"No se encontró gestor.so en {LIB_GESTOR_PATH}")

    # Cargar datos.so con RTLD_GLOBAL para símbolos compartidos
    ctypes.CDLL(LIB_DATOS_PATH, mode=ctypes.RTLD_GLOBAL)
    lib = ctypes.CDLL(LIB_GESTOR_PATH)

    # Definir firmas
    lib.test_automatico.argtypes = [ctypes.c_int, ctypes.c_char_p]
    lib.test_automatico.restype = None

    lib.test_especifico.argtypes = [ctypes.c_int, ctypes.c_double, ctypes.c_double]
    lib.test_especifico.restype = None

    lib.crear_fichero.argtypes = [
        ctypes.c_int, 
        ctypes.c_char_p, 
        ctypes.POINTER(ctypes.c_double), 
        ctypes.POINTER(ctypes.c_double), 
        ctypes.POINTER(ctypes.c_double), 
        ctypes.c_int
    ]
    lib.crear_fichero.restype = None

    return lib

def test_crear_fichero_moles(lib_gestor, tmp_path):
    """Verifica la creación física del archivo"""
    print("\n\n=== [PY-TEST] Iniciando Test: Crear Fichero Moles ===")
    
    n = 3
    v1 = (ctypes.c_double * n)(1.5, 2.0, 3.5)
    v2 = (ctypes.c_double * n)(10.0, 20.0, 30.0)
    res = (ctypes.c_double * n)(0.5, 0.6, 0.7)
    
    archivo_salida = tmp_path / "salida_test_moles.txt"
    archivo_salida_str = str(archivo_salida).encode('utf-8')

    print(f"[PY-INFO] Generando archivo en: {archivo_salida}")
    lib_gestor.crear_fichero(EJERCICIO_MOLES, archivo_salida_str, v1, v2, res, n)

    assert archivo_salida.exists()
    print("[PY-CHECK] Archivo creado correctamente.")

    content = archivo_salida.read_text()
    print("[PY-DATA] Contenido del archivo:")
    print(content.strip())
    
    lineas = content.strip().split('\n')
    assert len(lineas) == 3
    vals = lineas[0].split()
    assert float(vals[0]) == 1.5

def test_test_automatico_ejecucion(lib_gestor, tmp_path, capfd):
    """Verifica test_automatico y muestra logs de C"""
    print("\n\n=== [PY-TEST] Iniciando Test: Ejecución Automática (Moles) ===")
    
    archivo_input = tmp_path / "input_test_auto.txt"
    archivo_input.write_text("1.0 200.0 0.1276\n2.35 298.0 0.3319")
    archivo_input_str = str(archivo_input).encode('utf-8')

    print(f"[PY-INFO] Ejecutando función C 'test_automatico' con fichero simulado...")
    lib_gestor.test_automatico(EJERCICIO_MOLES, archivo_input_str)

    # Capturamos y volvemos a imprimir para que el usuario lo vea
    captured = capfd.readouterr()
    print("\n--- SALIDA CAPTURADA DE C ---")
    print(captured.out)
    print("-----------------------------")
    
    # Assertions
    assert "Iniciando Test Automático: MOLES" in captured.out
    assert "Fin Test MOLES" in captured.out
    assert "[ OK ]" in captured.out or "[FALLO]" in captured.out

def test_test_especifico_no_crashea(lib_gestor, capfd):
    """Verifica test_especifico y muestra logs de C"""
    print("\n\n=== [PY-TEST] Iniciando Test: Cálculo Específico (Densidad) ===")
    
    val1, val2 = 101325.0, 2000.0
    print(f"[PY-INFO] Llamando a C con valores: P={val1}, Vt={val2}")
    
    lib_gestor.test_especifico(EJERCICIO_DENSIDAD, ctypes.c_double(val1), ctypes.c_double(val2))
    
    captured = capfd.readouterr()
    print("\n--- SALIDA CAPTURADA DE C ---")
    print(captured.out)
    print("-----------------------------")
    
    assert "Test Específico: DENSIDAD" in captured.out
    assert "Hex Calculado" in captured.out

def test_archivo_inexistente(lib_gestor, capfd):
    """Verifica gestión de errores en C"""
    print("\n\n=== [PY-TEST] Iniciando Test: Archivo Inexistente ===")
    
    print("[PY-INFO] Intentando abrir archivo falso...")
    lib_gestor.test_automatico(EJERCICIO_CINETICA, b"ruta/falsa/no_existe.txt")
    
    captured = capfd.readouterr()
    print("\n--- SALIDA CAPTURADA DE C ---")
    print(captured.out)
    print("-----------------------------")
    
    assert "ERROR" in captured.out or "No se pudo abrir" in captured.out