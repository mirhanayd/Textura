// Advanced cloth simulation with wind, sphere collision, and software rasterization
// Single-file implementation for course submission. Uses Image.h API (ColorImage)

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <cstring>
#include "Image.h"
#include <array>

using namespace std;

// --- Utility Vec3 ---
struct Vec3 {
    float x, y, z;
    Vec3(): x(0), y(0), z(0) {}
    Vec3(float X, float Y, float Z=0.0f): x(X), y(Y), z(Z) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x,y+o.y,z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x,y-o.y,z-o.z); }
    Vec3 operator*(float s) const { return Vec3(x*s,y*s,z*s); }
    Vec3 operator/(float s) const { return Vec3(x/s,y/s,z/s); }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const { return Vec3(y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x); }
    float length() const { return sqrtf(x*x + y*y + z*z); }
    Vec3 normalized() const { float L = length(); return L==0? Vec3(0,0,0) : (*this)/L; }
};

// --- CONFIG ---
const float GRAVITY = 0.5f;
const float DAMPING = 0.99f;
const int ITERATIONS = 8;

struct Particle {
    float x,y,z; // position
    float oldX, oldY, oldZ; // previous position (Verlet)
    bool isPinned;
    // normal for lighting / wind
    float nx, ny, nz;

    Particle(float px=0,float py=0,float pz=0,bool pinned=false)
      : x(px), y(py), z(pz), oldX(px), oldY(py), oldZ(pz), isPinned(pinned), nx(0), ny(0), nz(0) {}
};

struct Constraint {
    int p1, p2;
    float restLength;
    Constraint(int a,int b,float r): p1(a), p2(b), restLength(r) {}
};

struct Sphere { float x,y,z,radius; };

class TexturaSimulation {
public:
    int gridW, gridH;
    vector<Particle> particles;
    vector<Constraint> constraints;
    Sphere sphere;
    float timeSec = 0.0f;

    TexturaSimulation(int gw, int gh, float spacing, int canvasW, int canvasH) {
        gridW = gw; gridH = gh;
        // construct cloth centered roughly
        for (int r=0;r<gridH;r++){
            for (int c=0;c<gridW;c++){
                bool pinned = (r==0 && (c==0 || c==gridW/2 || c==gridW-1));
                float px = 200.0f + c*spacing;
                float py = 50.0f + r*spacing;
                float pz = 0.0f; // flat cloth initially
                particles.emplace_back(px,py,pz,pinned);
            }
        }
        for (int r=0;r<gridH;r++){
            for (int c=0;c<gridW;c++){
                if (c < gridW-1) constraints.emplace_back(r*gridW+c, r*gridW+(c+1), spacing);
                if (r < gridH-1) constraints.emplace_back(r*gridW+c, (r+1)*gridW+c, spacing);
            }
        }

        // place sphere slightly behind the cloth in z
        sphere.x = canvasW * 0.5f;
        sphere.y = canvasH * 0.5f + 40.0f;
        sphere.z = 30.0f; // towards camera
        sphere.radius = 80.0f;
    }

    void applyVerlet(float dt=1.0f) {
        for (auto &p : particles) {
            if (p.isPinned) continue;
            float vx = (p.x - p.oldX) * DAMPING;
            float vy = (p.y - p.oldY) * DAMPING;
            float vz = (p.z - p.oldZ) * DAMPING;
            p.oldX = p.x; p.oldY = p.y; p.oldZ = p.z;
            p.x += vx;
            p.y += vy + GRAVITY * dt;
            p.z += vz; // gravity only in y for cloth
        }
    }

    void satisfyConstraints() {
        for (int i=0;i<ITERATIONS;i++){
            for (auto &c : constraints){
                Particle &a = particles[c.p1];
                Particle &b = particles[c.p2];
                Vec3 pa(a.x,a.y,a.z), pb(b.x,b.y,b.z);
                Vec3 d = pb - pa;
                float dist = d.length(); if (dist==0) continue;
                float diff = (c.restLength - dist)/dist;
                Vec3 ofs = d * (0.5f * diff);
                if (!a.isPinned) { a.x -= ofs.x; a.y -= ofs.y; a.z -= ofs.z; }
                if (!b.isPinned) { b.x += ofs.x; b.y += ofs.y; b.z += ofs.z; }
            }
        }
    }

