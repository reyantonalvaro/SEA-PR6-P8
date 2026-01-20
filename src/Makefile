#Definiciones directorios
DIR_DATOS   = Capa1_Datos
DIR_GESTOR  = Capa2_Intermedia
DIR_GUI     = Capa3_GUI

# Nombre del programa
TARGET = programa_final

# Compilador y Flags
CC = gcc
# Incluimos (-I) los directorios para que se encuentren los .h entre capas
CFLAGS = -I$(DIR_DATOS) -I$(DIR_GESTOR) -I$(DIR_GUI) $(shell pkg-config --cflags gtk+-3.0)
LIBS   = -lm $(shell pkg-config --libs gtk+-3.0)

# Regla por defecto
all: gui

# 1. DATOS
datos: $(DIR_DATOS)/datos.c $(DIR_DATOS)/datos.h
	$(CC) -c $(DIR_DATOS)/datos.c -o $(DIR_DATOS)/datos.o
	$(CC) -shared -fPIC -o $(DIR_DATOS)/datos.so $(DIR_DATOS)/datos.c -lm

# 2. GESTOR
gestor: $(DIR_GESTOR)/gestor.c $(DIR_GESTOR)/gestor.h
	# 1. Compilar objeto (necesita CFLAGS para encontrar datos.h)
	$(CC) -c $(DIR_GESTOR)/gestor.c -o $(DIR_GESTOR)/gestor.o $(CFLAGS)
	# 2. Compilar biblioteca compartida
	$(CC) -shared -fPIC -o $(DIR_GESTOR)/gestor.so $(DIR_GESTOR)/gestor.c -lm $(CFLAGS) $(LIBS)

# 3. GUI
gui: $(DIR_GUI)/gui.c datos gestor
	# Copiamos la interfaz.glade a la raíz para que el ejecutable la encuentre al correr
	cp $(DIR_GUI)/interfaz.glade ../
	# Compilamos el ejecutable final
	$(CC) -rdynamic -o ../$(TARGET) $(DIR_GUI)/gui.c $(DIR_DATOS)/datos.o $(DIR_GESTOR)/gestor.o $(CFLAGS) $(LIBS)

# Limpieza
clean:
	rm -f ../$(TARGET) ../interfaz.glade
	rm -f $(DIR_DATOS)/*.o $(DIR_DATOS)/*.so
	rm -f $(DIR_GESTOR)/*.o $(DIR_GESTOR)/*.so
	rm -f $(DIR_GUI)/*.o $(DIR_GUI)/*.so
