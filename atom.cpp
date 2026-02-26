#include <GLFW/glfw3.h>
#include <cmath>
#include <random>
#include <iostream>
#include <vector>

using namespace std;

// callback per chiudere la finestra: (TAB, ENTER)
void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_ENTER && action == GLFW_PRESS)|| (key == GLFW_KEY_TAB && action == GLFW_PRESS))
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

// strutture della simulazione
struct vec2 {
    float x;
    float y;

    vec2 operator*(float dt) const {
        return {x * dt, y * dt};
    }

    vec2 operator+(const vec2& v) const {
        return {x + v.x, y + v.y};
    }

    vec2 operator-(const vec2& v) const {
        return {x - v.x, y - v.y};
    }

    vec2& operator+=(const vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    vec2& operator-=(const vec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    float dot(const vec2& v) const {
        return x * v.x + y * v.y;
    }

    float norm () const {
        return sqrt(x * x + y * y);
    }
};


class body {
    private:
        vec2 position;
        vec2 velocity;
        float r; // raggio delle particelle

    public:
        body (vec2 pos, vec2 vel) {
            position = pos;
            velocity = vel;
        }
        body (float x, float y, float vx, float vy) {
            position.x = x;
            position.y = y;
            velocity.x = vx;
            velocity.y = vy;
        }
        void setPos(vec2 pos) {position = pos;}
        void setPos_x(float x) { position.x = x; }
        void setPos_y(float y) { position.y = y; }

        void setVel_x(float vx) { velocity.x = vx; }
        void setVel_y(float vy) { velocity.y = vy; }

        void setR(float R) { r = R; }

        vec2 getPos() const { return position; }
        float getPos_x() const { return position.x; }
        float getPos_y() const { return position.y; }

        vec2 getVel () const { return velocity; }
        float getVel_x() const { return velocity.x; }
        float getVel_y() const { return velocity.y; }

        float getR() const { return r; }
};

const vec2 lim_x = {-1.0f, 1.0f};
const vec2 lim_y = {-1.0f, 1.0f};

void verifyCollision(body &b)
{
    float r = b.getR();
    // collisione con bordi
    if (b.getPos().x < lim_x.x + r) {
        b.setPos_x(lim_x.x + r);
        b.setVel_x(b.getVel_x()*(-1.0f));
    }
    if (b.getPos().x > lim_x.y - r) {
        b.setPos_x(lim_x.y - r );
        b.setVel_x(b.getVel_x()*(-1.0f));
    }
        if (b.getPos().y < lim_y.x + r) {
        b.setPos_y(lim_y.x + r);
        b.setVel_y(b.getVel_y()*(-1.0f));
    }
        if (b.getPos().y > lim_y.y - r ) {
        b.setPos_y(lim_y.y - r);
        b.setVel_y(b.getVel_y()*(-1.0f));
    }

}

int main()
{
    int pixel_width = 800;
    int pixel_height = 600;

    double lastTime; // = glfwGetTime() prende il tempo SOLO DOPO aver inizializzato GLFW con glfwInit()
    double currentTime;
    double dt;
    
    int N = 10;                             // numero di corpi
    float dimension = 1.0f;
    float v_max;

    float g = -0.981;

    cout << "Parametri della simulazione:\n";
    cout << "inserisci numero di corpi --> ";
    cin >> N;
    cout << "\ndimensioni di ogni corpo --> ";
    cin >> dimension;
    cout << "\nvelocità massima -->";
    cin >> v_max;
    cout << "\nENTER o TAB per terminare";

    if (v_max < 0)
        v_max *= -1.0f;
    vector<body> bodies;
    bodies.reserve(N);                      // alloca memoria e impedisce che quegli indirizzi vengano riallocati
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distPos(-0.9f, 0.9f);
    uniform_real_distribution<float> distVel(-v_max, v_max);



    
    // GLFW va inizializzato --> int glfwInit(), in caso di errore chiama glfwTerminate()
    if (!glfwInit()) {
        cerr << "Errore inizializzazione GLFW\n";
        return -1;
    }

    // crea il puntatore a una variabile di tipo GLFWwindow
    GLFWwindow* window = glfwCreateWindow(pixel_width, pixel_height, "Atom", NULL, NULL);

    if (!window) {
        cerr << "Errore creazione finestra\n";
        glfwTerminate();
        return -1;
    }

    
    glfwMakeContextCurrent(window);                                 // crea un contesto OpenGL
    // glfwSwapInterval(1);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    float r = (dimension) / width;   // raggio in coordinate OpenGL
    
    // Inizializzazione
    for (int i = 0; i < N; i++) {
        body b(distPos(gen), distPos(gen), distVel(gen), distVel(gen));
        b.setR(r);
        bodies.push_back(b);
    }

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // imposta il colore di sfondo

    glfwSetKeyCallback(window, key_callback);                       // setting della key_callback

    lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        currentTime = glfwGetTime();
        dt = currentTime - lastTime;
        lastTime = currentTime;
        
        int i = 0;

        glClear(GL_COLOR_BUFFER_BIT);

        glPointSize(2*dimension);
        glBegin(GL_POINTS);
        glColor3f(1.0f, 1.0f, 0.0f);

        /*
        for (const auto &b : bodies)
        {
            
        }        
        */
        for (auto &b : bodies)
        {
            b.setVel_y(b.getVel_y() + g*dt);
            b.setPos(b.getPos() + b.getVel() * dt);

            verifyCollision(b);

            
            for (int j = 0; j < i; j++)
            {
                vec2 delta = b.getPos() - bodies[j].getPos();
                float dist = delta.norm();
                float minDist = b.getR() + bodies[j].getR();

                if (dist <= minDist && dist > 0.0f)
                {
                    vec2 n = { delta.x / dist, delta.y / dist };

                    vec2 v1 = b.getVel();
                    vec2 v2 = bodies[j].getVel();

                    float v1n = v1.dot(n);
                    float v2n = v2.dot(n);

                    float p = v1n - v2n;

                    b.setVel_x(v1.x - p * n.x);
                    b.setVel_y(v1.y - p * n.y);

                    bodies[j].setVel_x(v2.x + p * n.x);
                    bodies[j].setVel_y(v2.y + p * n.y);

                    
                    float overlap = minDist - dist;
                    vec2 correction = { n.x * (overlap * 0.5f), n.y * (overlap * 0.5f) };

                    b.setPos( b.getPos() + correction );
                    bodies[j].setPos( bodies[j].getPos() - correction );
                    
                }
            }
            
            
            glVertex2f( b.getPos_x(), b.getPos_y());
            i++;
            

        }





        glEnd();

        glfwPollEvents(); // intercetta tutti gli eventi
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    // chiusura di GLFW
    glfwTerminate();
    return 0;
}