/**

./main.cpp

Created on: 16 May 2013

Copyright (c) 2013, Philip Deegan

This file is part of fragex.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <regex>
#include <vector>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

class Quadrant{
    private:
        ushort r = 255, g = 255, b = 255;
    public:
        const ushort x;
        const ushort y;
        const ushort w;
        const ushort h;
        const std::string i;
        Quadrant(const ushort& x, const ushort& y, const ushort& w, const ushort& h, const std::string& i) : x(x), y(y), w(w), h(h), i(i){}
        std::vector<Quadrant> solve(){
            std::vector<Quadrant> pixels;
            if(w == 1 && h == 1) pixels.push_back(*this);           
            else{
                for(Quadrant p : Quadrant(x      , y      , w/2, h/2, i + "0").solve()) pixels.push_back(p);
                for(Quadrant p : Quadrant(x + w/2, y      , w/2, h/2, i + "1").solve()) pixels.push_back(p);
                for(Quadrant p : Quadrant(x      , y + h/2, w/2, h/2, i + "2").solve()) pixels.push_back(p);
                for(Quadrant p : Quadrant(x + w/2, y + h/2, w/2, h/2, i + "3").solve()) pixels.push_back(p);
            }
            return pixels;
        }
        void rgb  (const ushort& r, const ushort& g, const ushort& b){ this->r = r; this->g = g; this->b = b; }

};

static void error_callback(int error, const char* description){
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
}

static void initialize(GLuint &vao) {
    // Use a Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnable(GL_PROGRAM_POINT_SIZE);
    // 4 triangles to be rendered
    GLfloat vertices_position[24] = {
    
    
    };
    
    // Create a Vector Buffer Object that will store the vertices on video memory
    GLuint vbo;
    glGenBuffers(1, &vbo);
    
    // Allocate space and upload the data from CPU to GPU
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position), vertices_position, GL_STATIC_DRAW);
    
    // GLuint shaderProgram = create_program("shaders/vert.shader", "shaders/frag.shader");
    
    // // Get the location of the attributes that enters in the vertex shader
    // GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
        
    // // Specify how the data for position can be accessed
    // glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // // Enable the attribute
    // glEnableVertexAttribArray(position_attribute);
}

static void display(GLuint& vao) {
    float* pixels[640*480*3] = { 0 };;
    for(int i=0;i< 640*480*3; i++) {
    // colour(10.0-((i*20.0)/ 640*480*3 ), &pixels[i*3]);
    } 

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawPixels(640,480,GL_RGB,GL_FLOAT,pixels);
}

int main(int argc, char* argv[]) {
    if(argc == 1) exit(EXIT_FAILURE);
    if(!glfwInit()) exit(EXIT_FAILURE);
    glfwSetErrorCallback(error_callback);

    GLFWwindow* window(glfwCreateWindow(640, 480, "Simple example", NULL, NULL));
    if (!window){
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    std::regex re(argv[1]);
    std::vector<Quadrant> pixels;
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        for(Quadrant p : Quadrant(0  , 0  , w/2, h/2, "0").solve()) pixels.push_back(p);
        for(Quadrant p : Quadrant(w/2, 0  , w/2, h/2, "1").solve()) pixels.push_back(p);
        for(Quadrant p : Quadrant(0  , h/2, w/2, h/2, "2").solve()) pixels.push_back(p);
        for(Quadrant p : Quadrant(w/2, h/2, w/2, h/2, "3").solve()) pixels.push_back(p);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    

    GLfloat g_color_buffer_data[pixels.size()];
    for(int i = 0; i < pixels.size(); i++)
        if(std::regex_match(pixels[i].i, re))  g_color_buffer_data[i] = 0.771f;
        else g_color_buffer_data[i] = 0.014f;

    GLuint vao;
    initialize(vao);
    while (!glfwWindowShouldClose(window))
    {
        display(vao);

        glfwPollEvents();
        glfwSwapBuffers(window);

    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
