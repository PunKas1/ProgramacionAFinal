#!/bin/bash

# 1. Limpiar versiones anteriores para evitar conflictos
rm -rf build
mkdir build
cd build

# 2. Configurar el proyecto (esto bajará SFML 3 si usaste FetchContent)
cmake ..

# 3. Compilar usando todos los núcleos de tu procesador
cmake --build . -j $(nproc)

# 4. Volver a la raíz
cd ..

echo "------------------------------------------"
echo "¡Proceso terminado!"
echo "Ejecutables en: build/bin/"
echo "------------------------------------------"
