// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan
// Modifications and additional functionality by Cameron Hardy (ID 10084560)
// Date:    December 2015
// Modifications made: September 2016
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include "glm/glm.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

// Forward definitions
bool CheckGLErrors();
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// --------------------------------------------------------------------------
// GLFW callback functions
int scene = 1;
int level = 1;
int width = 700;
int height = 700;
// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    // use number keys 1-5 to select a scene
    // alternatively use left and right arrow keys
    
    // use up and down key to increase/decrease number of drawn levels
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        scene = 1;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        scene = 2;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        scene = 3;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
        scene = 4;
    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
        scene = 5;
    if (key == GLFW_KEY_6 && action == GLFW_PRESS)
        scene = 6;
    if (key == GLFW_KEY_7 && action == GLFW_PRESS)
        scene = 7;
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        level++;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && level > 0)
        level--;
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS && scene > 1)
        scene--;
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS && scene < 7)
        scene++;
}




//==========================================================================


// vec2 and vec3 are part of the glm math library.
// Include in your own project by putting the glm directory in your project,
// and including glm/glm.hpp as I have at the top of the file.
// "using namespace glm;" will allow you to avoid writing everyting as glm::vec2
vector<vec2> points;
vector<vec3> colors;

// Structs are simply acting as namespaces
// Access the values like so: VAO::LINES
struct VAO{
    enum {LINES=0, COUNT};                  // Enumeration assigns each name a value going up
    //LINES=0, COUNT=1
};

struct VBO{
    enum {POINTS=0, COLOR, COUNT};          // POINTS=0, COLOR=1, COUNT=2
};

struct SHADER{
    enum {LINE=0, COUNT};                   // LINE=0, COUNT=1
};

GLuint vbo [VBO::COUNT];                    // Array which stores OpenGL's vertex buffer object handles
GLuint vao [VAO::COUNT];                    // Array which stores Vertex Array Object handles
GLuint shader [SHADER::COUNT];              // Array which stores shader program handles


// Gets handles from OpenGL
void generateIDs()
{
    glGenVertexArrays(VAO::COUNT, vao);		// Tells OpenGL to create VAO::COUNT many
                                            // Vertex Array Objects, and store their
                                            // handles in vao array
    glGenBuffers(VBO::COUNT, vbo);          // Tells OpenGL to create VBO::COUNT many
                                            // Vertex Buffer Objects and store their
                                            // handles in vbo array
}

// Clean up IDs when you're done using them
void deleteIDs()
{
    for(int i=0; i<SHADER::COUNT; i++)
    {
        glDeleteProgram(shader[i]);
    }
    
    glDeleteVertexArrays(VAO::COUNT, vao);
    glDeleteBuffers(VBO::COUNT, vbo);
}


// Describe the setup of the Vertex Array Object
bool initVAO()
{
    glBindVertexArray(vao[VAO::LINES]);                 //Set the active Vertex Array
    
    glEnableVertexAttribArray(0);                       // Tell opengl you're using layout attribute 0 (For shader input)
    glBindBuffer( GL_ARRAY_BUFFER, vbo[VBO::POINTS] );	// Set the active Vertex Buffer
    glVertexAttribPointer(
                          0,                            // Attribute
                          2,                            // Size # Components
                          GL_FLOAT,                     // Type
                          GL_FALSE,                     // Normalized?
                          sizeof(vec2),                 // Stride
                          (void*)0                      // Offset
                          );
    
    glEnableVertexAttribArray(1);                       // Tell opengl you're using layout attribute 1
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::COLOR]);
    glVertexAttribPointer(
                          1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(vec3),
                          (void*)0
                          );
    
    return !CheckGLErrors();                            // Check for errors in initialize
}


//Loads buffers with data
bool loadBuffer(const vector<vec2>& points, const vector<vec3>& colors)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::POINTS]);
    glBufferData(
                 GL_ARRAY_BUFFER,                       // Which buffer you're loading too
                 sizeof(vec2)*points.size(),            // Size of data in array (in bytes)
                 &points[0],							// Start of array (&points[0] will give you pointer to start of vector)
                 GL_STATIC_DRAW                         // GL_DYNAMIC_DRAW if you're changing the data often
                                                        // GL_STATIC_DRAW if you're changing seldomly
                 );
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::COLOR]);
    glBufferData(
                 GL_ARRAY_BUFFER,
                 sizeof(vec3)*colors.size(),
                 &colors[0],
                 GL_STATIC_DRAW
                 );
    
    return !CheckGLErrors();
}

