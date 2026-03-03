#include <GLFW/glfw3.h>
#include <cmath>
#include <random>
#include <iostream>
#include <vector>

using namespace std;

// -------------------- ISSUES --------------------
// • if the window is the same size of the upper frame it's ok, but when you make the window bigger than the frame 
// it duplicate the upgrade on the other edge
//
// • renderGraph not implemented yet



// callback to close the window: (TAB, ENTER)
void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_ENTER && action == GLFW_PRESS)|| (key == GLFW_KEY_TAB && action == GLFW_PRESS))
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

// simulation's objects
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

struct frame {
    // everything in pixels
    int x_left_angle;
    int y_left_angle;
    int width;
    int height;
};

class body {
private:
    vec2 position;
    vec2 velocity;
    float r; // raggio delle particelle

public:
    body(vec2 pos, vec2 vel)
    {
        position = pos;
        velocity = vel;
    }
    body(float x, float y, float vx, float vy)
    {
        position.x = x;
        position.y = y;
        velocity.x = vx;
        velocity.y = vy;
    }
    void setPos(vec2 pos) { position = pos; }
    void setPos_x(float x) { position.x = x; }
    void setPos_y(float y) { position.y = y; }

    void setVel(vec2 vel) { velocity = vel; }
    void setVel_x(float vx) { velocity.x = vx; }
    void setVel_y(float vy) { velocity.y = vy; }

    void setR(float R) { r = R; }

    vec2 getPos() const { return position; }
    float getPos_x() const { return position.x; }
    float getPos_y() const { return position.y; }

    vec2 getVel() const { return velocity; }
    float getVel_x() const { return velocity.x; }
    float getVel_y() const { return velocity.y; }

    float getR() const { return r; }
};

void chosenAlgorithm (body& b, float dt, vec2 acceleration)
{
    // Euler algorithm
    b.setVel(b.getVel() + acceleration * dt);
    b.setPos(b.getPos() + b.getVel() * dt);
}

void verifyWallCollision(body &b, const vec2& lim_x, const vec2& lim_y)
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

int verifyBodiesCollision (body& b, vector<body>& bodies, int i)
{
    int collisions = 0;
    for (int j = 0; j < i; j++) // verify bodies collisions
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
            collisions++;
        }
    }
    return collisions;
}

void drawDisk(float x, float y, float r)
{
    int segments = 30;
    float angle;
    float ax, ay;

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex2f(x, y); // draw the disk's center

    for (int i = 0; i <= segments; i++)
    {
        angle = 2.0f * M_PI * i / segments;
        ax = x + cos(angle) * r;
        ay = y + sin(angle) * r;
        glVertex2f(ax, ay);
    }
    glEnd();
}

void updateSimulation (vector<body>& bodies, float dt, vec2 acceleration, const vec2& lim_x, const vec2& lim_y,
                       int& collisions, bool status_collision = false)
{
    int i = 0;
    if (status_collision == true) {
        for (auto &b : bodies)
        {
            chosenAlgorithm(b, dt, acceleration);
            verifyWallCollision(b, lim_x, lim_y);
            collisions += verifyBodiesCollision(b, bodies, i);
            i++;
        }        
    }
    else {
        for (auto &b : bodies)
        {
            chosenAlgorithm(b, dt, acceleration);
            verifyWallCollision(b, lim_x, lim_y);
            i++;
        }  
    }

}

void renderSimulation (GLFWwindow* window, const vector<body>& bodies, frame upperFrame)
{
    // display on the upper frame of the window
    glViewport(upperFrame.x_left_angle, upperFrame.y_left_angle, upperFrame.width, upperFrame.height);
    
    for(auto& b : bodies)
    {
        drawDisk(b.getPos_x(), b.getPos_y(), b.getR());
    }
}

void computeEnergy (const vector<body>& bodies, vector<float>& energyHistory)
{
    // pass each body OR the entire vector and compute
    float E = 0;
    for (auto &b : bodies)
    {
        E += b.getVel_x()*b.getVel_x() + b.getVel_y()*b.getVel_y();
    }
    energyHistory.push_back(E);
}

void renderGraph ()
{
    // display on the lower frame of the window
}

string collisionStatus (bool status)
{
    if (status == true)
        return "ON";

    return "OFF";
}

