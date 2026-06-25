// Third part deps
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// Cpp STD
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#include "macros.h"

static void clearAllGlErrors()
{
    while (glGetError() != GL_NO_ERROR) {}
}

static std::string getGlErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    default:
        return "UNKNOWN_GL_ERROR";
    }
}

static bool checkGlErrorStatus(const char* function, int line)
{
    bool hasErrors = false;
    while (GLenum error = glGetError())
    {
        println_(
          "--------------------------------------------------------------------------------\n" <<
          "OpenGL Error: '" << getGlErrorString(error) <<
          "'.\nLine: "      << line <<
          ".\nFunction: "   << function <<
          ".\n--------------------------------------------------------------------------------"
        );
        hasErrors = true;
    }
    return hasErrors;
}

#ifdef DEBUG_MODE
    #define glCheck_(x) clearAllGlErrors(); x; checkGlErrorStatus(#x, __LINE__);
#else
    #define glCheck_(x) x;
#endif

struct FpsState
{
    uint64_t previousFrame;
    uint32_t counter;
    float acc;
};

struct VertexSpec
{
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
    GLuint indexBufferObject;
};

struct AppContext
{
    const uint32_t screenWidth;
    const uint32_t screenHeight;
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    VertexSpec vertexSpec = {};
    GLuint shaderProgram = 0;
    float uOffset = 0.0f;

    AppContext(uint32_t width, uint32_t height)
        : screenWidth(width), screenHeight(height) {}

    ~AppContext()
    {
        if (glContext) SDL_GL_DeleteContext(glContext);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        debug_println_("[Success] AppContext exits with no errors.");
    }

    void quit(uint32_t errorCode)
    {
        if (glContext) SDL_GL_DeleteContext(glContext);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        if (errorCode == 0)
            debug_println_("[Success] Quiting the app.");
        else
            debug_println_("[ERROR] Something bad happend. Code: " << errorCode << ". Quiting the app.");
        exit(errorCode);
    }

    bool init()
    {
        // Init SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            std::cout << "[ERROR] SDL2 could not initialize video subsystem.\n";
            return false;
        }
        debug_println_("[Success] SDL initialized video subsystem.");

        // Set opengl version 4.1
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        // Disable deprecated functions of opengl
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // Smoother transitions
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        SDL_DisplayMode mode;
        const bool isDisplayModeSuccess = SDL_GetDesktopDisplayMode(0, &mode);
        if (isDisplayModeSuccess != 0) // Dont need to crash on error. just use some defaults
            println_("[ERROR] SDL2 could not get display information");

        // Get x and y of the display to center the window at start
        // mode.w is host width and mode.h is host height
        const uint32_t windowX = isDisplayModeSuccess != 0 ? 0 : (mode.w / 2) - (screenWidth / 2);
        const uint32_t windowY = isDisplayModeSuccess != 0 ? 0 : (mode.h / 2) - (screenHeight / 2);

        // Create SDL Window
        const uint32_t winFlags = SDL_WINDOW_OPENGL;
        window = SDL_CreateWindow("My OpenGL App", windowX, windowY, screenWidth, screenHeight, winFlags);
        if (window == nullptr)
        {
            std::cout << "[ERROR] SDL2 could not create a window.\n";
            return false;
        }
        debug_println_("[Success] Window is ready to go.");

        // Create OpenGL Context
        glContext = SDL_GL_CreateContext(window);
        if (glContext == nullptr)
        {
            std::cout << "[ERROR] SDL2 could not create OpenGL context.\n";
            return false;
        }
        debug_println_("[Success] OpenGl context created.");

        // Init glad
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
        {
            std::cout << "[ERROR] Glad failed to initialize\n";
            return false;
        }
        debug_println_("[Success] Glad loaded. OpenGL functions available.");

        return true;
    }
};

std::string loadShaderAsString(const std::string& fileName)
{
    std::ifstream sourceFile(fileName.c_str());

    if (!sourceFile.is_open())
        return "";

    std::string loadedShader = "";
    std::string line = "";
    while (std::getline(sourceFile, line))
        loadedShader += line + '\n'; // Adding \n to be sure and is cheap

    sourceFile.close();

    return loadedShader;
}

/**
 * processInput called in the main loop to handle sdl events queue
 * @return boolean to signal if app should close
 */
bool processInput(AppContext& app)
{
    SDL_Event event;

    // Handle all events in the queue until the queue is empty if no early return
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT: // handles OS asks for window close
            println_("Goodbye!");
            return true;
        }
    }

    // Retrieve keyboard state
    const uint8_t* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_UP])
    {
        app.uOffset += 0.01f;
        // printf_("app.uOffset = {}\n", app.uOffset);
    }
    if (state[SDL_SCANCODE_DOWN])
    {
        app.uOffset -= 0.01f;
        // printf_("app.uOffset = {}\n", app.uOffset);
    }

    return false;
}

/**
 * preDraw called in the main loop to setup state before starting drawing
 * @return void
 */
