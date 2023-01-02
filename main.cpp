#include <iostream>
#include <SDL.h>
#include <vector>
#include <fstream>
#include <strstream>

#define Width  600
#define Height 600

#define Offset 500

using namespace std;


struct vec3d {
    float x, y, z;
    vec3d(float a, float b, float c) {
        this->x = a;
        this->y = b;
        this->z = c;
    }
    vec3d() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }
};


struct vec2d {
    float x, y;
    vec2d(float a, float b) {
        this->x = a;
        this->y = b;
    }
    vec2d() {
        this->x = 0;
        this->y = 0;
    }
};

struct triangle {
    vec3d p[3];
    vec3d color;
    triangle(float x1, float y1, float z1, 
        float x2, float y2, float z2, 
        float x3, float y3, float z3) {
        this->p[0] = vec3d(x1, y1, z1);
        this->p[1] = vec3d(x2, y2, z2);
        this->p[2] = vec3d(x3, y3, z3);
        this->color = vec3d(255, 255, 255);
    }
    triangle(float x1, float y1, float z1, 
        float x2, float y2, float z2, 
        float x3, float y3, float z3, vec3d col) {
        this->p[0] = vec3d(x1, y1, z1);
        this->p[1] = vec3d(x2, y2, z2);
        this->p[2] = vec3d(x3, y3, z3);
        this->color = col;
    }
    triangle(vec3d a, vec3d b, vec3d c, vec3d col) {
        this->p[0] = a;
        this->p[1] = b;
        this->p[2] = c;
        this->color = col;
    }
    triangle() {
        for (int i = 0; i < 3; i++)
            this->p[i] = vec3d();
        this->color = vec3d(255, 255, 255);
    }
    vec3d center() {
        return vec3d((this->p[0].x + this->p[1].x + this->p[2].x) / 3.f,
            (this->p[0].y + this->p[1].y + this->p[2].y) / 3.f,
            (this->p[0].z + this->p[1].z + this->p[2].z) / 3.f);
    }
};

struct mesh {
    vector<triangle> tris;
    bool open(string sFilename)
    {
        ifstream f(sFilename);
        if (!f.is_open())
            return false;

        // Local cache of verts
        vector<vec3d> verts;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            strstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back(triangle(verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1], vec3d(255, 255, 255)));
            }
        }

        return true;
    }
};

int deltaTime(int previous, int offset) {
    return (clock() - previous) + offset;
}

void Render3Dto2D(triangle tri, vec3d camera, vec3d angle, vec2d *ret);
vec3d Light(vec3d light, float strength, triangle tri);

vec3d RotateY(vec3d c, float angle, vec3d p);

void DrawTriangle(vec2d tri[3], SDL_Renderer *renderer);
void DrawFillTriangle(vec2d tri[3], vec3d color, SDL_Renderer *renderer);

bool CollideRectPoint(vec2d point, vec2d r_start, vec2d r_size);
bool PointInLine(vec2d point, vec2d l_start, vec2d l_end);
vector<vec2d> CollideRectOutlineLine(vec2d l_start, vec2d l_end, vec2d r_start, vec2d r_size);

vec3d to_camera(vec3d p, vec3d camera, vec3d angle) {
    p.x -= camera.x;
    p.y -= camera.y;
    p.z -= camera.z - 4.f;
    return RotateY(vec3d(0, 0, 4), -angle.y, p);
}

int partition(vector<triangle> &arr, int start, int end, vec3d camera, vec3d angle) {
    float pivot = to_camera(arr[start].center(), camera, angle).z;
 
    int count = 0;
    for (int i = start + 1; i <= end; i++) {
        if (to_camera(arr[i].center(), camera, angle).z <= pivot)
            count++;
    }
 
    // Giving pivot element its correct position
    int pivotIndex = start + count;
    iter_swap(arr.begin() + pivotIndex, arr.begin() + start);
 
    // Sorting left and right parts of the pivot element
    int i = start, j = end;
 
    while (i < pivotIndex && j > pivotIndex) {
 
        while (to_camera(arr[i].center(), camera, angle).z <= pivot) {
            i++;
        }
 
        while (to_camera(arr[j].center(), camera, angle).z > pivot) {
            j--;
        }
 
        if (i < pivotIndex && j > pivotIndex) {
            iter_swap(arr.begin() + i++, arr.begin() + j--);
        }
    }
 
    return pivotIndex;
}
 
void quickSort(vector<triangle> &arr, int start, int end, vec3d camera, vec3d angle) {
 
    // base case
    if (start >= end)
        return;
 
    // partitioning the array
    int p = partition(arr, start, end, camera, angle);
 
    // Sorting the left part
    quickSort(arr, start, p - 1, camera, angle);
 
    // Sorting the right part
    quickSort(arr, p + 1, end, camera, angle);
}