// Compile and link shaders, storing the program ID in shader array
bool initShader()
{
    // Put vertex file text into string
    string vertexSource = LoadSource("vertex.glsl");
    // Put fragment file text into string
    string fragmentSource = LoadSource("fragment.glsl");
    
    GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    shader[SHADER::LINE] = LinkProgram(vertexID, fragmentID);	// Link and store program ID in shader array
    
    return !CheckGLErrors();
}

void generateSquares(int level) {
    points.clear();
    colors.clear();
    
    // set unique colors for squares and diamonds
    vec3 squareColour(1.0, 0.5, 0.0);
    vec3 diamondColor(0.0, 0.5, 1.0);
    // set initial dimensions of the square
    float size = 0.9;
    
    // each level draws a square containing a diamond
    for(int i=0; i<level; i++) {
        // draw a mediocre square
        points.push_back(vec2(-size, -size));
        points.push_back(vec2(-size,  size));
        colors.push_back(squareColour);
        colors.push_back(squareColour);
        points.push_back(vec2(-size,  size));
        points.push_back(vec2( size,  size));
        colors.push_back(squareColour);
        colors.push_back(squareColour);
        points.push_back(vec2( size,  size));
        points.push_back(vec2( size, -size));
        colors.push_back(squareColour);
        colors.push_back(squareColour);
        points.push_back(vec2( size, -size));
        points.push_back(vec2(-size, -size));
        colors.push_back(squareColour);
        colors.push_back(squareColour);
    
        // draw a mediocre diamond
        points.push_back(vec2( 0.0, -size));
        points.push_back(vec2(-size,  0.0));
        colors.push_back(diamondColor);
        colors.push_back(diamondColor);
        points.push_back(vec2(-size,  0.0));
        points.push_back(vec2( 0.0,  size));
        colors.push_back(diamondColor);
        colors.push_back(diamondColor);
        points.push_back(vec2( 0.0,  size));
        points.push_back(vec2( size,  0.0));
        colors.push_back(diamondColor);
        colors.push_back(diamondColor);
        points.push_back(vec2( size,  0.0));
        points.push_back(vec2( 0.0, -size));
        colors.push_back(diamondColor);
        colors.push_back(diamondColor);
        
        size /= 2;
        // fade the color towards black a little bit each iteration
        squareColour *= 0.8;
        diamondColor *= 0.8;
    }
}

void generateSpiral(int level){
    points.clear();
    colors.clear();
    
    // define start and end colors for the spiral to fade between
    vec3 startColor(0.0, 1.0, 0.5);
    vec3 endColor(1.0, 0.0, 0.5);
    
    float t = 0;
    float bound = level*2*3.141592653589793238462643383;
    // an interval of 0.01 between points produces a nice smooth spiral
    float interval = 0.01;
    float a = (1/bound);
    
    // generate the spiral with a simple parametric equation
    // blend between the start and end colors
    for(t=0; t<bound; t+=interval) {
        points.push_back(vec2(a*t*cos(t), a*t*sin(t)));
        colors.push_back(startColor*(1-(a*t)) + endColor*(a*t));
    }
}

void drawTriangle(int level, vec2 pointA, vec2 pointB, vec2 pointC, vec3 color) {
    // added a failsafe to make sure the render doesn't melt your CPU
    while(level > 10)
        level--;
    
    if(level<=0) {
        // draw triangle with vertexes A, B, and C
        points.push_back(pointA);
        points.push_back(pointB);
        points.push_back(pointC);
        colors.push_back(color);
        colors.push_back(color);
        colors.push_back(color);
    }
    else {
        // calculate the midpoints of the current triangle, and then recursively draw 3 smaller triangles using two of the midpoints and one of the original corners
        // it also recursively increases one RGB value while decreasing the other two values - produces a cool swirl effect
        drawTriangle(level-1, pointA, vec2((pointA.x+pointB.x)/2, (pointA.y+pointB.y)/2), vec2((pointA.x+pointC.x)/2, (pointA.y+pointC.y)/2), color*vec3(1.2, 0.8, 0.8));
        drawTriangle(level-1, pointB, vec2((pointB.x+pointA.x)/2, (pointB.y+pointA.y)/2), vec2((pointB.x+pointC.x)/2, (pointB.y+pointC.y)/2), color*vec3(0.8, 1.2, 0.8));
        drawTriangle(level-1, pointC, vec2((pointC.x+pointA.x)/2, (pointC.y+pointA.y)/2), vec2((pointC.x+pointB.x)/2, (pointC.y+pointB.y)/2), color*vec3(0.8, 0.8, 1.2));
    }
}

