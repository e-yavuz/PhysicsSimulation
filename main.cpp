#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <random>
#include <stdexcept>
#include <vector>
#include <math.h>
#include <chrono>

void processInput(GLFWwindow* window);
unsigned int init_GL_Shader(std::string filePath, GLenum shaderType);
unsigned int init_GL_Program(std::vector<unsigned int> shaders);
void updateBuffer(uint &id, uint offset, void *data, uint size, GLenum shaderType);

class GLFW_Wrapper
{
public:
    GLFW_Wrapper() {};
    GLFW_Wrapper(int major, int minor, int width, int height, const char* title)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif

        window = glfwCreateWindow(width, height, title, NULL, NULL);
        
        if (window == NULL)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSwapInterval(0);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Failed to initialize GLAD");
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    GLFWwindow* window;
};

int main()
{
    GLFW_Wrapper glfw;

    try {
        glfw = GLFW_Wrapper(3, 3, 900, 900, "Physics Simulation");
    } catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    GLFWwindow* window = glfw.window;

    unsigned int vertexShader, fragmentShader, shaderProgram;

    try {
        vertexShader = init_GL_Shader("GLSL/V1.glsl", GL_VERTEX_SHADER);
        fragmentShader = init_GL_Shader("GLSL/F1.glsl", GL_FRAGMENT_SHADER);
        shaderProgram = init_GL_Program(std::vector<unsigned int>{vertexShader, fragmentShader});
    } catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }



    std::vector<float> vertices(10*9, 0.f);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*static_cast<uint>(vertices.size()), vertices.data(), GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // render loop
    //
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        
        // rendering commands here
        //
        glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);
        updateBuffer(VBO, 0, vertices.data(), sizeof(vertices[0])*static_cast<uint>(vertices.size()), GL_ARRAY_BUFFER);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

        // check and call events and swap the buffers
        //
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

unsigned int init_GL_Shader(std::string filePath, GLenum shaderType)
{
    std::ifstream in(filePath);

    std::string content(std::string((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>()));
    const char* src = content.data();

    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success;
    char info_log[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        throw std::runtime_error(info_log);
    }

    return shader;
}

unsigned int init_GL_Program(std::vector<unsigned int> shaders)
{

    unsigned int shaderProgram = glCreateProgram();
    
    for(unsigned int elm: shaders)
        glAttachShader(shaderProgram, elm);
        
    glLinkProgram(shaderProgram);

    int success;
    char info_log[512];

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, info_log);
        throw std::runtime_error(info_log);
    }

    for(unsigned int elm: shaders)
        glDeleteShader(elm);
        
    return shaderProgram;
}

void updateBuffer(uint &id, uint offset, void *data, uint size, GLenum shaderType) 
 {
    glBindBuffer(shaderType, id);
    glBufferSubData(shaderType, offset, size, data);
}