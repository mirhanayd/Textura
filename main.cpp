#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "Image.h"
#include <array>

using namespace std;

// Vektor islemleri
struct V3 {
    float x, y, z;
    V3(float a=0, float b=0, float c=0) : x(a), y(b), z(c) {}
    
    V3 operator+(const V3& o) const { return V3(x+o.x, y+o.y, z+o.z); }
    V3 operator-(const V3& o) const { return V3(x-o.x, y-o.y, z-o.z); }
    V3 operator*(float s) const { return V3(x*s, y*s, z*s); }
    V3 operator/(float s) const { return V3(x/s, y/s, z/s); }
    
    float dot(const V3& o) const { return x*o.x + y*o.y + z*o.z; }
    
    V3 cross(const V3& o) const { 
        return V3(y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x); 
    }
    
    float len() const { return sqrtf(x*x + y*y + z*z); }
    
    V3 norm() const { 
        float L = len(); 
        return L==0 ? V3(0,0,0) : (*this)/L; 
    }
};

struct Nokta {
    float x, y, z;       // su anki pozisyon
    float ex, ey, ez;    // eski pozisyon
    float nx, ny, nz;    //isik ve ruzgar
    bool kilit;          // sabit mi??

    Nokta(float a=0, float b=0, float c=0, bool k=false) 
        : x(a), y(b), z(c), ex(a), ey(b), ez(c), nx(0), ny(0), nz(0), kilit(k) {}
};

struct Bag { 
    int p1, p2; 
    float mesafe; 
};

class Simulasyon {
public:
    int gen, yuk;
    vector<Nokta> noktalar;
    vector<Bag> baglar;
    float zaman = 0.0f;

    Simulasyon(int g, int y, float aralik) : gen(g), yuk(y) {
        //kumasi olusturuoz
        for (int r = 0; r < yuk; r++) {
            for (int c = 0; c < gen; c++) {
                bool sabit = (r == 0 && (c == 0 || c == gen/2 || c == gen-1));
                noktalar.emplace_back(200.0f + c*aralik, 50.0f + r*aralik, 0.0f, sabit);
            }
        }
        //bağlantilari kur
        for (int r = 0; r < yuk; r++) {
            for (int c = 0; c < gen; c++) {
                if (c < gen - 1) baglar.push_back({r*gen + c, r*gen + c + 1, aralik});
                if (r < yuk - 1) baglar.push_back({r*gen + c, (r+1)*gen + c, aralik});
            }
        }
    }

