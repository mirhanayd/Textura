#!/bin/bash

# Orijinal zlib ve PNG kaynaklarını indir ve değiştir
echo "Orijinal zlib ve PNG kaynaklarını indiriyorum..."
curl -L -o zlib-1.2.13.tar.gz https://zlib.net/zlib-1.2.13.tar.gz
tar -xzf zlib-1.2.13.tar.gz
rm -rf zlib
mv zlib-1.2.13 zlib

curl -L -o libpng-1.6.40.tar.gz https://download.sourceforge.net/libpng/libpng-1.6.40.tar.gz
tar -xzf libpng-1.6.40.tar.gz
rm -rf PNG
mv libpng-1.6.40 PNG

# Emscripten kurulumu
echo "Emscripten kurulumu başlıyor..."
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Proje klasörüne dön
cd /mnt/c/Users/mirha/EUL_sftwr/4/4_fall/COMP465/Textura

# Derleme
echo "Derleme başlıyor..."
em++ main.cpp PNG/*.c zlib/*.c -I. -IPNG -Izlib -o textura.html -s USE_LIBPNG=1 -s USE_ZLIB=1 -O2

# Web sunucusu başlat
echo "Web sunucusu başlatılıyor..."
emrun --no_browser --port 8080 textura.html &

echo "Tamamlandı! Tarayıcıda http://localhost:8080/textura.html aç."