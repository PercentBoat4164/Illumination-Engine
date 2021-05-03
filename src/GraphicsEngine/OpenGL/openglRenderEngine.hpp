#pragma once

#define GLEW_IMPLEMENTATION
#include <glew/include/GL/glew.h>

#include <GLFW/glfw3.h>

#include <array>
#include <string>
#include <fstream>
#include <vector>


#include "../OpenGL/openglSettings.hpp"

/** This is the main OpenGL Render Engine Class.
 * It is run when the user selects the "o" option at the runtime. */
class OpenGLRenderEngine {
public:
    /** This creates a local instance of the settings{} class. */
    OpenGLSettings settings{};
    /** This creates the window{} variable. */
    GLFWwindow *window{};
    /** This creates the vertexBuffer{} variable. */
    GLuint vertexBuffer{};
    /** This creates the programID{} variable. */
    GLuint programID{};

    /** This is the constructor and initializes the render engine
     * @param initialSettings This is the variable where all of the settings/variables for the render engine can be accessed.*/
    explicit OpenGLRenderEngine(OpenGLSettings &initialSettings = *new OpenGLSettings{}) {
        settings = initialSettings;
        if(!glfwInit()) { throw std::runtime_error("failed to initialize GLFW"); }
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), nullptr, nullptr);
        if (window == nullptr) { throw std::runtime_error("failed to open GLFW window!"); }
        glfwMakeContextCurrent(window);
        glewExperimental = true;
        if (glewInit() != GLEW_OK) { throw std::runtime_error("failed to initialize GLEW!"); }
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        //Create vertex buffer
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
         programID = loadShaders({"Shaders/vertexShader.glsl", "Shaders/fragmentShader.glsl"});
    }
/** This method updates/renders the screen and is run multiple times per second.
 * @return This method returns either a 0 or 1 depending on if the window is open or closed and if the program is running correctly.*/
[[nodiscard]] int update() {
    if (glfwWindowShouldClose(window)) { return 1; }
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    glfwSwapBuffers(window);
    auto currentTime = (float)glfwGetTime();
    frameTime = currentTime - previousTime;
    previousTime = currentTime;
    ++frameNumber;
    return 0;
}

/** This finishes the program and calls the cleanUp() method.*/
~OpenGLRenderEngine() {
    cleanUp();
}

/** This is the variable that contains the frames per second.*/
float frameTime{};
/** This is the previous time variable that is used to determine the frames per second.*/
float previousTime{};
/** This is the variable that contains the number of frames that have been displayed.*/
int frameNumber{};

private:
/** This constant holds the buffer data for the vertex buffer.*/
constexpr static const GLfloat g_vertex_buffer_data[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f};

/** This is the method that finishes and cleans up the program.*/
static void cleanUp() {
    glFinish();
    glfwTerminate();
}

/** This method loads the shaders into the render engine.
 * @param paths This is the location of the shaders.
 * @return ProgramID*/
static GLuint loadShaders(const std::array<std::string, 2>& paths) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::array<GLuint, 2> shaderIDs = {vertexShaderID, fragmentShaderID};
    GLint Result = GL_FALSE;
    int InfoLogLength{0};
    for (unsigned int i = 0; i < paths.size(); i++) {
        std::ifstream file(paths[i], std::ios::in);
        if (!file.is_open()) { throw std::runtime_error("failed to load shader: " + paths[i]); }
        std::stringstream stringStream;
        stringStream << file.rdbuf();
        std::string shaderCode = stringStream.str();
        file.close();
        char const *sourcePointer = shaderCode.c_str();
        glShaderSource(shaderIDs[i], 1, &sourcePointer, nullptr);
        glCompileShader(shaderIDs[i]);
        glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 1) { throw std::runtime_error("failed to compile shader: " + paths[i]); }
    }
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, vertexShaderID);
    glAttachShader(ProgramID, fragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    glDetachShader(ProgramID, vertexShaderID);
    glDetachShader(ProgramID, fragmentShaderID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    return ProgramID;
}
};
