#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <iostream>
#include <cstdint>
#include <vector>

#include "macros.h"
#include "constants.h"

struct App
{
    const uint32_t screenWidth;
    const uint32_t screenHeight;
    SDL_Window* window;
    SDL_GLContext glContext;

    App(uint32_t width, uint32_t height)
        : screenWidth(width), screenHeight(height), window(nullptr), glContext(nullptr) {}

    ~App()
    {
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        DEBUG("[Success] App exits with no errors.\n");
    }

    bool init()
    {
        // Init SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            std::cout << "[ERROR] SDL2 could not initialize video subsystem.\n";
            return false;
        }
        DEBUG("[Success] SDL initialized video subsystem.\n");

        // Set opengl version 4.1
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        // Disable deprecated functions of opengl
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // Smoother transitions
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Create SDL Window
        const uint32_t winFlags = SDL_WINDOW_OPENGL;
        window = SDL_CreateWindow("My OpenGL App", 0, 0, screenWidth, screenHeight, winFlags);
        if (window == nullptr)
        {
            std::cout << "[ERROR] SDL2 could not create a window.\n";
            SDL_Quit();
            return false;
        }
        DEBUG("[Success] Window is ready to go.\n");

        // Create OpenGL Context
        glContext = SDL_GL_CreateContext(window);
        if (glContext == nullptr)
        {
            std::cout << "[ERROR] SDL2 could not create OpenGL context.\n";
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }
        DEBUG("[Success] OpenGl context created.\n");

        // Init glad
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
        {
            std::cout << "[ERROR] Glad failed to initialize\n";
            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }
        DEBUG("[Success] Glad loaded. OpenGL functions available.\n");

        return true;
    }
};

struct VertexSpec
{
    const GLuint vertexArrayObject;
    const GLuint vertexBufferObject;

    VertexSpec(GLuint vao, GLuint vbo)
        : vertexArrayObject(vao), vertexBufferObject(vbo) {}
};

bool processInput()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT: {
            std::cout << "Goodbye!\n";
            return true;
        }
        }
    }

    return false;
}

void preDraw() {}

void draw() {}

void mainLoop(App& app)
{
    bool shouldClose = false;

    float fpsTimeAcc = 0.0f;
    uint32_t fpsCounter = 0;
    const float targetFrameTime = FRAME_TIME_120FPS;

    while (!shouldClose)
    {
        const uint64_t start = SDL_GetTicks64();

        shouldClose = processInput();

        preDraw();

        draw();

        // Updates the screen
        SDL_GL_SwapWindow(app.window);

        // Manage FPS
        fpsCounter += 1;
        fpsTimeAcc += targetFrameTime;
        if (fpsTimeAcc >= MS_SECOND) // Run it every 1 second
        {
            std::cout << "FPS: " << fpsCounter << '\n';
            fpsCounter = 0;
            fpsTimeAcc = 0.0f;
        }

        // FrameTime
        const uint64_t end = SDL_GetTicks64();
        const uint64_t frameTime = end - start;

        // Delay if needed
        if (frameTime < targetFrameTime)
        {
            const uint32_t delay = static_cast<uint32_t>(targetFrameTime) - frameTime;
            if (delay < MS_SECOND) // Won't crash on weird stuff, only small freeze
                SDL_Delay(delay);
        }
    }
}

void getOpenGLVersionInfo()
{
    std::cout << "Vendor: "           << glGetString(GL_VENDOR)                    << '\n'
              << "Renderer: "         << glGetString(GL_RENDERER)                  << '\n'
              << "Version: "          << glGetString(GL_VERSION)                   << '\n'
              << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << '\n';
}

VertexSpec genVertexSpec()
{
    // Setup in the CPU
    const std::vector<GLfloat> vertexPositions{
        //  x,     y,    z,
        -0.8f, -0.8f, 0.0f, // vertex 1
         0.8f, -0.8f, 0.0f, // vertex 2
         0.0f,  0.8f, 0.0f, // vertex 3
    };

    // Setup for the GPU

    // VAO - Vertex Array Object
    GLuint vertexArrayObject = 0;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (GLvoid*) 0);

    // Clean up
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);

    VertexSpec spec(vertexArrayObject, vertexBufferObject);
    return spec;
}

void createGraphicsPipeline() {}

int main(void)
{
    App app(1280, 720);
    if (!app.init()) return 1;

    getOpenGLVersionInfo();

    VertexSpec spec = genVertexSpec();
    createGraphicsPipeline();

    mainLoop(app);

    return 0;
}
