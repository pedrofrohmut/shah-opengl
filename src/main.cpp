#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <iostream>
#include <cstdint>
#include <vector>

#include "macros.h"
#include "constants.h"

struct VertexSpec
{
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;

    VertexSpec(GLuint vao, GLuint vbo)
        : vertexArrayObject(vao), vertexBufferObject(vbo) {}
};

struct App
{
    const uint32_t screenWidth;
    const uint32_t screenHeight;
    SDL_Window* window;
    SDL_GLContext glContext;
    VertexSpec vertexSpec;
    GLuint shaderProgram;

    App(uint32_t width, uint32_t height)
        : screenWidth(width),
          screenHeight(height),
          window(nullptr),
          glContext(nullptr),
          vertexSpec(0, 0),
          shaderProgram(0)
    {
    }

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

void preDraw(App& app)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, app.screenWidth, app.screenHeight);
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(app.shaderProgram);
}

void draw(App& app)
{
    glBindVertexArray(app.vertexSpec.vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, app.vertexSpec.vertexBufferObject);
    glDrawArrays(GL_TRIANGLES, 0 , 3);
}

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

        preDraw(app);

        draw(app);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);

    // Clean up
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);

    VertexSpec spec(vertexArrayObject, vertexBufferObject);
    return spec;
}

GLuint compileShader(GLenum shaderType, const std::string& shaderSource)
{
    GLuint shaderObject = 0;

    switch (shaderType)
    {
    case GL_VERTEX_SHADER: {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    } break;
    case GL_FRAGMENT_SHADER: {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    } break;
    default:
        std::cout << "Invalid shader type.\n";
        exit(1);
    }

    const char* src = shaderSource.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    return shaderObject;
}

GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
    GLuint programObject = glCreateProgram();
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);

    // TODO: glDetachShader, glDeleteShader
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

    return programObject;
}

GLuint createGraphicsPipeline()
{
    // Vertex shaders executes once per vertex, and will be in charge of the final
    // position of the vertex.
    const std::string vertexShaderSource =
        "#version 410 core\n"
        "in vec4 position;\n"
        "void main()\n"
        "{\n"
        "  gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
        "}\n";

    // Fragment Shader executes once per fragment (i.e. roughly for every pixel that
    // will be rasterized), and in part determines the final color that will be sent
    // to the screen.
    const std::string fragmentShaderSource =
        "#version 410 core\n"
        "out vec4 color;\n"
        "void main()"
        "{\n"
        "  color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
        "}\n";

    return createShaderProgram(vertexShaderSource, fragmentShaderSource);
}

int main(void)
{
    App app(1280, 720);
    if (!app.init())
        return 1;

    getOpenGLVersionInfo();

    app.vertexSpec = genVertexSpec();
    app.shaderProgram = createGraphicsPipeline();

    mainLoop(app);

    return 0;
}
