# Textura Simulation

## Türkçe Açıklama

### Proje Hakkında
Bu proje, kumaş simülasyonu ve kare kare PNG çıktısı üretimi yapar. Sonrasında bu karelerden video oluşturabilirsiniz.

### Derleme ve Çalıştırma

#### 1. Linux/WSL/VSCode Terminali
1. Proje klasörüne girin:
   ```bash
   cd /path/to/Textura
   ```
2. Aşağıdaki komutları sırasıyla çalıştırın:
   ```bash
   mkdir -p build/obj bin
   for f in PNG/*.c; do gcc -I. -I./PNG -I./zlib -O2 -c $f -o build/obj/$(basename $f .c).o; done
   for f in zlib/*.c; do gcc -I. -I./PNG -I./zlib -O2 -c $f -o build/obj/$(basename $f .c).o; done
   g++ -I. -I./PNG -I./zlib -O2 -c main.cpp -o build/obj/main.o
   g++ build/obj/*.o -lpng -lz -o bin/textura_exe
   ./bin/textura_exe
   ```
3. Çıktı kareleri `outputs/` klasöründe oluşur.
4. Video yapmak için (ffmpeg yüklü olmalı):
   ```bash
   ffmpeg -framerate 30 -i outputs/frame_%03d.png -c:v libx264 -pix_fmt yuv420p final_simulasyon.mp4
   ```

#### 2. Windows + Visual Studio
- Projeyi "Console Application" olarak açın.
- `main.cpp`, `Image.h` ve ilgili tüm PNG/zlib dosyalarını projeye ekleyin.
- Derleyiciye `PNG` ve `zlib` klasörlerini include path olarak ekleyin.
- Derleme ayarlarında C++17 veya üstü seçin.
- Çıktı dosyası `bin\textura_exe.exe` olarak ayarlanabilir.

#### 3. Windows + VSCode (MinGW/MSYS2 ile)
- Terminalde yukarıdaki Linux/WSL adımlarını uygulayabilirsiniz.
- MinGW/MSYS2 yüklü olmalı ve `gcc`, `g++` komutları terminalde çalışmalı.

#### 4. Makefile ile (Linux/WSL/VSCode)
- Sadece şunu yazmanız yeterli:
   ```bash
   make run
   ```
- Klasörler otomatik oluşur, derleme ve çalıştırma tek komutla yapılır.

---

## English Instructions

### About the Project
This project simulates a cloth and generates PNG frames. You can combine these frames into a video.

### Compile and Run

#### 1. Linux/WSL/VSCode Terminal
1. Enter the project folder:
   ```bash
   cd /path/to/Textura
   ```
2. Run these commands in order:
   ```bash
   mkdir -p build/obj bin
   for f in PNG/*.c; do gcc -I. -I./PNG -I./zlib -O2 -c $f -o build/obj/$(basename $f .c).o; done
   for f in zlib/*.c; do gcc -I. -I./PNG -I./zlib -O2 -c $f -o build/obj/$(basename $f .c).o; done
   g++ -I. -I./PNG -I./zlib -O2 -c main.cpp -o build/obj/main.o
   g++ build/obj/*.o -lpng -lz -o bin/textura_exe
   ./bin/textura_exe
   ```
3. Output frames will be in the `outputs/` folder.
4. To create a video (requires ffmpeg):
   ```bash
   ffmpeg -framerate 30 -i outputs/frame_%03d.png -c:v libx264 -pix_fmt yuv420p final_simulasyon.mp4
   ```

#### 2. Windows + Visual Studio
- Open as a "Console Application" project.
- Add `main.cpp`, `Image.h`, and all PNG/zlib files to the project.
- Add `PNG` and `zlib` folders to the include path.
- Set C++17 or higher as the language standard.
- Output file can be set as `bin\textura_exe.exe`.

#### 3. Windows + VSCode (with MinGW/MSYS2)
- You can use the same steps as Linux/WSL above.
- Make sure `gcc` and `g++` are available in your terminal.

#### 4. With Makefile (Linux/WSL/VSCode)
- Just run:
   ```bash
   make run
   ```
- All folders will be created automatically, build and run in one step.

---

**Not:** PNG ve zlib kaynak dosyalarının orijinal ve uyumlu olması gereklidir. Hatalı derleme durumunda orijinal kaynakları tekrar indirip kullanın.

**Note:** PNG and zlib source files must be original and compatible. If you get build errors, re-download and use the official sources.