void generateSierpinski(int level) {
    points.clear();
    colors.clear();

    // set the intial color value and then get started on the recursion (yee haw)
    vec3 color(0.5, 0.5, 0.5);
    drawTriangle(level, vec2(-1.0, -0.933), vec2(0.0, 0.933), vec2(1.0, -0.933), color);
}

void generateSnowFractal(int level){
    // TODO: make da snow fractal (if you feel sassy enough)
    // this fractal reminded me of winter and that put me in a foul mood
    // instead of snow fractals, I made a dragon curve, which you'll have to admit is way more badass
}

void generateFern(int level) {
    points.clear();
    colors.clear();
    
    // set initial color value
    vec3 color(0.5, 0.5, 0.5);
    
    float x = 0;
    float y = 0;
    float xPrev;
    float yPrev;
    
    // since this fractal is probability-based, it needs TONS of iterations
    level = level*50000;
    for (int i = 0; i<level; i++) {
        xPrev = x;
        yPrev = y;
        // choose a random number between 0 and 99
        // depending on the result, the next point to be drawn gets mapped to a new part of the fern based on specific probabilities
        int seed = rand()%100;
        // maps to the first stem
        if(seed == 0) {
            x = 0;
            y = 0.16*yPrev;
            // make the stem white
            color = vec3(1.0, 1.0, 1.0);
        }
        // maps to the next level
        else if(0<seed && seed<=85) {
            x = 0.85*xPrev + 0.04*yPrev;
            y = -0.04*xPrev + 0.85*yPrev + 1.6;
            // make the fern fade out near the tip
            color *= vec3(0.95, 0.95, 0.95);
        }
        // maps to the left side
        else if(85<seed && seed<=92) {
            x = 0.2*xPrev - 0.26*yPrev;
            y = 0.23*xPrev + 0.22*yPrev + 1.6;
            // make the left side red
            color = vec3(1.0, 0.2, 0.2);
        }
        //maps to the right side
        else if(92<seed && seed<=99) {
            x = -0.15*xPrev + 0.28*yPrev;
            y = 0.26*xPrev + 0.24*yPrev + 0.44;
            // make the right side blue
            color = vec3(0.2, 0.2, 1.0);
        }
        
        // apply some scaling and translating to make sure it fits in the render window
        points.push_back(vec2((x/3), (y/5.3)-1.0));
        colors.push_back(color);
    }
}

vec2 plotLeft(vec2 pointA, vec2 pointB, int mode) {
    // calculate midpoint of A and B
    vec2 midpoint((pointA.x+pointB.x)/2, (pointA.y+pointB.y)/2);
    // calculate the change in each dimension
    vec2 delta = midpoint - pointA;
    
    // depending on which mode we're in, calcluate the extrusion on the left
    if(mode%4 == 0)
        return vec2(midpoint.x, midpoint.y+delta.x);
    if(mode%4 == 1)
        return vec2(pointA.x, pointB.y);
    if(mode%4 == 2)
        return vec2(midpoint.x-delta.y, midpoint.y);
    if(mode%4 == 3)
        return vec2(pointB.x, pointA.y);
    
    // this is here for bug catching
    // under normal circumstances this should never be returned
    return vec2(2.0, 2.0);
}

vec2 plotRight(vec2 pointA, vec2 pointB, int mode) {
    // calculate midpoint of A and B
    vec2 midpoint((pointA.x+pointB.x)/2, (pointA.y+pointB.y)/2);
    // calculate the change in each dimension
    vec2 delta = midpoint - pointA;
    
    // depending on which mode we're in, calcluate the extrusion on the right
    if(mode%4 == 0)
        return vec2(midpoint.x, midpoint.y-delta.x);
    if(mode%4 == 1)
        return vec2(pointB.x, pointA.y);
    if(mode%4 == 2)
        return vec2(midpoint.x+delta.y, midpoint.y);
    if(mode%4 == 3)
        return vec2(pointA.x, pointB.y);
    
    // this is here for bug catching
    // under normal circumstances, this should never be returned
    return vec2(2.0, 2.0);
}