    void guncelle() {
        zaman += 0.06f;

        // verlet
        for (auto &p : noktalar) {
            if (p.kilit) continue;
            
            float vx = (p.x - p.ex) * 0.99f;
            float vy = (p.y - p.ey) * 0.99f;
            float vz = (p.z - p.ez) * 0.99f;
            
            p.ex = p.x; 
            p.ey = p.y; 
            p.ez = p.z;
            
            p.x += vx;
            p.y += vy + 0.5f; // Yercekiminiburda ayarrlıoz
            p.z += vz;
        }
        
        for (auto &p : noktalar) { p.nx = p.ny = p.nz = 0; }
        
        for (int r = 0; r < yuk - 1; r++) {
            for (int c = 0; c < gen - 1; c++) {
                int i0 = r*gen + c;
                int i1 = r*gen + c + 1;
                int i2 = (r+1)*gen + c;
                int i3 = (r+1)*gen + c + 1;
                
                //iki ucgen icin normal hesabi
                auto hesapla = [&](int a, int b, int c_idx) {
                    V3 v1(noktalar[b].x - noktalar[a].x, noktalar[b].y - noktalar[a].y, noktalar[b].z - noktalar[a].z);
                    V3 v2(noktalar[c_idx].x - noktalar[a].x, noktalar[c_idx].y - noktalar[a].y, noktalar[c_idx].z - noktalar[a].z);
                    V3 n = v1.cross(v2);
                    noktalar[a].nx += n.x; noktalar[a].ny += n.y; noktalar[a].nz += n.z;
                    noktalar[b].nx += n.x; noktalar[b].ny += n.y; noktalar[b].nz += n.z;
                    noktalar[c_idx].nx += n.x; noktalar[c_idx].ny += n.y; noktalar[c_idx].nz += n.z;
                };
                
                hesapla(i0, i2, i1);
                hesapla(i1, i2, i3);
            }
        }
        // normalize et
        for (auto &p : noktalar) {
            V3 n(p.nx, p.ny, p.nz);
            V3 nn = n.norm();
            p.nx = nn.x; p.ny = nn.y; p.nz = nn.z; 
        }
        //ruzgar
        V3 W(sinf(zaman * 2.0f) + 0.5f, cosf(zaman), sinf(zaman));
        for (auto &p : noktalar) {
            if (p.kilit) continue;
            
            V3 N(p.nx, p.ny, p.nz);
            float etki = max(0.0f, N.norm().dot(W.norm()));
            
            p.x += N.x * etki * 0.55f;
            p.y += N.y * etki * 0.55f;
            p.z += N.z * etki * 0.55f;
        }
        //baglanti kontrolu (constraints)
        for (int i = 0; i < 20; i++) {
            for (auto &b : baglar) {
                Nokta &p1 = noktalar[b.p1];
                Nokta &p2 = noktalar[b.p2];
                
                V3 delta(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
                float dist = delta.len();
                
                if (dist == 0) continue;
                
                float fark = (b.mesafe - dist) / dist;
                V3 duzeltme = delta * (0.5f * fark);
                
                if (!p1.kilit) { p1.x -= duzeltme.x; p1.y -= duzeltme.y; p1.z -= duzeltme.z; }
                if (!p2.kilit) { p2.x += duzeltme.x; p2.y += duzeltme.y; p2.z += duzeltme.z; }
            }
        }
    }
};
void ucgen_ciz(ColorImage& im, Nokta &p1, Nokta &p2, Nokta &p3, V3 isik, vector<float> &zb) {
    int w = im.GetWidth();
    int h = im.GetHeight();
    
    // yüzey normali ve isik siddeti
    V3 v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    V3 v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
    float yogunluk = max(0.0f, v1.cross(v2).norm().dot(isik.norm()));
    unsigned char renk = (unsigned char)max(20.0f, min(255.0f, 255.0f * yogunluk));

    // bounding box
    int minX = max(0, (int)floor(min({p1.x, p2.x, p3.x})));
    int maxX = min(w-1, (int)ceil(max({p1.x, p2.x, p3.x})));
    int minY = max(0, (int)floor(min({p1.y, p2.y, p3.y})));
    int maxY = min(h-1, (int)ceil(max({p1.y, p2.y, p3.y})));

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            // Barycentric
            float payda = (p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y);
            if (fabs(payda) < 1e-6) continue;
            
            float u = ((p2.y - p3.y)*(x - p3.x) + (p3.x - p2.x)*(y - p3.y)) / payda;
            float v = ((p3.y - p1.y)*(x - p3.x) + (p1.x - p3.x)*(y - p3.y)) / payda;
            float w_bar = 1.0f - u - v;
            
            if (u >= -1e-6 && v >= -1e-6 && w_bar >= -1e-6) {
                float z = u*p1.z + v*p2.z + w_bar*p3.z;
                if (z > zb[y*w + x]) {
                    zb[y*w + x] = z;
                    im(x,y).r = renk; 
                    im(x,y).g = renk; 
                    im(x,y).b = renk; 
                    im(x,y).a = 255;
                }
            }
        }
    }
}

int main() {
    int W = 800, H = 600;
    ColorImage img(W, H);
    Simulasyon sim(25, 25, 12.0f);     // 25x25 kumas 12.0 birim aralik
    V3 isik(0.0f, -0.5f, -1.0f);

    int fsayisi =300; // kare sayısını burdan ayarlıoruz

    for (int i = 0; i < fsayisi; ++i) {
        sim.guncelle();
        
        vector<float> zbuf(W*H, -1e9f);
        for(int k=0; k<W*H; k++) { 
            auto &p = img(k%W, k/W); 
            p.r = 30; p.g = 30; p.b = 30; p.a = 255; 
        }
        
        // çizimkısmı
        for (int r = 0; r < sim.yuk - 1; r++) {
            for (int c = 0; c < sim.gen - 1; c++) {
                int idx = r*sim.gen + c;
                ucgen_ciz(img, sim.noktalar[idx], sim.noktalar[idx+sim.gen], sim.noktalar[idx+1], isik, zbuf);
                ucgen_ciz(img, sim.noktalar[idx+1], sim.noktalar[idx+sim.gen], sim.noktalar[idx+sim.gen+1], isik, zbuf);
            }
        }

        stringstream ss; 
        ss << "outputs/frame_" << setw(3) << setfill('0') << i << ".png";
        img.Save(ss.str());
        
        std::cout << ss.str() << " saved." << std::endl;
    }
    std::cout << "Process finished." << fsayisi << " frames created." << std::endl;
    return 0;
}