bool OnUserCreate(mesh &m) {
    m.open("models/hand.obj");
    //m.tris.push_back(triangle(-1, -1, 1,  -1, 1, 1,  1, 1, 1, vec3d(255, 0, 0)));
    // m.tris.push_back(triangle(-1, -1, 1,  1, 1, 1,  1, -1, 1, vec3d(255, 0, 0)));
    // m.tris.push_back(triangle(-1, -1, -1,  -1, 1, -1,  1, 1, -1, vec3d(255, 0, 0)));
    // m.tris.push_back(triangle(-1, -1, -1,  1, 1, -1,  1, -1, -1, vec3d(255, 0, 0)));
    return true;
}

bool OnUserUpdate(SDL_Window *window, SDL_Renderer *renderer, mesh &m, int delta, vec3d camera, vec3d angle) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    quickSort(m.tris, 0, m.tris.size() - 1, camera, angle);
    // for (int j = 0; j < m.tris.size(); j++) {
    //     int id = 0;
    //     for (int i = 0; i < m.tris.size() - j; i++) {
    //         triangle tri = m.tris[i];
    //         for (int h = 0; h < 3; h++) {
    //             tri.p[h].x -= camera.x;
    //             tri.p[h].y -= camera.y;
    //             tri.p[h].z -= camera.z - 4.f;
    //             tri.p[h] = RotateY(vec3d(0, 0, 4), -angle.y, tri.p[h]);
    //         }
    //         triangle id_tri = m.tris[id];
    //         for (int h = 0; h < 3; h++) {
    //             id_tri.p[h].x -= camera.x;
    //             id_tri.p[h].y -= camera.y;
    //             id_tri.p[h].z -= camera.z - 4.f;
    //             id_tri.p[h] = RotateY(vec3d(0, 0, 4), -angle.y, id_tri.p[h]);
    //         }
    //         if (id_tri.p[0].z + id_tri.p[1].z + id_tri.p[2].z > tri.p[0].z + tri.p[1].z + tri.p[2].z)
    //             id = i;
    //     }
    //     triangle a = m.tris[id];
    //     m.tris.erase(m.tris.begin() + id);
    //     m.tris.push_back(a);
    // }

    for (int t = 0; t < m.tris.size(); t++) {
        vec2d points[3];
        Render3Dto2D(m.tris[t], camera, angle, points);
        for (int p = 0; p < 3; p++) {
            m.tris[t].p[p] = RotateY(vec3d(), (float)delta / 1000000.f, m.tris[t].p[p]);
        }
        bool render = true;
        for (vec2d point : points) {
            if (point.x < 0 - Offset || point.y < 0 - Offset || point.x >= Width + Offset|| point.y >= Height + Offset) {
                render = false;
                break;
            }
        }

        if (render)
            DrawFillTriangle(points, Light(vec3d(0, 0, 4), 4.5f, m.tris[t]), renderer);
    }

    SDL_RenderPresent(renderer);
    return true;
}

