// @ts-nocheck
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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

    inline void processInput()
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

    inline void updateBuffer(uint &id, uint offset, void *data, uint size, GLenum shaderType) 
    {
        glBindBuffer(shaderType, id);
        glBufferSubData(shaderType, offset, size, data);
    }

    GLFWwindow* window;
};

