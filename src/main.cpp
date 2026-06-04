#include <SDL2/SDL.h>

#include <iostream>
#include <cstdint>

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

        const uint32_t winFlags = SDL_WINDOW_OPENGL;
        window = SDL_CreateWindow("My OpenGL App", 0, 0, screenWidth, screenHeight, winFlags);
        if (window == nullptr)
        {
            std::cout << "[ERROR] SDL2 could not create a window.\n";
            SDL_Quit();
            return false;
        }
        DEBUG("[Success] Window is ready to go.\n");

        glContext = SDL_GL_CreateContext(window);
        if (glContext == nullptr)
        {
            std::cout << "[ERROR] SDL2 could not create OpenGL context.\n";
            SDL_DestroyWindow(window);
            SDL_Quit();
            return false;
        }
        DEBUG("[Success] OpenGl context created.\n");

        return true;
    }
};

bool pollEvents()
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

void mainLoop(App& app)
{
    bool shouldClose = false;

    while (!shouldClose)
    {
        shouldClose = pollEvents();

        SDL_GL_SwapWindow(app.window);

        SDL_Delay(16); // 16ms ~63fps
    }
}

int main(void)
{
    App app(640, 480);

    if (!app.init()) return 1;

    mainLoop(app);

    return 0;
}
