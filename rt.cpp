//////////////////////////////////////////////////////
//
// A simple ray tracer.
//
//////////////////////////////////////////////////////

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "Camera.h"
#include "KBUI.h"

#include "GeomLib.h"
#include "Color.h"
#include "Material.h"
#include "Object.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Light.h"
#include "Hit.h"

using namespace std;

KBUI the_ui;
Camera cam;

void camera_changed(float);
void reset_camera(float);
void reRender();

ofstream dbgfile("debug.dat");
bool debugOn = false;
typedef unsigned char byte;

// The initial image is 100 x 100 x 3 bytes (3 bytes per pixel)
int winWidth  = 500;
int winHeight = 500;



byte *img = NULL;

Point4  eye(0, 0, 0);
Point4  lookat(0, 0, 0);
Vector4 vup(0, 0, 0);

// The clipping frustum
float clipL = 0;
float clipR = 0;
float clipB = 0;
float clipT = 0;
float clipN = 0;


vector<Object*> sceneObjects; // list of object in the scene

vector<Light> sceneLights; // list of lights in the scene

vector<Material> materials; // list of available materials

vector<Hit> hits; // list of available hits
int hitSize = 10;

Matrix4 Mvcswcs;  // the inverse of the view matrix.
Hit *hitPool;     // array of available hit records.

Color ambientLight(0,0,0); // indirect light that shines when all lights are blocked

bool frame_buffer_stale = true;
int mouse_x, mouse_y;

Ray4 Mainray;
bool shadowOn = false;

// Forward declarations for functions in this file
void init_UI();
void setupCamera();
void setRay(int xDCS, int yDCS, Ray4& ray);
Vector4 mirrorDirection(Vector4& L, Vector4& N);
Color localIllum(Vector4& V, Vector4& N, Vector4& L,
                 Material& mat, Color& ls);
float power(float x, int n);
Hit firstHit(Ray4 &ray);
Hit shadowray_First_Hit(Ray4 &ray);
void camera_changed(float dummy);
void reRender();
Color glossy_color(Ray4 &ray, Hit &hit);
Color rayColor(int xDCS, int yDCS);
void render();
string downcase(const string &s);
void match(ifstream &file, const string& pattern);
void readScene(char *sceneFile);
void reset_camera(float dummy);
void mouse_button_callback( GLFWwindow* window, int button,
                            int action, int mods );
void mouse_position_callback( GLFWwindow* window, double x, double y );
static void error_callback(int error, const char* description);
void display();
static void key_callback(GLFWwindow* window, int key,
                         int scancode, int action, int mods);
int main(int argc, char *argv[]);


