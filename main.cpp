#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "src/GLFW/GLFW_Wrapper.hpp"
#include "src/Physics/Solver.hpp"
#include "src/Physics/Entities/Circle.hpp"
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <math.h>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main()
{
    GLFW_Wrapper glfw;
    Solver solver;
    std::vector<Circle> circles;



    try {
        glfw = GLFW_Wrapper(3, 3, 900, 900, "Physics Simulation");
    } catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    unsigned int vertexShader, fragmentShader, shaderProgram;

    try {
        vertexShader = glfw.init_GL_Shader("src/Shaders/V1.glsl", GL_VERTEX_SHADER);
        fragmentShader = glfw.init_GL_Shader("src/Shaders/F1.glsl", GL_FRAGMENT_SHADER);
        shaderProgram = glfw.init_GL_Program(std::vector<unsigned int>{vertexShader, fragmentShader});
    } catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    

    glBindVertexArray(VAO);

    VertexContainer<float> vertices(VBO);

    solver.vertices = &vertices;

    glBindVertexArray(0);
    
    // render loop
    //
    int not60FPSCount = 0;
    while(!glfwWindowShouldClose(glfw.window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfw.processInput();
        
        if(solver.objectsCurrentPosition.size() < pow(2,20))
        {
            solver.addCircle(-0.9f,0.7f,0,0.01,80);
            solver.objectsAcceleration.back().x=0.2f;
        }
        


        solver.Update(8);
        
        // rendering commands here
        //
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);
        
        vertices.updateBuffer(0);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

        // check and call events and swap the buffers
        //
        glfwSwapBuffers(glfw.window);
        glfwPollEvents();

        auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start);

        if(not60FPSCount > 50)
            break;
        not60FPSCount = end.count() > 17?not60FPSCount+1:0;
        

        std::this_thread::sleep_for(17ms-end);
        
    }
    std::cout << solver.objectsCurrentPosition.size() <<"\n";
    // std::cout << solver.spatialHash.size();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}