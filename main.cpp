#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <graphics.h>

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define PI 3.14159265358979323846

/* --- MATHS --- */

struct Vertex{
    float x, y, z;

    Vertex() : x(0), y(0), z(0) {};
    Vertex(const Vertex& v) : x(v.x), y(v.y), z(v.z) {};
    Vertex(float x, float y, float z) : x(x), y(y), z(z) {};
};
std::vector<Vertex> vertices;
std::vector<Vertex> mvpVertices;
std::vector<int> indices;

void matmul(float m[][3], Vertex& v){
    Vertex vc(v);
    vc.x = (m[0][0] * v.x) + (m[0][1] * v.y) + (m[0][2] * v.z);
    vc.y = (m[1][0] * v.x) + (m[1][1] * v.y) + (m[1][2] * v.z);
    vc.z = (m[2][0] * v.x) + (m[2][1] * v.y) + (m[2][2] * v.z);
    v = vc;
}
void matmul(float a[][3], float b[][3], float c[][3]){
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          float sum = 0;
          for (int k = 0; k < 3; k++) {
            sum += a[i][k] * b[k][j];
          }
          c[i][j] = sum;
        }
    }
}
void scale(Vertex& v, float scalar){
    v.x *= scalar;
    v.y *= scalar;
    v.z *= scalar;
}
void translate(Vertex& v, float x, float y, float z){
    v.x += x;
    v.y += y;
    v.z += z;
}
void clamp(float& x, float min, float max){
    if(x > max) x = max;
    else if(x < min) x = min;
}

/* --- IO obj loading --- */

char filepath[100];
void filedialog(){
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof ( ofn );
	ofn.lpstrFile = filepath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filepath);
	ofn.lpstrFilter = "obj\0*.OBJ\0";
	ofn.nFilterIndex =1;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
    GetOpenFileName(&ofn);
}

bool load(const char* filename){
    std::ifstream file(filename);
    if(file.is_open() == false) return false;

    vertices.clear();
    indices.clear();

    while(!file.eof()){
        char c;
        file >> c;
        if(c == 'v'){
            float x, y, z;
            file >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }else if(c == 'f'){
            int x, y, z;
            file >> x >> y >> z;
            indices.push_back(x);
            indices.push_back(y);
            indices.push_back(z);
        }
    }

    file.close();
    return true;
}

/* --- USER INTERFACE --- */

bool mouseDown = false;
bool mouseClicked = false;
int pmx = 0, pmy = 0;
bool button(int x, int y, int w, int h, const char* text){
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(x, y, x+w, y+h);
    setcolor(LIGHTGRAY);
    rectangle(x, y, x+w, y+h);

    setbkcolor(DARKGRAY);
    setcolor(WHITE);
    settextjustify(CENTER_TEXT, CENTER_TEXT);
    outtextxy(x + w/2, y + h/2 + 5, (char*)text);

    setbkcolor(BLACK);
    setcolor(WHITE);

    return (mouseClicked && mousex() > x && mousey() > y && mousex() < x+w && mousey() < y+h);
}

/* --- MAIN --- */

float angleX = 0, angleY = 0;
float zoom = 5.0f;
bool ortho = false;
int main(){
    initwindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Atestat (3d model renderer)", (getmaxwidth() - WINDOW_WIDTH) / 2, (getmaxheight() - WINDOW_HEIGHT) / 2, false, true);

    registermousehandler(WM_LBUTTONDOWN, [](int x, int y){
        mouseDown = true;
        mouseClicked = true;
    });
    registermousehandler(WM_LBUTTONUP, [](int x, int y){
        mouseDown = false;
    });

    while(true){
        cleardevice();

        /* --- CAMERA CONTROLL --- */

        if(kbhit()){
            char k = getch();
            if(k == '+'){
                zoom += zoom * 0.1f;
            }else if(k == '-'){
                zoom -= zoom * 0.1f;
            }
            if(k == 'p'){
                ortho = !ortho;
            }
            if(k == 'o'){
                filedialog();
                load(filepath);
            }
            clamp(zoom, 0.5f, 10.0f);
        }

        if(mouseDown){
            angleY += (mousex() - pmx) * 0.01f;
            angleX += (mousey() - pmy) * 0.01f;
        }
        clamp(angleX, -PI*0.5f, PI*0.5f);
        pmx = mousex();
        pmy = mousey();
        mouseClicked = false;


        /* --- MODEL DRAWING --- */

        if(vertices.empty() == false){
            mvpVertices.resize(vertices.size());
            settextjustify(LEFT_TEXT, CENTER_TEXT);
            outtextxy(10, WINDOW_HEIGHT - 15, filepath);

            for(unsigned int i = 0; i < vertices.size(); i++){
                Vertex p(vertices[i]);

                float rotX[3][3] = {
                    { 1, 0, 0},
                    { 0, cosf(angleX), -sinf(angleX)},
                    { 0, sinf(angleX), cosf(angleX)}
                };
                float rotY[3][3] = {
                    { cosf(angleY), 0, sinf(angleY)},
                    { 0, 1, 0},
                    { -sinf(angleY), 0, cosf(angleY)}
                };
                float r[3][3] = {0};

                matmul(rotX, rotY, r);
                matmul(r, p);

                float d = (ortho) ? 1 : (1.0f / (3.0f - p.z));
                float projection[3][3] = {
                    {d, 0, 0},
                    {0, d, 0},
                    {0, 0, 0}
                };
                matmul(projection, p);
                scale(p, 100);
                scale(p, zoom);
                translate(p, 250, 250, 0);
                mvpVertices[i] = p;
            }

            for(unsigned int i = 0; i < indices.size()-2; i += 3){
                int a = indices[i]-1, b = indices[i+1]-1, c = indices[i+2]-1;
                line(mvpVertices[a].x, WINDOW_HEIGHT-mvpVertices[a].y, mvpVertices[b].x, WINDOW_HEIGHT-mvpVertices[b].y);
                line(mvpVertices[b].x, WINDOW_HEIGHT-mvpVertices[b].y, mvpVertices[c].x, WINDOW_HEIGHT-mvpVertices[c].y);
                line(mvpVertices[a].x, WINDOW_HEIGHT-mvpVertices[a].y, mvpVertices[c].x, WINDOW_HEIGHT-mvpVertices[c].y);
            }
        }


        /* --- MENU --- */

        if(button(10, 10, 70, 30, "Open")){
            filedialog();
            load(filepath);
        }
        if(button(90, 10, 70, 30, "Clear")){
            vertices.clear();
            indices.clear();
        }
        if(button(250, 10, 160, 30, "Change Projection")){
            ortho = !ortho;
        }
        if(button(420, 10, 30, 30, "+")){
            zoom += zoom * 0.1f;
        }
        if(button(460, 10, 30, 30, "-")){
            zoom -= zoom * 0.1f;
        }

        swapbuffers();
    }

    closegraph();
    return 0;
}
