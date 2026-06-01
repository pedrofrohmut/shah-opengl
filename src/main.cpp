#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>

#ifdef DEBUG_MODE
  #define DEBUG(expr) std::cout << expr
#else
  #define DEBUG(expr)
#endif

SDL_GLContext InitializeProgram(SDL_Window* window, int width, int height);
void MainLoop(SDL_Window* window);
void CleanUp(SDL_Window* window);
bool Input();
void PreDraw();
void Draw();
void GetOpenGLVersionInfo();

int main(void)
{
    const uint32_t screenHeight = 480;
    const uint32_t screenWidth  = 640;
    SDL_Window* window = nullptr;

    SDL_GLContext glContext = InitializeProgram(window, screenWidth, screenHeight);

    (void)glContext; // TODO: For compile error only. Remove it

    MainLoop(window);

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

    const uint32_t win_flags = SDL_WINDOW_OPENGL;
    window = SDL_CreateWindow("My OpenGL App", 0, 0, width, height, win_flags);
    if (window == NULL)
    {
        std::cout << "[ERROR] SDL2 could not create a window.\n";
        exit(1);
    }
    DEBUG("[Success] Window is ready to go.\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

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

void PreDraw() {}
void Draw() {}

void MainLoop(SDL_Window* window)
{
    bool shouldClose = false;
    while (!shouldClose)
    {
        shouldClose = Input();
        PreDraw();
        Draw();

        SDL_GL_SwapWindow(window);
        SDL_Delay(16); // 16 ms for ~60fps
    }
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