    void collisionSphere(float epsilon=0.01f) {
        for (auto &p : particles) {
            Vec3 pos(p.x,p.y,p.z);
            Vec3 center(sphere.x,sphere.y,sphere.z);
            Vec3 V = pos - center;
            float D = V.length();
            float minD = sphere.radius + epsilon;
            if (D < minD) {
                Vec3 n = (D==0? Vec3(0,1,0) : V / D);
                Vec3 newPos = center + n * minD;
                // simple friction: damp velocity via old position
                float vx = (p.x - p.oldX) * 0.8f;
                float vy = (p.y - p.oldY) * 0.8f;
                float vz = (p.z - p.oldZ) * 0.8f;
                p.x = newPos.x; p.y = newPos.y; p.z = newPos.z;
                p.oldX = p.x - vx * 0.5f; p.oldY = p.y - vy * 0.5f; p.oldZ = p.z - vz * 0.5f;
            }
        }
    }

    void computeNormals() {
        // reset
        for (auto &p: particles) { p.nx = p.ny = p.nz = 0.0f; }
        // accumulate triangle normals
        for (int r=0;r<gridH-1;r++){
            for (int c=0;c<gridW-1;c++){
                int i0 = r*gridW + c;
                int i1 = r*gridW + (c+1);
                int i2 = (r+1)*gridW + c;
                int i3 = (r+1)*gridW + (c+1);
                // tri A: i0, i2, i1
                addTriNormal(i0,i2,i1);
                // tri B: i1, i2, i3
                addTriNormal(i1,i2,i3);
            }
        }
        // normalize
        for (auto &p: particles) {
            Vec3 n(p.nx,p.ny,p.nz);
            Vec3 nn = n.normalized(); p.nx = nn.x; p.ny = nn.y; p.nz = nn.z;
        }
    }

    void addTriNormal(int ia,int ib,int ic){
        Particle &A = particles[ia]; Particle &B = particles[ib]; Particle &C = particles[ic];
        Vec3 v1(B.x - A.x, B.y - A.y, B.z - A.z);
        Vec3 v2(C.x - A.x, C.y - A.y, C.z - A.z);
        Vec3 n = v1.cross(v2);
        // accumulate to vertices
        A.nx += n.x; A.ny += n.y; A.nz += n.z;
        B.nx += n.x; B.ny += n.y; B.nz += n.z;
        C.nx += n.x; C.ny += n.y; C.nz += n.z;
    }

    void applyWind(float dt) {
        // wind vector that changes over time
        Vec3 W(sinf(timeSec*2.0f) + 0.5f, cosf(timeSec), sinf(timeSec));
        float strength = 3.5f;
        for (auto &p: particles) {
            if (p.isPinned) continue;
            Vec3 N(p.nx,p.ny,p.nz);
            float fac = max(0.0f, N.normalized().dot(W.normalized()));
            // move particle by wind projected onto normal
            p.x += N.x * fac * 0.03f * strength;
            p.y += N.y * fac * 0.03f * strength;
            p.z += N.z * fac * 0.03f * strength;
        }
    }

    void update() {
        timeSec += 0.06f;
        applyVerlet();
        computeNormals();
        applyWind(0.06f);
        satisfyConstraints();
        collisionSphere(0.5f);
    }

    // triangle generation for drawing
    void getTriangles(vector<array<int,3>>& tris) {
        tris.clear();
        for (int r=0;r<gridH-1;r++){
            for (int c=0;c<gridW-1;c++){
                int i0 = r*gridW + c;
                int i1 = r*gridW + (c+1);
                int i2 = (r+1)*gridW + c;
                int i3 = (r+1)*gridW + (c+1);
                std::array<int,3> t1 = {i0,i2,i1};
                std::array<int,3> t2 = {i1,i2,i3};
                tris.push_back(t1);
                tris.push_back(t2);
            }
        }
    }
};