int main() {
    SDL_Window* window = nullptr;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        std::cout << "SDL could not be initialized: " << SDL_GetError();
    else
        std::cout << "SDL video system is ready to go\n";

    window = SDL_CreateWindow("3D Engine", 20, 20, Width, Height, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = nullptr;
    renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    // Mesh
    mesh meshCube;

    bool gameIsRunning = true;

    int prime = clock();
    int offset = 0; 

    vec3d camera(0, 0, 5);
    vec3d camera_angle;

    OnUserCreate(meshCube);

    while(gameIsRunning){
        int delta = deltaTime(prime, offset);
        prime = clock();
        offset = delta % 1000;
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            vec3d last_camera = camera;
            switch(event.type) {
                case SDL_QUIT:
                    gameIsRunning = false;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_RIGHT:
                            camera_angle.y += delta / 500000.f;
                            break;
                        case SDLK_LEFT:
                            camera_angle.y -= delta / 500000.f;
                            break;
                        case SDLK_UP:
                            camera_angle.x += delta / 500000.f;
                            break;
                        case SDLK_DOWN:
                            camera_angle.x -= delta / 500000.f;
                            break;
                        case SDLK_w:
                            camera = RotateY(camera, -camera_angle.y, camera);
                            camera.z -= delta / 100000.f;
                            camera = RotateY(last_camera, camera_angle.y, camera);
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }

        OnUserUpdate(window, renderer, meshCube, delta, camera, camera_angle);
    }
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

void Render3Dto2D(triangle tri, vec3d camera, vec3d angle, vec2d *ret) {
    int i = 0;
    bool norm = true;
    for (vec3d p : tri.p) {
        p.x -= camera.x;
        p.y -= camera.y;
        p.z -= camera.z - 4.f;
        p = RotateY(vec3d(0, 0, 4), -angle.y, p);
        if (p.z > 4) {
            norm = false;
            break;
        }
        ret[i] = vec2d((float)Width * (p.x - p.z + 4) / (8 - 2*p.z), (float)Height * (4 - p.z - p.y) / (8 - 2*p.z));
        i++;
    }
    if (!norm) {
        for (i = 0; i < 3; i++)
            ret[i] = vec2d(-1, -1);
    }
}

void DrawTriangle(vec2d tri[3], SDL_Renderer *renderer) {
    for (int i = 0; i < 3; i++) {
        int j = i + 1 >= 3 ? 0 : i + 1;
        SDL_RenderDrawLine(renderer, tri[i].x, tri[i].y, tri[j].x, tri[j].y);
    }
}

void DrawFillTriangle(vec2d tri[3], vec3d color, SDL_Renderer *renderer) {
    vector< SDL_Vertex > verts;

    for (int i = 0; i < 3; i++) {
        int el = i > 0 ? i - 1 : 2;
        vector<vec2d> ret = CollideRectOutlineLine(tri[i], tri[el], vec2d(0, 0), vec2d(Width, Height));
        for (vec2d r : ret) 
            verts.push_back({ SDL_FPoint{ r.x, r.y }, SDL_Color{ (uint8_t)color.x, (uint8_t)color.y, (uint8_t)color.z, 255 }, SDL_FPoint{ 0 }, });
    }

    for (int i = 0; i < 3; i++) {
        if (CollideRectPoint(tri[i], vec2d(0, 0), vec2d(Width, Height)))
            verts.push_back({ SDL_FPoint{ tri[i].x, tri[i].y }, SDL_Color{ (uint8_t)color.x, (uint8_t)color.y, (uint8_t)color.z, 255 }, SDL_FPoint{ 0 }, });
    }
    if (verts.size() > 2) {     
        for (int i = 0; i < verts.size() - 2; i++) {
            for (int j = i + 1; j < verts.size() - 1; j++) {
                int indexes[3] = {i, j, (int)verts.size() - 1};
                SDL_RenderGeometry(renderer, nullptr, verts.data(), verts.size(), indexes, 3);
            }
        }
    }
}

vec3d Light(vec3d light, float strength, triangle tri) {
    float X = (tri.p[0].x + tri.p[1].x + tri.p[2].x) / 3.f;
    float Y = (tri.p[0].y + tri.p[1].y + tri.p[2].y) / 3.f;
    float Z = (tri.p[0].z + tri.p[1].z + tri.p[2].z) / 3.f;
    X -= light.x;
    Y -= light.y;
    Z -= light.z;
    float dist = sqrt(X*X + Y*Y + Z*Z);
    dist /= strength;
    if (dist > 1)
        dist = 1;
    return vec3d(tri.color.x - (float)dist * tri.color.x, tri.color.y - (float)dist * tri.color.y, tri.color.z - (float)dist * tri.color.z);
}

vec3d RotateY(vec3d c, float angle, vec3d p) {
    float sn = sin(angle);
    float cs = cos(angle);

    p.x -= c.x;
    p.z -= c.z;

    float xnew = p.x * cs - p.z * sn;
    float znew = p.x * sn + p.z * cs;

    p.x = xnew + c.x;
    p.z = znew + c.z;
    return p;
}

bool CollideRectPoint(vec2d point, vec2d r_start, vec2d r_size) {
    if (point.x >= r_start.x && point.x <= r_start.x + r_size.x - 1 &&
        point.y >= r_start.y && point.y <= r_start.y + r_size.y - 1)
        return true;
    return false;
}

bool PointInLine(vec2d point, vec2d l_start, vec2d l_end) {
    return point.x <= max(l_start.x, l_end.x) && point.x >= min(l_start.x, l_end.x) && 
           point.y <= max(l_start.y, l_end.y) && point.y >= min(l_start.y, l_end.y);
}

vector<vec2d> CollideRectOutlineLine(vec2d l_start, vec2d l_end, vec2d r_start, vec2d r_size) {
    vec2d dot1, dot2, dot3, dot4;
    vector<vec2d> ret;

    if (l_start.x == l_end.x) {
        if (PointInLine(vec2d(l_start.x, r_start.y), l_start, l_end))
            ret.push_back(vec2d(l_start.x, r_start.y));
        if (PointInLine(vec2d(l_start.x, r_start.y + r_size.y - 1), l_start, l_end))
            ret.push_back(vec2d(l_start.x, r_start.y + r_size.y - 1));
        return ret;
    }

    float k, b;
    k = (l_start.y - l_end.y)/(l_start.x - l_end.x);
    b = l_start.y - k * l_start.x;

    dot1.y = r_start.x * k + b;
    dot1.x = r_start.x;

    if (CollideRectPoint(dot1, r_start, r_size) && PointInLine(dot1, l_start, l_end))
        ret.push_back(dot1);

    dot2.y = (r_start.x + r_size.x - 1) * k + b;
    dot2.x = r_start.x + r_size.x - 1;

    if (CollideRectPoint(dot2, r_start, r_size) && PointInLine(dot2, l_start, l_end))
        ret.push_back(dot2);

    if (k != 0) {
        dot3.x = (r_start.y - b) / k;
        dot3.y = r_start.y;

        if (CollideRectPoint(dot3, r_start, r_size) && PointInLine(dot3, l_start, l_end))
            ret.push_back(dot3);

        dot4.x = (r_start.y + r_size.y - 1 - b) / k;
        dot4.y = r_start.y + r_size.y - 1;

        if (CollideRectPoint(dot4, r_start, r_size) && PointInLine(dot4, l_start, l_end))
            ret.push_back(dot4);
    }

    return ret;
}