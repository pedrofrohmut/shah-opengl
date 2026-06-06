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

struct AppContext
{
    const uint32_t screenWidth;
    const uint32_t screenHeight;
    SDL_Window* window;
    SDL_GLContext glContext;
    VertexSpec vertexSpec;
    GLuint shaderProgram;

    AppContext(uint32_t width, uint32_t height)
        : screenWidth(width),
          screenHeight(height),
          window(nullptr),
          glContext(nullptr),
          vertexSpec(0, 0),
          shaderProgram(0)
    {
    }

    ~AppContext()
    {
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        DEBUG("[Success] AppContext exits with no errors.\n");
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

/**
 * processInput called in the main loop to handle sdl events queue
 * @return boolean to signal if app should close
 */
bool processInput()
{
    SDL_Event event;

    // Handle all events in the queue until the queue is empty if no early return
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        // handles OS asks for window close
        case SDL_QUIT: {
            std::cout << "Goodbye!\n";
            return true;
        }
        }
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
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // Paint the background
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear buffers to be paint

    glUseProgram(app.shaderProgram); // Select current program to be used
}

/**
 * draw called in the main loop to draw in the screen
 * @return void
 */
void draw(AppContext& app)
{
    // Select VAO and VBO for drawing
    glBindVertexArray(app.vertexSpec.vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, app.vertexSpec.vertexBufferObject);

    // Draw a triangle with the selected VAO and VBO
    glDrawArrays(GL_TRIANGLES, 0 , 3);

    // Stop using current shaderProgram.
    // Obs: Not necessary if using only 1 graphics pipeline.
    glUseProgram(0);
}

void mainLoop(AppContext& app)
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

        // FrameTime - Time that takes to pass all stages of frame creation in int milliseconds
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

VertexSpec setupVertexSpec()
{
    // Geometry Data:
    // Stores x, y, z positions for the vertexes, It is initially store in the cpu + ram memory
    // in the vector. Later this will be stored in the gpu + vram, to do this we call
    // glBufferData that will store the information in a vertexBufferObject.
    const std::vector<GLfloat> vertexPositions{
        //  x,     y,    z,
        -0.8f, -0.8f, 0.0f, // vertex 1
         0.8f, -0.8f, 0.0f, // vertex 2
         0.0f,  0.8f, 0.0f, // vertex 3
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
    // VBO is the buffer of data
    // that we use in the gpu.

    // VBO - Vertex Buffer Object
    GLuint vertexBufferObject = 0;
    // Create a new buffer and set the id to &vertexBufferObject
    glGenBuffers(1, &vertexBufferObject);
    // Bind or select the buffer that we want to work with at the moment
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    // Here we populate the buffer (VBO) with the vertexPositions we defined before
    // That means data from ram to vram
    glBufferData(GL_ARRAY_BUFFER,                           // type of buffer
                 vertexPositions.size() * sizeof(GLfloat),  // size of data
                 vertexPositions.data(),                    // raw pointer to ram data (stack or heap)
                 GL_STATIC_DRAW);                           // expected usage

    // For a given VAO we need to tell how the data in the buffer will be used
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, // index: index of vertexAttrib to use
                          3, // size: what is the number of components on our data chunks. in our case
                             // is 3 chunk = vertex and components is the x, y, z coords
                          GL_FLOAT, // type: type of the data
                          GL_FALSE, // normalized: is the data normalized?
                          sizeof(GLfloat) * 3, // stride: is the size of the chunks
                          (void*) 0); // pointer: in case there is a offset before the data to be used

    // Clean up
    glBindVertexArray(0); // Unbind the current VAO
    glDisableVertexAttribArray(0); // Disables/Closes any vertex attrib opened

    VertexSpec spec(vertexArrayObject, vertexBufferObject);
    return spec;
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
    // Setup screen width and height.
    AppContext app(640, 480);

    // Create Init SDL, create Window, create OpenGL Context, Init Glad, return true if all ok.
    if (!app.init())
        return 1;

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