void generateDragon(int level) {
    points.clear();
    colors.clear();
    
    // added a failsafe to make sure the render doesn't melt the CPU
    while(level > 17)
        level--;
    
    bool right = true;
    vector<vec2> array;
    
    // initial coordinates and colors
    array.push_back(vec2(-0.7, 0.2));
    array.push_back(vec2(0.5, 0.2));
    vec3 startColor(0.0, 0.6, 0.9);
    vec3 endColor(1.0, 0.4, 0.1);
    
    // there are 4 "modes" to aid with folding calculations
    // the mode tells the function which way the line is propagating, so it know which way to extrude
    // Mode 0 = west/east
    // Mode 1 = northeast/southwest
    // Mode 2 = north/south
    // Mode 3 = northwest/southeast
    int mode = 0;
    while(level > 1) {
        // iterate through the vector of values, use adjacent values to calculate intermediate values and insert them back into the vector
        for(int index=0; index < array.size()-1; index += 2) {
            // alternate between turning left and right
            if(right) {
                array.insert(array.begin() + index+1, plotRight(array.at(index), array.at(index+1), mode));
                right = false;
            }
            else if(!right) {
                array.insert(array.begin() + index+1, plotLeft(array.at(index), array.at(index+1), mode));
                right = true;
            }
            // a 90 turn = a mode shift of 2
            mode += 2;
        }
        // shift the mode to prepare for the next level
        mode++;
        level--;
    }
    
    // draw all points and blend between two colors
    float size = array.size();
    for(int i=0; i<array.size(); i++) {
        points.push_back(array[i]);
        colors.push_back(startColor*(1-(i/size)) + endColor*(i/size));
    }
}

vec3 hsv_to_rgb(float h, float s, float v) {
    if(v>1.0)
        v = 1.0;
    float hp = h/60.0;
    float c = v*s;
    float x = c*(1-abs((fmod(hp, 2)-1)));
    vec3 rgb(0.0, 0.0, 0.0);
    
    if(0<=hp && hp<1)
        rgb = vec3(c, x, 0);
    if(1<=hp && hp<2)
        rgb = vec3(x, c, 0);
    if(2<=hp && hp<3)
        rgb = vec3(0, c, x);
    if(3<=hp && hp<4)
        rgb = vec3(0, x, c);
    if(4<=hp && hp<5)
        rgb = vec3(x, 0, c);
    if(5<=hp && hp<6)
        rgb = vec3(c, 0, x);
    
    float m = v - c;
    rgb += vec3(m, m, m);
    
    return rgb;
}

vec3 mapColor(int i, float r, float c) {
    int di = i;
    float zn;
    float hue;
    
    zn = sqrt(r + c);
    hue = di + 1.0 - log(log(abs(zn))) / log(2.0);
    hue = 0.95 + 20.0 * hue;
    while(hue > 360.0)
        hue -= 360.0;
    while(hue < 0.0)
        hue += 360.0;
    
    return hsv_to_rgb(hue, 0.8, 1.0);
}

void generateMandelbrot(int level) {
    points.clear();
    colors.clear();
    
    float x0;
    float y0;
    float x;
    float y;
    float xtemp;
    float ytemp;
    int levelx;
    
    for(int i=0; i<height; i++) {
        for(int j=0; j<width; j++) {
            x0 = (3.5/(float)width)*j-2.5;
            y0 = (3.0/(float)height)*i-1.5;
            x = 0.0;
            y = 0.0;
            levelx = level;
            while(x*x + y*y < 4.0 && levelx > 0) {
                xtemp = x*x - y*y + x0;
                ytemp = 2*x*y + y0;
                if (x == xtemp  &&  y == ytemp) {
                    levelx = 0;
                    break;
                }
                x = xtemp;
                y = ytemp;
                levelx--;
            }
            
            // some weird scaling stuff, I dunno
            x0 += 0.75;
            x0 /= 1.75;
            y0 /= 1.5;
            
            points.push_back(vec2(x0, y0));
            if(levelx == 0)
                colors.push_back(vec3(0.0, 0.0, 0.0));
            else
                colors.push_back(mapColor(levelx, 360.0*x0+365.0, y0));
        }
    }
}