void preDraw(AppContext& app)
{
    // Disable stuff not needed for a 2D app
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, app.screenWidth, app.screenHeight); // Position and Size of opengl drawing
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Paint the background
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear buffers to be paint

    glUseProgram(app.shaderProgram); // Select current program to be used
    const GLchar* uniformName = "u_offset";
    GLint location = glGetUniformLocation(app.shaderProgram, uniformName); // gets uniform location to use
    if (location < 0) // negative values in case of not found
    {
        printf_("[ERROR] Could not find uniform location for '{}'\n", uniformName);
        app.quit(1);
    }
    glUniform1f(location, app.uOffset); // Pass the cpu value to the gpu
}

/**
 * draw called in the main loop to draw in the screen
 * @return void
 */
void draw(AppContext& app)
{
    // Select VAO and VBO for drawing
    // glBindVertexArray(app.vertexSpec.vertexArrayObject);
    // glBindBuffer(GL_ARRAY_BUFFER, app.vertexSpec.vertexBufferObject); // Without IBO

    // Select VAO and IBO
    glCheck_(glBindVertexArray(app.vertexSpec.vertexArrayObject));
    glCheck_(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app.vertexSpec.indexBufferObject));

    // Draw a triangle with the selected VAO and VBO
    // glDrawArrays(GL_TRIANGLES, 0 , 6);

    // Draw triangles with the VAO and IBO selected
    glCheck_(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    // Stop using current shaderProgram.
    // Obs: Not necessary if using only 1 graphics pipeline.
    glCheck_(glUseProgram(0));
}

void showFps(FpsState& state)
{
    const uint64_t thisFrame = SDL_GetTicks64();
    const uint64_t deltaTime = thisFrame - state.previousFrame;

    state.acc += deltaTime;
    state.counter += 1;
    if (state.acc >= 1000)
    {
        println_("fps: " << state.counter);
        state.acc = 0;
        state.counter = 0;
    }

    state.previousFrame = thisFrame;
}

void mainLoop(AppContext& app)
{
    bool shouldClose = false;
    FpsState fpsState = { .previousFrame = SDL_GetTicks64(), .counter = 0, .acc = 0 };

    while (!shouldClose)
    {
        shouldClose = processInput(app);
        preDraw(app);
        draw(app);
        SDL_GL_SwapWindow(app.window); // Updates the screen
        showFps(fpsState);
    }
}

