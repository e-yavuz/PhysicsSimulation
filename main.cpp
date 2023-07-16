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
        vertexShader = glfw.init_GL_Shader("src/GLSL/V1.glsl", GL_VERTEX_SHADER);
        fragmentShader = glfw.init_GL_Shader("src/GLSL/F1.glsl", GL_FRAGMENT_SHADER);
        shaderProgram = glfw.init_GL_Program(std::vector<unsigned int>{vertexShader, fragmentShader});
    } catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    

    size_t maxVertexCount = 612*3000;
    std::vector<float> vertices;
    vertices.reserve(maxVertexCount);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*static_cast<uint>(maxVertexCount), vertices.data(), GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
    // render loop
    //
    int count = 0;
    while(!glfwWindowShouldClose(glfw.window))
    {
        auto start = std::chrono::high_resolution_clock::now();
        glfw.processInput();
        
        if(count++ < 5000)
        {
            solver.verletObjects.emplace_back(std::make_unique<Circle>(Circle(-0.9f,0.7f,0,0.02,70,vertices)));
            solver.verletObjects.back()->acceleration.x=0.5f;
            solver.verletObjects.emplace_back(std::make_unique<Circle>(Circle(-0.9f,0.5f,0,0.02,70,vertices)));
            solver.verletObjects.back()->acceleration.x=0.5f;
            solver.verletObjects.emplace_back(std::make_unique<Circle>(Circle(-0.9f,0.3f,0,0.02,70,vertices)));
            solver.verletObjects.back()->acceleration.x=0.5f;
        }
        // std::cout << count*3 << "\n";
        


        solver.Update(8);
        
        // rendering commands here
        //
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);
        
        glfw.updateBuffer(VBO, 0, vertices.data(), sizeof(float)*static_cast<uint>(vertices.size()), GL_ARRAY_BUFFER);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

        // check and call events and swap the buffers
        //
        glfwSwapBuffers(glfw.window);
        glfwPollEvents();

        auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start);

        std::this_thread::sleep_for(17ms-end);
        
    }
    // std::cout << count <<"\n";

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}