void generateJulia(int level) {
    points.clear();
    colors.clear();
    
    float x0;
    float y0;
    float x;
    float y;
    float xtemp;
    float ytemp;
    int levelx;
    
    for(int i=0; i<height; i++) {
        for(int j=0; j<width; j++) {
            x0 = (3.5/(float)width)*j-1.75;
            y0 = (3.0/(float)height)*i-1.5;
            x = x0;
            y = y0;
            levelx = level;
            while(x*x + y*y < 4.0 && levelx > 0) {
                xtemp = x*x - y*y - 0.8;
                ytemp = 2*x*y + 0.156;
                if (x == xtemp  &&  y == ytemp) {
                    levelx = 0;
                    break;
                }
                x = xtemp;
                y = ytemp;
                levelx--;
            }
            
            // some weird scaling stuff, I dunno
            x0 /= 1.75;
            y0 /= 1.5;
            
            points.push_back(vec2(x0, y0));
            if(levelx == 0)
                colors.push_back(vec3(0.0, 0.0, 0.0));
            else
                colors.push_back(mapColor(levelx, 360.0*x0+365.0, y0));
        }
    }
}

// Initialization
void initGL()
{
    // Only call these once - don't call again every time you change geometry
    generateIDs();		// Create VertexArrayObjects and Vertex Buffer Objects and store their handles
    initShader();		// Create shader and store program ID

    initVAO();			// Describe setup of Vertex Array Objects and Vertex Buffer Objects
}

// Draws buffers to screen
void render()
{
    glClearColor(0.f, 0.f, 0.f, 0.f);		// Color to clear the screen with (R, G, B, Alpha)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear color and depth buffers (Haven't covered yet)
    
    // Don't need to call these on every draw, so long as they don't change
    glUseProgram(shader[SHADER::LINE]);		// Use LINE program
    glBindVertexArray(vao[VAO::LINES]);		// Use the LINES vertex array
    
    switch(scene){
        case 1:
            generateSquares(level);		// Create geometry - CHANGE THIS FOR DIFFERENT SCENES
            loadBuffer(points, colors);
            glDrawArrays(GL_LINES, 0, points.size()); // boxes and diamonds
            break;
        case 2:
            generateSpiral(level);
            loadBuffer(points, colors);
            glDrawArrays(GL_LINE_STRIP, 0, points.size()); // spiral
            break;
        case 3:
            generateSierpinski(level);
            loadBuffer(points, colors);
            glDrawArrays(GL_TRIANGLES, 0, points.size()); // sierpinski carpet
            break;
        case 4:
            generateFern(level);
            loadBuffer(points, colors);
            glDrawArrays(GL_POINTS, 0, points.size()); // fern fractal
            break;
        case 5:
            generateDragon(level);
            loadBuffer(points, colors);
            glDrawArrays(GL_LINE_STRIP, 0, points.size()); // dragon curve
            break;
        case 6:
            generateMandelbrot(level);
            loadBuffer(points, colors);
            glDrawArrays(GL_POINTS, 0, points.size()); // mandelbrot set
            break;
        case 7:
            generateJulia(level);
            loadBuffer(points, colors);
            glDrawArrays(GL_POINTS, 0, points.size()); // julia set
            break;
    }
}




// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);
    
    // attempt to create a window with an OpenGL 3.3 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }
    
    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    
    // query and print out information about our OpenGL environment
    QueryGLVersion();
    
    initGL();
    
    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        // call function to draw our scene
        render();
        
        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);
        
        // sleep until next event before drawing again
        glfwWaitEvents();
    }
    
    // clean up allocated resources before exit
    deleteIDs();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    
    cout << "OpenGL [ " << version << " ] "
    << "with GLSL [ " << glslver << " ] "
    << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
            case GL_INVALID_ENUM:
                cout << "GL_INVALID_ENUM" << endl; break;
            case GL_INVALID_VALUE:
                cout << "GL_INVALID_VALUE" << endl; break;
            case GL_INVALID_OPERATION:
                cout << "GL_INVALID_OPERATION" << endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
            case GL_OUT_OF_MEMORY:
                cout << "GL_OUT_OF_MEMORY" << endl; break;
            default:
                cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;
    
    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
        << filename << endl;
    }
    
    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);
    
    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);
    
    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }
    
    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();
    
    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);
    
    // try linking the program with given attachments
    glLinkProgram(programObject);
    
    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }
    
    return programObject;
}


// ==========================================================================
