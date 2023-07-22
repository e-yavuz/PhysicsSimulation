#ifndef GLFW_WRAPPER_H
#define GLFW_WRAPPER_H
// @ts-nocheck
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <climits>
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

    static inline void updateBuffer(uint &id, uint offset, void *data, uint size, GLenum shaderType) 
    {
        glBindBuffer(shaderType, id);
        glBufferSubData(shaderType, offset, size, data);
    }

    GLFWwindow* window;
};

template<typename T1>
class VertexContainer
{
    public: 
        VertexContainer(unsigned int VBO, uint sz) : sz(sz), VBO(VBO)
        {
            resize(sz);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(T1)*capacity, data.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(T1), (void*)0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        VertexContainer(unsigned int VBO, uint sz, T1 val) : sz(sz), VBO(VBO)
        {
            resize(sz, val);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(T1)*capacity, data.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(T1), (void*)0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        VertexContainer(unsigned int VBO) : sz(0), VBO(VBO) 
        {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(T1)*capacity, data.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(T1), (void*)0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        size_t size() { return sz; }

        void reserve(size_t sz) { data.reserve(sz); capacity = sz;}
        void resize(size_t sz) { data.resize(sz); this->sz = sz; capacity = sz;}
        void resize(size_t sz, T1 elm) { data.resize(sz, elm); this->sz = sz; capacity = sz;}
        void push_back(T1 elm)
        {
            
            if(++sz > capacity/3)
            {
                if(capacity >= UINT_MAX/2) capacity = UINT_MAX;
                else capacity*=2;
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(T1)*capacity, data.data(), GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            data.push_back(elm);
        }

        T1& operator[](size_t idx) { return data[idx]; }

        void updateBuffer(uint offset)
        {
            GLFW_Wrapper::updateBuffer(VBO, offset, data.data(), sizeof(T1)*sz, GL_ARRAY_BUFFER);
        }
        std::vector<T1> data;
        friend class GLFW_Wrapper;
    private:
        uint sz;
        uint capacity = 3;
        unsigned int VBO;
};

#endif