// --- Rasterizer ---
struct Light { Vec3 dir; };

// Barycentric helper
static void barycentric(float x1,float y1,float x2,float y2,float x3,float y3,float px,float py,float &a,float &b,float &c){
    float denom = (y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3);
    if (fabsf(denom) < 1e-6f) { a=b=c=-1.0f; return; }
    a = ((y2 - y3)*(px - x3) + (x3 - x2)*(py - y3)) / denom;
    b = ((y3 - y1)*(px - x3) + (x1 - x3)*(py - y3)) / denom;
    c = 1.0f - a - b;
}

// draw triangle with flat shading and z-buffer
void drawTriangle(ColorImage& img, const Particle &p1, const Particle &p2, const Particle &p3, Light light, vector<float> &zbuf) {
    int w = img.GetWidth(), h = img.GetHeight();
    // project (orthographic)
    float x1 = p1.x, y1 = p1.y, z1 = p1.z;
    float x2 = p2.x, y2 = p2.y, z2 = p2.z;
    float x3 = p3.x, y3 = p3.y, z3 = p3.z;

    // compute triangle normal in 3D for lighting
    Vec3 v1(x2-x1,y2-y1,z2-z1), v2(x3-x1,y3-y1,z3-z1);
    Vec3 N = v1.cross(v2).normalized();
    Vec3 L = light.dir.normalized();
    float intensity = max(0.0f, N.dot(L));
    // base color white scaled by intensity
    unsigned char col = (unsigned char)max(20.0f, min(255.0f, 255.0f * intensity));

    // bounding box
    int minX = max(0, (int)floorf(min(min(x1,x2),x3)));
    int maxX = min(w-1, (int)ceilf(max(max(x1,x2),x3)));
    int minY = max(0, (int)floorf(min(min(y1,y2),y3)));
    int maxY = min(h-1, (int)ceilf(max(max(y1,y2),y3)));

    for (int py=minY; py<=maxY; ++py) {
        for (int px=minX; px<=maxX; ++px) {
            float a,b,cw; barycentric(x1,y1,x2,y2,x3,y3,(float)px+0.5f,(float)py+0.5f,a,b,cw);
            if (a >= -1e-6f && b >= -1e-6f && cw >= -1e-6f) {
                float pixelZ = a*z1 + b*z2 + cw*z3;
                int idx = py * w + px;
                if (pixelZ > zbuf[idx]) {
                    zbuf[idx] = pixelZ;
                    auto &pix = img(px,py);
                    pix.r = col; pix.g = col; pix.b = col; pix.a = 255;
                }
            }
        }
    }
}

int main() {
    cout << "--- Textura Dynamics Simulasyonu (advanced) Basliyor ---" << endl;
    int canvasW = 800, canvasH = 600;
    ColorImage img(canvasW, canvasH);

    TexturaSimulation sim(40, 40, 8.0f, canvasW, canvasH);
    Light light; light.dir = Vec3(0.0f, -0.5f, -1.0f);

    for (int frame=0; frame<300; ++frame) {
        sim.update();

        // clear and z-buffer
        vector<float> zbuf(canvasW * canvasH, -1e9f);
        for (int y=0;y<canvasH;y++) for (int x=0;x<canvasW;x++){ auto &p = img(x,y); p.r=30; p.g=30; p.b=30; p.a=255; }

        // compute per-triangle and draw
        vector<array<int,3>> tris; sim.getTriangles(tris);
        for (auto &t : tris) {
            drawTriangle(img, sim.particles[t[0]], sim.particles[t[1]], sim.particles[t[2]], light, zbuf);
        }

        // save frame
        ostringstream ss; ss << "outputs/frame_" << setw(3) << setfill('0') << frame << ".png";
        string name = ss.str(); img.Save(name);
        cout << name << " kaydedildi." << endl;
    }

    cout << "Islem tamam. 300 kare olusturuldu." << endl;
    return 0;
}