int main()
{
    // ------------ PHYSICAL BOX DIMENSIONS (simulation space) ------------
    // These define the REAL size of the simulation box (independent of window size)
    float boxWidth  = 250.0f;   // physical width
    float boxHeight = 150.0f;    // physical height

    // ------------ FRAME PIXEL DIMENSIONS ------------
    int Upper_pixel_width  = boxWidth*10;
    int Upper_pixel_height = boxHeight*10;

    int Lower_pixel_height = 0;
    int Lower_pixel_width  = Upper_pixel_width;   // stesso width del frame superiore

    // finestra = somma delle altezze, larghezza comune
    int window_width  = Upper_pixel_width/2 ;
    int window_height = Upper_pixel_height/2 + Lower_pixel_height;

    frame upperFrame = {0,
                        Lower_pixel_height,
                        Upper_pixel_width,
                        Upper_pixel_height};

    frame lowerFrame = {0,
                        0,
                        Lower_pixel_width,
                        Lower_pixel_height};

    vec2 lim_x = {0.0f, boxWidth};
    vec2 lim_y = {0.0f, boxHeight};

    // ------------ simulation variables ------------
    double lastTime; // = glfwGetTime() prende il tempo SOLO DOPO aver inizializzato GLFW con glfwInit()
    double currentTime;
    double dt;
    
    int N = 2000;                             // number of bodies
    float dimension = 0.5f;
    float v_max = 100.0f;

    float g = 0;                            // constant y-acceleration
    vec2 acceleration = {0, g};

    bool status_collision = true;
    int collisions = 0;

    vector<float> energyHistory;
    vector<float> timeHistory;

    /*
    // ------------ user interface ------------ TO BE REVIEWED
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
    */
    vector<body> bodies;
    bodies.reserve(N);

    // ------------ generating random position and velocity ------------
    random_device rd;
    mt19937 gen(rd());
    float r = dimension;   // now radius is in PHYSICAL units
    uniform_real_distribution<float> distPosX(0.0f + r, boxWidth - r);
    uniform_real_distribution<float> distPosY(0.0f + r, boxHeight - r);
    uniform_real_distribution<float> distVel(-v_max, v_max);

    // ------------ simulation'stats ------------
    cout << "\n------------ SIMULATION'S STATS ------------" << endl;
    cout << "| " << "Number of bodies \t\t--> " << N << endl;
    cout << "| " << "Dimension of each body \t--> " << dimension << endl;
    cout << "| " << "Range of velocities \t\t--> " << v_max << endl;
    cout << "| " << "Collisions within bodies \t--> " << collisionStatus(status_collision) << endl;

    // -----------------------------------------------------
    // ----------------- Simulation begins -----------------
    // -----------------------------------------------------

    // GLFW va inizializzato --> int glfwInit(), in caso di errore chiama glfwTerminate()
    if (!glfwInit()) {
        cerr << "Errore inizializzazione GLFW\n";
        return -1;
    }

    // crea il puntatore a una variabile di tipo GLFWwindow


    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Atom", NULL, NULL);

    if (!window) {
        cerr << "Errore creazione finestra\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);                 // crea un contesto OpenGL
    glfwSwapInterval(1);

    
    // does it has to be the frame or the window width? (for now they are the same)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);


    

    // Inizializzazione
    for (int i = 0; i < N; i++) {
        body b(distPosX(gen), distPosY(gen), distVel(gen), distVel(gen));
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
        
        updateSimulation(bodies, dt, acceleration, lim_x, lim_y, collisions, status_collision);

        computeEnergy(bodies, energyHistory);

        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(upperFrame.x_left_angle,
                   upperFrame.y_left_angle,
                   upperFrame.width,
                   upperFrame.height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // Calcola la scala in unità fisiche per pixel (uguale su entrambi gli assi -> cerchi perfetti)
        float scaleX = boxWidth  / (float)upperFrame.width;
        float scaleY = boxHeight / (float)upperFrame.height;
        float scale  = (scaleX > scaleY) ? scaleX : scaleY; // usa la scala più grande per contenere tutta la box

        float viewW = scale * upperFrame.width;   // larghezza logica mappata sul viewport
        float viewH = scale * upperFrame.height;  // altezza  logica mappata sul viewport

        // Centra la box fisica nel range logico
        float cx = boxWidth  * 0.5f;
        float cy = boxHeight * 0.5f;
        glOrtho(cx - viewW * 0.5f, cx + viewW * 0.5f,
                cy - viewH * 0.5f, cy + viewH * 0.5f,
                -1.0f, 1.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        renderSimulation(window, bodies, upperFrame);

        // render graph: display energy of the current time and A or B
        // A: update each time (or with a few skip) the scale of the graph
        // B: uptade the scale whenever the time reaches the limit
        // ...

        glfwPollEvents(); 
        glfwSwapBuffers(window);
    }

    if (status_collision == true)
        cout << "| " << "Number of collisions \t\t--> " << collisions << endl;

    cout << "| " << "Energy variation \t\t--> " << energyHistory.back() - energyHistory[0] << endl;
    cout << "--------------------------------------------" << endl;
    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}