///////////////////////////////////////////////////////////////////////
// Initialize the keyboard-driven UI.
// (no need to change this)
///////////////////////////////////////////////////////////////////////
void init_UI() {
    // These variables will trigger a call-back when they are changed.
    the_ui.add_variable("Eye X", &eye.X(), -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Eye Y", &eye.Y(), -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Eye Z", &eye.Z(), -10, 10, 0.2, camera_changed);

    the_ui.add_variable("Ref X", &lookat.X(), -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Ref Y", &lookat.Y(), -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Ref Z", &lookat.Z(), -10, 10, 0.2, camera_changed);

    the_ui.add_variable("Vup X", &vup.X(), -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Vup Y", &vup.Y(), -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Vup Z", &vup.Z(), -10, 10, 0.2, camera_changed);

    the_ui.add_variable("Clip L", &clipL,   -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Clip R", &clipR,   -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Clip B", &clipB,   -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Clip T", &clipT,   -10, 10, 0.2, camera_changed);
    the_ui.add_variable("Clip N", &clipN,   -10, 10, 0.2, camera_changed);

    float dummy2;
    the_ui.add_variable("Reset Camera", &dummy2,0,100, 0.001, reset_camera);

    the_ui.done_init();

}

/////////////////////////////////////////////////////////////////////////
// Initialize the VCS-to-WCS matrix
/////////////////////////////////////////////////////////////////////////
void setupCamera() {
    
    // The camera's basis vectors
    Vector4 cam_Z = (eye - lookat).normalized();
    Vector4 cam_X = (vup ^ cam_Z).normalized();
    Vector4 cam_Y = cam_Z ^ cam_X;

    // The camera-to-world matrix
    Mvcswcs.set(cam_X.X(), cam_Y.X(), cam_Z.X(), eye.X(),
                    cam_X.Y(), cam_Y.Y(), cam_Z.Y(), eye.Y(),
                    cam_X.Z(), cam_Y.Z(), cam_Z.Z(), eye.Z(),
                      0,         0,         0,         1);

    // reserve vector size
    hits.reserve(hitSize);

}

/////////////////////////////////////////////////////////////////////////
// Create a ray which starts at the given (x y)DCS pixel
/////////////////////////////////////////////////////////////////////////

void setRay(int xDCS, int yDCS, Ray4& ray) {

    
    Point4 P_dcs(xDCS, yDCS, 0);

    // The pixel in VCS
    float dx = (clipR - clipL) / winWidth;
    float dy = (clipT - clipB) / winHeight;
    float x_vcs = clipL + (P_dcs.X() + 0.5) * dx;
    float y_vcs = clipB + (P_dcs.Y() + 0.5) * dy;
    float z_vcs = -clipN;

    /////////////////////////////////////////////////////////////////////////
    // The abouve calculation produces numbers like 4.85e its basically zero,
    // But the below calculation produces zeroes properly
    /////////////////////////////////////////////////////////////////////////

    // float x_vcs = clipL +(clipR - clipL) * ( P_dcs.X() + 0.5) / winWidth;
    // float y_vcs = clipB +(clipT - clipB) * ( P_dcs.Y() + 0.5) / winHeight;
    // float z_vcs = -clipN;

    Point4 P_vcs(x_vcs, y_vcs, z_vcs);


    Float4 F_wcs = Mvcswcs * P_vcs;
    Point4 P_wcs(F_wcs.X(), F_wcs.Y(), F_wcs.Z());



    Point4 S = P_wcs;           // ray start

    Vector4 V = (P_wcs - eye).normalized(); // ray direction 


    Mainray.start = S;
    Mainray.direction = V;


}

/////////////////////////////////////////////////////////////////////////
// Given a light direction and surface normal, get the
// mirror direction (No need to change this)
/////////////////////////////////////////////////////////////////////////
Vector4 mirrorDirection(Vector4& L, Vector4& N) {
    float NL = N * L;
    return (2.0f * NL) * N - L;
}


// Helper function for computing power
// of a float and positive n
float power(float x, int n)
{
    float pow = 1;
    float squares = x;

    while(n>0)
    {
        if(n%2 == 1)
        {
            pow *= squares;
        }
        squares *= squares;
        n /= 2;
    }

    return pow;
}



/////////////////////////////////////////////////////////////////////////
// Compute the Phong local illumination color.
/////////////////////////////////////////////////////////////////////////
Color computeIntensity(Ray4 &ray, Hit &hit)
{
    // Intenstity components
    Color Intensity(0,0,0);
    Color ambient_term;
    Color diffuse_term;
    Color specular_term;
    Vector4 L; // position from light to hit point
    Vector4 N; // surface normal at hit point
    Vector4 R; // mirror reflection
    Vector4 V; // ray direction
    Color I_L; // Light color;

    Color _ambientLight;

    Color ambient;
    Color diffuse;
    Color specular;

    int n = 0;

    Color I;

    // get the material properties of the hit object
    ambient = hit.material.getAmbient();
    diffuse = hit.material.getDiffuse();
    specular= hit.material.getSpecular();
    n = hit.material.getShininess();

    Color IaKa;


    // Lights ambient color
    _ambientLight = ambientLight;

    for (auto h : sceneLights)
    {
        
        L = (h.getLightPos() - hit.hit_point).normalized();  // Vector from hit to light

        N = hit.normal;
        R = mirrorDirection(L, N).normalized(); // Calculate mirror reflection from light on object
        V = (-ray.direction).normalized(); // ray direction
        I_L = h.getLightColor();

        ambient_term = ambient ^ I_L;
        float RV = (R * V );
        IaKa = ambientLight ^ ambient_term;

        Ray4 shadowray(hit.hit_point, L);           // shadow ray sent from hit point to Light direction
        Hit shadowhit = shadowray_First_Hit(shadowray); // compute shadow hit if there is any light cant illuminate in that portion

        if((shadowOn))
        {
            if ((N * L) > 0)
            {
                diffuse_term = (diffuse ^ I_L) * (N * L);
                if(RV < 0)
                {
                    RV = 0;
                }
                specular_term = (specular ^ I_L) * power (RV, n);
                Intensity +=  IaKa +  diffuse_term + specular_term;

            }

        }
        else
        {
            Intensity += IaKa;
        }

    }

    return Intensity;
}

/////////////////////////////////////////////////////////////////////////
// Find the first object hit by the shadow ray, if any
/////////////////////////////////////////////////////////////////////////
Hit shadowray_First_Hit(Ray4 &ray){

    Hit h;
    Hit Besthit;

    float tmin = 1000;
    for(Object* obj : sceneObjects )
    {
        if(obj -> intersects(ray, h) )
        {
            if( h.t < tmin )
            {
                Besthit = h;
                tmin = h.t;
                shadowOn = true;
            }
        }
    }


    return Besthit;

}


/////////////////////////////////////////////////////////////////////////
// Find the first object hit by the ray, if any
/////////////////////////////////////////////////////////////////////////

Hit firstHit(Ray4 &ray) {

    Hit h;
    Hit Besthit;

    float tmin = 1000;
    for(Object* obj : sceneObjects )
    {
        if(obj -> intersects(ray, h) )
        {
            if( h.t < tmin )
            {
                Besthit = h;
                tmin = h.t;
            }
        }
    }


    return Besthit;
}



/////////////////////////////////////////////////////////////////////////
// Triggered by KBUI when user changes a camera parameter
// (No need to change this)
/////////////////////////////////////////////////////////////////////////
void camera_changed(float dummy) {
    reRender();
}

/////////////////////////////////////////////////////////////////////////
// Called when picture needs to be rendered again.
// (No need to change this)
/////////////////////////////////////////////////////////////////////////
void reRender() {
    frame_buffer_stale = true;
}


/////////////////////////////////////////////////////////////////////////
// Create a ray starting at (x y)DCS,
// test it against all objects in the scene,
// get the first object hit,
// and compute the intensity at the hit point,
/////////////////////////////////////////////////////////////////////////
Color rayColor(int xDCS, int yDCS) {

    // back ground is black
    Color background(1, 1, 1);

    Ray4 ray;
    setRay(xDCS, yDCS, ray);
    Hit hit = firstHit(Mainray);

    if(hit.t > 0)
    {
        return computeIntensity(Mainray, hit);
    }

    else
    {
        return background;
    }

}

/////////////////////////////////////////////////////////////////////////
//
// This function actually generates the ray-traced image.
// This is where main code will go.
/////////////////////////////////////////////////////////////////////////

void render() {
    int x,y;
    byte r,g,b;
    int p = 0;

    // actual ray tracer rendering code

    Color c;

    for (y=0; y<winHeight; y++)
    {
        for (x=0; x<winWidth; x++)
        {
            
            p = (y*winWidth + x) * 3;

            c= rayColor(x, y);

            for(int i = 0; i< 3; i++)
            {
                if(c[i] > 1.0f)
                {
                    c[i] = 1.0f;
                }
            }

            r = c[0] * 255;
            g = c[1] * 255;
            b = c[2] * 255;

            img[p++] = r;
            img[p++] = g;
            img[p] =   b;

        }
    }

}

/////////////////////////////////////////////////////////////////////////
// Turn string to lowercase.
// (No need to change this)
/////////////////////////////////////////////////////////////////////////

string downcase(const string &s) {
    string result = "";
    for (int i=0; i<(int)s.length(); i++) {
        char c = s[i];
        if ('A' <= c && c <= 'Z')
            c = c - 'A' + 'a';
        result += c;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////
// Convenience function for checking that the upcoming
// word in the file matches what you are expecting.
// (No need to change this)
/////////////////////////////////////////////////////////////////////////

void match(ifstream &file, const string& pattern) {
    string word;
    file >> word;
    word = downcase(word);
    if (word != string(pattern)) {
        cerr << "Parse error on data file: expected \""
             << pattern << "\" but found \"" << word << "\"\n";
        exit(EXIT_FAILURE);
    }
}



/////////////////////////////////////////////////////////////////////////
// helper function to count no of lines in a file
/////////////////////////////////////////////////////////////////////////
int noOfLines(ifstream &file)
{
    int number_of_lines =0;
     string line;
    if(file.is_open()){
        while(!file.eof()){
            getline(file,line);
            //cout<< line << endl;
            number_of_lines++;
        }
        file.close();
    }

    return number_of_lines;
}

/////////////////////////////////////////////////////////////////////////
// THis function reads material description from input file
/////////////////////////////////////////////////////////////////////////
void readMaterials(ifstream &file)
{
    
    Color ka;
    Color kd;
    Color ks;

    int n = 0;

    string word;

    //ambient component
    file >> word;
    file >> ka.X() >> ka.Y() >> ka.Z();
  
    // Difffuse component
    file >> word;
    file >> kd.X() >> kd.Y() >> kd.Z();
    
    // Specular component
    file >> word;
    file >> ks.X() >> ks.Y() >> ks.Z();

    // Shininess
    file >> word;
    file >> n;
        

    materials.push_back(Material(ka, kd, ks, n));




    
}

/////////////////////////////////////////////////////////////////////////
// Utility function -reads Light description from input file
/////////////////////////////////////////////////////////////////////////
void readLights(ifstream &file)
{

    Point4 position;
    Color color;


    string word;

    // read color
    file >> word;
    file >> color.X() >> color.Y() >> color.Z();
    
    // read position
    file >> word;
    file >> position.X() >> position.Y() >> position.Z();

    ambientLight.X() += 0.30 * color.X();
    ambientLight.Y() += 0.30 * color.Y();
    ambientLight.Z() += 0.30 * color.Z();



    sceneLights.push_back(Light(position, color));



}


/////////////////////////////////////////////////////////////////////////
// Utility function -reads Triangle description from input file
/////////////////////////////////////////////////////////////////////////
void readTriangle(ifstream &file)
{
    Point4 v1;
    Point4 v2;
    Point4 v3;

    int material = 0;

    string word;

    // V1
    file >> word;
    file >> v1.X() >> v1.Y() >> v1.Z();

    // V2
    file >> word;
    file >> v2.X() >> v2.Y() >> v2.Z();

    // V3
    file >> word;
    file >> v3.X() >> v3.Y() >> v3.Z();

    // material
    file >> word;
    file >> material;

    Material color = materials[material];


    sceneObjects.push_back(new Triangle(v1, v2, v3, color));






}

/////////////////////////////////////////////////////////////////////////
// Utility function -reads Sphere description from input file
/////////////////////////////////////////////////////////////////////////
void readSphere(ifstream &file)
{
    Point4 center;
    float radius = 0;
    int material = 0;


    string word;

    // Center 
    file >> word;
    file >> center.X() >> center.Y() >> center.Z();

    // Radius
    file >> word;
    file >> radius;

    //material
    file >> word;
    file >> material;

    Material color = materials[material];

    sceneObjects.push_back(new Sphere(center, radius, color));


}

//////////////////////////////////////////////////////
//
// This function reads the scene from the data file,
// which is an argument given on the command line.
// (You must implement this function)
//
/////////////////////////////////////////////////////
void readScene(char *sceneFile) {
    ifstream file(sceneFile);

    if (!file) {
        cerr << "Can't read from " << sceneFile << endl;
        exit(EXIT_FAILURE);
    }



    int materials_count = 0;
    int lights_count = 0;
    int objects_count = 0;

    string materials,lights,objects;

    while(!file.eof())
    {
        
        string word;
        file >> word;

        
        if( word == "#materials")
        {
            materials = word;
            file >> word;
            materials_count = stoi(word);
            materials.reserve(materials_count);
            
        }

        else if( word == "#lights")
        {
            lights = word;
            file >> word;
            lights_count = stoi(word);
            sceneLights.reserve(lights_count);
        }

        else if( word == "#objects")
        {
            objects = word;
            file >> word;
            objects_count = stoi(word);
            sceneObjects.reserve(objects_count);

        }

        else if(word == "camera_eye")
        {
            file >> eye.X() >> eye.Y() >> eye.Z();
        }

        else if(word == "camera_lookat")
        {
            file >> lookat.X() >> lookat.Y() >> lookat.Z();
        }

        else if (word == "camera_vup")
        {
            file >> vup.X() >> vup.Y() >> vup.Z();
        }

        else if(word == "camera_clip")
        {
            file >> clipL >> clipR >> clipB >> clipT >> clipN;
        }

        else if( word == "material")
        {
            readMaterials(file);
        }

        else if( word == "light")
        {
            readLights(file);
        }

        else if( word == "triangle" || word == "sphere")
        {
            if( word == "triangle")
            {
                readTriangle(file);
            }
            else if(word == "sphere")
            {
                readSphere(file);
            }
           
        }

        
    }


}

///////////////////////////////////////////////////
//
// Resets the camera parameters.
//
// **** NO NEED TO CHANGE THIS ****
//
///////////////////////////////////////////////////

void reset_camera(float dummy) {
    eye.X() = 0;
    eye.Y() = 0;
    eye.Z() = 4;

    lookat.X() = 0;
    lookat.Y() = 0;
    lookat.Z() = 0;

    vup.X() = 0;
    vup.Y() = 5;
    vup.Z() = 0;

    clipL = -1;
    clipR = +1;
    clipB = -1;
    clipT = +1;
    clipN =  2;
}

//////////////////////////////////////////////////////
//
// Displays, on STDOUT, the color of the pixel that
//  the user clicked on.
//
// **** NO NEED TO CHANGE THIS ****
//
//////////////////////////////////////////////////////

void mouse_button_callback( GLFWwindow* window, int button,
                            int action, int mods )
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == GLFW_PRESS)
    {
        double wx, wy;
        cam.mouse_to_world(mouse_x, mouse_y, wx, wy);

        debugOn = true;
        Color pixelColor = rayColor(wx, wy);
        debugOn = false;

        cout << "Pixel Color = " << pixelColor << endl;
    }
    else {
        // Here, record the fact that NOTHING is being pressed
        // on anymore.
    }
}

//////////////////////////////////////////////////////
// Gets called when the mouse moves.
// (No need to change this).
//////////////////////////////////////////////////////
void mouse_position_callback( GLFWwindow* window, double x, double y )
{
    // Here, if mouse is currently pressing on some vertex,
    // change the vertex's position.

    mouse_x = (int)x;
    mouse_y = (int)y;
    double wx, wy;
    cam.mouse_to_world(mouse_x, mouse_y, wx, wy);
}

//////////////////////////////////////////////////////
// Gets called if there was an error in GLFW.
// (No need to change this)
//////////////////////////////////////////////////////
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

//////////////////////////////////////////////////////
//
// Show the image.
//
// **** NO NEED TO CHANGE THIS ****
//
//////////////////////////////////////////////////////

void display () {
    glClearColor(.1f,.1f,.1f, 1.f);   /* set the background colour */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cam.begin_drawing();

    glRasterPos3d(0.0,0.0,0.0);

    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    if (frame_buffer_stale) {
        //
        // Don't re-render the scene EVERY time display() is called.
        // It might get called if the window is moved, or if it
        // is exposed.  Only re-render if the window is RESIZED.
        // Resizing is detected by the Camera class,
        // frameBufferStale.
        //
        render();
        frame_buffer_stale = false;
    }

    //
    // This paints the current image buffer onto the screen.
    //
    glDrawPixels(winWidth,winHeight,
                 GL_RGB,GL_UNSIGNED_BYTE,img);

    glFlush();
}

void window_resized(int w, int h)
{
    winWidth  = w;
    winHeight = h;

    //

    if (img != NULL)
        delete [] img;
    img = new byte[winWidth*winHeight*3];

    //
    // Ask for image to be re-drawn, eventually.
    //
    reRender();
}

//////////////////////////////////////////////////////
//
// Basically, quit if the user hits "q" or "ESC".
//
// **** NO NEED TO CHANGE THIS ****
//
//////////////////////////////////////////////////////

static void key_callback(GLFWwindow* window, int key,
                         int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q ||
        key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (action == GLFW_RELEASE) {
        the_ui.handle_key(key);
    }
}


//////////////////////////////////////////////////////
//
// Main program.
//
// **** NO NEED TO CHANGE THIS ****
//
//////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage:\n";
        std::cerr << "  rt <scene_file.txt>\n";
        char line[100];
        std::cin >> line;
        exit(EXIT_FAILURE);
    }

    readScene(argv[1]);
    setupCamera();
    
    
    GLFWwindow* window;


    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        cerr << "glfwInit failed!\n";
        cerr << "PRESS Control-C to quit\n";
        char line[100];
        cin >> line;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(winWidth, winHeight,
                              "Ray Traced Scene", NULL, NULL);

    if (!window)
    {
        cerr << "glfwCreateWindow failed!\n";
        cerr << "PRESS Control-C to quit\n";
        char line[100];
        cin >> line;

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    int w = winWidth;
    int h = winHeight;

    window_resized(w, h);

    cam = Camera(0,0, w,h, w, h, window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window,   mouse_position_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        cam.check_resize();

        if (cam.get_win_W() != winWidth ||
            cam.get_win_H() != winHeight) {

            window_resized(cam.get_win_W(), cam.get_win_H());
        }

        display();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

