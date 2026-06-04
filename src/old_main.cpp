#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include "constants.h"
#include "macros.h"

struct Vertex_Spec {
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
};

SDL_GLContext InitializeProgram(SDL_Window* window, int width, int height);
Vertex_Spec VertexSpecification();
GLuint CompileShader(GLuint type, const std::string& source);
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
GLuint CreateGraphicsPipeline();
void MainLoop(SDL_Window* window, int width, int height, GLuint shaderProgram, Vertex_Spec vertexSpec);
void CleanUp(SDL_Window* window);
bool Input();
void PreDraw(int width, int height, GLuint shaderProgram);
void Draw(Vertex_Spec vertexSpec);
void GetOpenGLVersionInfo();

int main(void)
{
    const uint32_t screenHeight = 480;
    const uint32_t screenWidth  = 640;
    SDL_Window* window = nullptr;

    SDL_GLContext glContext = InitializeProgram(window, screenWidth, screenHeight);

    Vertex_Spec vertexSpec = VertexSpecification();

    GLuint shaderProgram = CreateGraphicsPipeline();

    MainLoop(window, screenWidth, screenHeight, shaderProgram, vertexSpec);

    CleanUp(window);

    return 0;
}

SDL_GLContext InitializeProgram(SDL_Window* window, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "[ERROR] SDL2 could not initialize video subsystem.\n";
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    const uint32_t winFlags = SDL_WINDOW_OPENGL;
    window = SDL_CreateWindow("My OpenGL App", 0, 0, width, height, winFlags);
    if (window == NULL)
    {
        std::cout << "[ERROR] SDL2 could not create a window.\n";
        exit(1);
    }
    DEBUG("[Success] Window is ready to go.\n");

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (glContext == NULL)
    {
        std::cout << "[ERROR] SDL2 could not create OpenGL context.\n";
        exit(1);
    }
    DEBUG("[Success] OpenGl context created.\n");

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        std::cout << "[ERROR] Glad failed to initialize\n";
        exit(1);
    }

    GetOpenGLVersionInfo();

    return glContext;
}

Vertex_Spec VertexSpecification()
{
    // Setup in the CPU
    const std::vector<GLfloat> vertexPositions {
        // x, y, z
        -0.8f, -0.8f,  0.0f, // vertex 1
        0.8f,  -0.8f,  0.0f, // vertex 2
        0.0f,   0.8f,  0.0f, // vertex 3
    };

    // Setup things in the GPU

    // VAO - Vertex Array Object
    GLuint vertexArrayObject = 0; // Acts like an ID
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // VBO - Vertex Buffer Object
    GLuint vertexBufferObject = 0;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexPositions.size() * sizeof(GLfloat),
                 vertexPositions.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);

    return { vertexArrayObject, vertexBufferObject };
}

GLuint CompileShader(GLuint type, const std::string& source)
{
    GLuint shaderObject = 0;

    switch (type)
    {
    case GL_VERTEX_SHADER: {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    } break;
    case GL_FRAGMENT_SHADER: {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    } break;
    default:
        assert(false && "Invalid shader type");
    }

    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
    GLuint programObject = glCreateProgram();

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    glLinkProgram(programObject);

    glValidateProgram(programObject);

    // TODO: glDetachShader, glDeleteShader

    return programObject;
}

GLuint CreateGraphicsPipeline()
{
    // Vertex shaders executes once per vertex, and will be in charge of the final
    // position of the vertex.
    const std::string vertexShaderSource =
        "version 410 core\n"
        "in vec4 position;\n"
        "void main() {"
        "  gl_Position = vec4(position.x, position.y, position.z, position.w);"
        "}";

    // Fragment Shader executes once per fragment (i.e. roughly for every pixel that
    // will be rasterized), and in part determines the final color that will be sent
    // to the screen.
    const std::string fragmentShaderSource =
        "version 410 core\n"
        "out vec4 color;\n"
        "void main() {"
        "  color = vec4(1.0f, 0.5f, 0.0f, 1.0f);"
        "}";

    // Program Object for our shaders
    GLuint graphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    return graphicsPipelineShaderProgram;
}

bool Input()
{
    bool shouldClose = false;
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            std::cout << "Goodbye!\n";
            shouldClose = true;
        }
    }
    return shouldClose;
}

void PreDraw(int width, int height, GLuint shaderProgram)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
}

void Draw(Vertex_Spec vertexSpec)
{
    glBindVertexArray(vertexSpec.vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexSpec.vertexBufferObject);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void MainLoop(SDL_Window* window, int width, int height, GLuint shaderProgram, Vertex_Spec vertexSpec)
{
    uint32_t fpsCounter = 0;
    float fpsTimeAcc = 0.0f;
    bool shouldClose = false;

    while (!shouldClose)
    {
        const uint64_t start = SDL_GetTicks64();

        shouldClose = Input();

        PreDraw(width, height, shaderProgram);

        Draw(vertexSpec);

        SDL_GL_SwapWindow(window);

        fpsCounter += 1;
        fpsTimeAcc += FRAME_TIME_60FPS;
        if (fpsTimeAcc >= 1000)
        {
            std::cout << "FPS: " << fpsCounter << '\n';
            fpsCounter = 0;
            fpsTimeAcc = 0.0f;
        }

        const uint64_t end = SDL_GetTicks64();
        const uint64_t frameTime = end - start;

        SDL_Delay(FRAME_TIME_60FPS - frameTime);
    }// game loop
}

void CleanUp(SDL_Window* window)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GetOpenGLVersionInfo()
{
    std::cout << "Vendor: "            << glGetString(GL_VENDOR)                    << '\n'
              << "Renderer: "          << glGetString(GL_RENDERER)                  << '\n'
              << "Version: "           << glGetString(GL_VERSION)                   << '\n'
              << "Shading Language: "  << glGetString(GL_SHADING_LANGUAGE_VERSION)  << '\n';
}