void getOpenGLVersionInfo()
{
    println_("Vendor: "           << glGetString(GL_VENDOR));
    println_("Renderer: "         << glGetString(GL_RENDERER));
    println_("Version: "          << glGetString(GL_VERSION));
    println_("Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION));
}

VertexSpec setupVertexSpec()
{
    // VertexData: stores the positions + colors of the triangles in the main memory. Later this
    // will be transfered to the gpu by a VBO.

    // Winding order: the order that the points are store. Could be CCW or CW, it does not matter
    // with one you take but all triangle should be in the same winding order. This is important
    // to know what is the front and back faces of the triangles.
    const std::vector<GLfloat> vertexData
    {
        // 0 - Vertex
         -0.5f, -0.5f,  0.0f, // Bottom-left vertex
          1.0f,  0.0f,  0.0f, // Color red

        // 1 - Vertex
          0.5f, -0.5f,  0.0f, // Bottom-right vertex
          0.0f,  1.0f,  0.0f, // Color green

        // 2 - Vertex
         -0.5f,  0.5f,  0.0f, // Top-left vertex
          0.0f,  0.0f,  1.0f, // Color blue

        // 3 - Vertex
          0.5f,  0.5f,  0.0f, // Top-right vertex
          1.0f,  0.0f,  0.0f, // Color red
    };

    // Setup for the GPU

    // VAO - Vertex Array Object
    // We can think of VAO as a "wrapper around" all of the vertex buffer objects, in the
    // sense that it encapsulates all VBO state that we are setting up.
    // Thus, it is important that we glBindVertexArray (select the VAO to use) before our
    // VBO operations.
    GLuint vertexArrayObject = 0;
    glGenVertexArrays(1, &vertexArrayObject);
    // We bind (select) the VAO we want to work with.
    glBindVertexArray(vertexArrayObject);

    // VAO defines how the gpu should understand the data and how to send it to the gpu
    // VBO is the buffer of data that we use in the gpu.

    // VBO - Vertex Buffer Object
    GLuint vertexBufferObject = 0;

    // Create a new buffer and set the id to &vertexBufferObject
    glGenBuffers(1, &vertexBufferObject);
    // Bind or select the buffer that we want to work with at the moment
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    // Here we populate the buffer (VBO) with the vertexPositions we defined before
    // That means data from ram to vram
    glBufferData(GL_ARRAY_BUFFER,                      // type of buffer
                 vertexData.size() * sizeof(GLfloat),  // size of data
                 vertexData.data(),                    // raw pointer to ram data (stack or heap)
                 GL_STATIC_DRAW);                      // expected usage


    // Index Buffer Object (IBO or EBO)
    // This is used to store the array of indicies that we want to draw from.
    GLuint indexBufferObject = 0;
    // Setup the Index Buffer Object (IBO i.e. EBO)
    glGenBuffers(1, &indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    const std::vector<GLuint> indexBufferData
    {
        2, 0, 1, // first triangle
        2, 1, 3, // second triangle
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,                  // target
                 indexBufferData.size() * sizeof(GLuint),  // size of data
                 indexBufferData.data(),                   // raw pointer to data
                 GL_STATIC_DRAW);                          // expected usage

    // For a given VAO we need to tell how the data in the buffer will be used
    glEnableVertexAttribArray(0); // VAO for positions
    glVertexAttribPointer(0, // index: index defined by glEnableVertexAttribArray
                          3, // size: what is the number of components on our data chunks. in our case
                             // is 3 chunk = vertex and components is the x, y, z coords
                          GL_FLOAT, // type: type of the data
                          GL_FALSE, // normalized: is the data normalized?
                          sizeof(GLfloat) * 6, // stride: the size of the chunk, here is position + colors
                          (GLvoid*) 0); // pointer: in case there is a offset before the data to be used

    // Linking the attributes of the VAO
    glEnableVertexAttribArray(1); // VAO for colors
    glVertexAttribPointer(1,                          // index
                          3,                          // size: r, g, b
                          GL_FLOAT,                   // type
                          GL_FALSE,                   // normalized?
                          sizeof(GLfloat) * 6,        // stride
                          (GLvoid*) (sizeof(GLfloat) * 3)); // pointer

    // Clean up -Unbind the current VAO
    glBindVertexArray(0);

    // Disables/Closes any vertex attrib opened
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    return { vertexArrayObject, vertexBufferObject, indexBufferObject };
}

/**
 * compileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
 * @param shaderType: tells open gl what is the type of shader that is going to be compiled
 * @param shaderSource: the source code of the shader to be compiled as a string
 * @return the id of the shader object used to compile the shaderSource
 */
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

    // Setup source with the object and try to compile
    const char* src = shaderSource.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    // Check up compilation result
    int compilationResult;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compilationResult);

    if (compilationResult == GL_FALSE)
    {
        // Steps to get error messages from OpenGL
        int length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        char* errorMessages = new char[length]; // Alloc memory for errorMessages
        glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

        // Error logging
        switch (shaderType)
        {
        case GL_VERTEX_SHADER: {
            std::cout << "[ERROR] GL_VERTEX_SHADER compilation failed.\n" << errorMessages << '\n';
        } break;
        case GL_FRAGMENT_SHADER: {
            std::cout << "[ERROR] GL_FRAGMENT_SHADER compilation failed.\n" << errorMessages << '\n';
        } break;
        }

        delete[] errorMessages;
        glDeleteShader(shaderObject); // Delete broken shader
        return 0;
    }

    return shaderObject;
}

/**
 * Creates a graphics program object (i.e. graphics pipeline) with a vertex shader
 * and a fragment shader
 * @param vertexShaderSource vertex source code as a string
 * @param fragmentShaderSource fragment source code as a string
 * @return id of the program object created
 */
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
    std::string vertexShaderSource = loadShaderAsString("./shaders/vertex.glsl");

    // Fragment Shader executes once per fragment (i.e. roughly for every pixel that
    // will be rasterized), and in part determines the final color that will be sent
    // to the screen.
    std::string fragmentShaderSource = loadShaderAsString("./shaders/fragment.glsl");

    return createShaderProgram(vertexShaderSource, fragmentShaderSource);
}

int main(void)
{
    // Setup screen width and height.
    AppContext app(640, 480);

    // Create Init SDL, create Window, create OpenGL Context, Init Glad, return true if all ok.
    if (!app.init())
    {
        app.quit(1);
    }

    getOpenGLVersionInfo();

    // Setup Geometry: setup vertex in the cpu and transfer it to the gpu.
    app.vertexSpec = setupVertexSpec();

    // Graphics Pipeline: https://wikis.khronos.org/opengl/Rendering_Pipeline_Overview
    // 1.  VertexSpecification
    // 2. *VertexShader
    // 3. *Tessellation
    // 4. *Geometry Shader
    // 5.  Vertex Post-Processing
    // 6.  Primitive Assembly
    // 7.  Rasterization
    // 8. *Fragment Shader
    // 9.  Per-Sample Operations
    // * are the ones that can be defined by the user.

    // Create Graphics Pipeline: setup vertex and fragment shaders.
    // Load them from source, compile, link then together and creating the pipeline
    app.shaderProgram = createGraphicsPipeline();

    // Main Loop: Use the graphics pipeline (shaderProgram) to draw the triangle in
    // the screen
    mainLoop(app);

    // Clean Up: it happens in the app context destructor ~AppContext()

    return 0;
}

// Including other source files to make a single compile unit
#include "./glad.c"
