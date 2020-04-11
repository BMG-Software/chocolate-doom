#include "i_glvideo.h"

#include "glew.h"

#include "SDL.h"
#include "SDL_opengl.h"

#include "m_argv.h"
#include "d_event.h"
#include "i_input.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <stdio.h>

static SDL_Window *window;
static SDL_GLContext *gl_context;

static const char* window_title = "OpenGL DOOM!";

static GLuint shader_program;

static GLint attribute_coord_3d;

#define SHADERBUFFERSIZE 2048

int fullscreen = false;

char *window_position = "center";

int video_display = 0;

boolean screensaver_mode = false;

boolean screenvisible = true;

// Joystick/gamepad hysteresis
unsigned int joywait = 0;


// Check the display bounds of the display referred to by 'video_display' and
// set x and y to a location that places the window in the center of that
// display.
static void CenterWindow(int *x, int *y, int w, int h)
{
    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(video_display, &bounds) < 0)
    {
        fprintf(stderr, "CenterWindow: Failed to read display bounds "
            "for display #%d!\n", video_display);
        return;
    }

    *x = bounds.x + SDL_max((bounds.w - w) / 2, 0);
    *y = bounds.y + SDL_max((bounds.h - h) / 2, 0);
}

void I_GetWindowPosition(int *x, int *y, int w, int h)
{
    // Check that video_display corresponds to a display that really exists,
    // and if it doesn't, reset it.
    if (video_display < 0 || video_display >= SDL_GetNumVideoDisplays())
    {
        fprintf(stderr,
            "I_GetWindowPosition: We were configured to run on display #%d, "
            "but it no longer exists (max %d). Moving to display 0.\n",
            video_display, SDL_GetNumVideoDisplays() - 1);
        video_display = 0;
    }

    // in fullscreen mode, the window "position" still matters, because
    // we use it to control which display we run fullscreen on.

    if (fullscreen)
    {
        CenterWindow(x, y, w, h);
        return;
    }

    // in windowed mode, the desired window position can be specified
    // in the configuration file.

    if (window_position == NULL || !strcmp(window_position, ""))
    {
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
    else if (!strcmp(window_position, "center"))
    {
        // Note: SDL has a SDL_WINDOWPOS_CENTER, but this is useless for our
        // purposes, since we also want to control which display we appear on.
        // So we have to do this ourselves.
        CenterWindow(x, y, w, h);
    }
    else if (sscanf(window_position, "%i,%i", x, y) != 2)
    {
        // invalid format: revert to default
        fprintf(stderr, "I_GetWindowPosition: invalid window_position setting\n");
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
}

GLuint GetShaderProgram(const char *vs_filename, const char *fs_filename)
{
    char vs_shader_source[SHADERBUFFERSIZE];
    if (FileReader(vs_filename, vs_shader_source, SHADERBUFFERSIZE) == -1)
    {
        // error out
    }
    GLuint vertexShader = LoadShader(vs_shader_source, GL_VERTEX_SHADER);
    char fs_shader_source[SHADERBUFFERSIZE];
    if (FileReader(fs_filename, fs_shader_source, SHADERBUFFERSIZE) == -1)
    {
        // error out
    }
    GLuint fragmentShader = LoadShader(fs_shader_source, GL_FRAGMENT_SHADER);

    GLint success, logLength;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = (GLchar*)malloc(logLength);
        glGetProgramInfoLog(program, logLength, NULL, log);
        free(log);
    }
    if (!success)
    {
        // we error out
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint LoadShader(const char* shader_source, GLenum shader_type)
{
    GLchar *log = NULL;
    GLint success, logLength;
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    log = (GLchar*)malloc(logLength);
    if (logLength > 0)
    {
        glGetShaderInfoLog(shader, logLength, NULL, log);
    }
    if (!success)
    {
        // we error out
    }
    free(log);
    return shader;
}

int FileReader(const char *filename, char *buffer, int buffer_size)
{
    FILE *file;
    file = fopen(filename, "r");
    int index = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
    {
        buffer[index] = (char)c;
        index++;
        if (index > buffer_size)
        {
            return -1; // errored out, shader file too long
        }
    }
    buffer[index] = '\0';
    return 1;
}

static boolean ToggleFullScreenKeyShortcut(SDL_Keysym *sym)
{
    Uint16 flags = (KMOD_LALT | KMOD_RALT);
#if defined(__MACOSX__)
    flags |= (KMOD_LGUI | KMOD_RGUI);
#endif
    return (sym->scancode == SDL_SCANCODE_RETURN ||
        sym->scancode == SDL_SCANCODE_KP_ENTER) && (sym->mod & flags) != 0;
}

static void I_ToggleFullScreen(void)
{
    /*
    unsigned int flags = 0;

    // TODO: Consider implementing fullscreen toggle for SDL_WINDOW_FULLSCREEN
    // (mode-changing) setup. This is hard because we have to shut down and
    // restart again.
    if (fullscreen_width != 0 || fullscreen_height != 0)
    {
        return;
    }

    fullscreen = !fullscreen;

    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_SetWindowFullscreen(screen, flags);

    if (!fullscreen)
    {
        AdjustWindowSize();
        SDL_SetWindowSize(screen, window_width, window_height);
    }*/
}


static void HandleWindowEvent(SDL_WindowEvent *event)
{
    int i;

    switch (event->event)
    {
#if 0 // SDL2-TODO
    case SDL_ACTIVEEVENT:
        // need to update our focus state
        UpdateFocus();
        break;
#endif
        /*
    case SDL_WINDOWEVENT_EXPOSED:
        palette_to_set = true;
        break;

    case SDL_WINDOWEVENT_RESIZED:
        need_resize = true;
        last_resize_time = SDL_GetTicks();
        break;
        */
        // Don't render the screen when the window is minimized:

    case SDL_WINDOWEVENT_MINIMIZED:
        screenvisible = false;
        break;

    case SDL_WINDOWEVENT_MAXIMIZED:
    case SDL_WINDOWEVENT_RESTORED:
        screenvisible = true;
        break;

        // Update the value of window_focused when we get a focus event
        //
        // We try to make ourselves be well-behaved: the grab on the mouse
        // is removed if we lose focus (such as a popup window appearing),
        // and we dont move the mouse around if we aren't focused either.
        /*
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        window_focused = true;
        break;

    case SDL_WINDOWEVENT_FOCUS_LOST:
        window_focused = false;
        break;
        */
        // We want to save the user's preferred monitor to use for running the
        // game, so that next time we're run we start on the same display. So
        // every time the window is moved, find which display we're now on and
        // update the video_display config variable.

    case SDL_WINDOWEVENT_MOVED:
        i = SDL_GetWindowDisplayIndex(window);
        if (i >= 0)
        {
            video_display = i;
        }
        break;

    default:
        break;
    }
}

void I_GLGetEvent(void)
{
    extern void I_HandleKeyboardEvent(SDL_Event *sdlevent);
    extern void I_HandleMouseEvent(SDL_Event *sdlevent);
    SDL_Event sdlevent;

    SDL_PumpEvents();

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
        case SDL_KEYDOWN:
            if (ToggleFullScreenKeyShortcut(&sdlevent.key.keysym))
            {
                I_ToggleFullScreen();
                break;
            }
            // deliberate fall-though

        case SDL_KEYUP:
            I_HandleKeyboardEvent(&sdlevent);
            break;
            
            /*
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
            if (usemouse && !nomouse && window_focused)
            {
                I_HandleMouseEvent(&sdlevent);
            }
            break;
            */

        case SDL_QUIT:
            if (screensaver_mode)
            {
                I_Quit();
            }
            else
            {
                event_t event;
                event.type = ev_quit;
                D_PostEvent(&event);
            }
            break;

        case SDL_WINDOWEVENT:
            if (sdlevent.window.windowID == SDL_GetWindowID(window))
            {
                HandleWindowEvent(&sdlevent.window);
            }
            break;

        default:
            break;
        }
    }
}

void I_GLInitGraphics()
{

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return;

    int x, y;
    int w = SCREENWIDTH, h = SCREENHEIGHT; // Set to defaults

    int i = M_CheckParmWithArgs("-width", 1);

    if (i > 0)
    {
        w = atoi(myargv[i + 1]);
        // fullscreen = false; 
    }

    i = M_CheckParmWithArgs("-height", 1);

    if (i > 0)
    {
        h = atoi(myargv[i + 1]);
        // fullscreen = false;
    }

    I_GetWindowPosition(&x, &y, w, h);

    window = SDL_CreateWindow(window_title, x, y, w, h, SDL_WINDOW_OPENGL);
    gl_context = SDL_GL_CreateContext(window);

    glewInit();
    
    // TODO: Bin off the magic values that are here
    shader_program = GetShaderProgram("shaders\\vert.glsl", "shaders\\frag.glsl");

    // TODO: Copy across the shaders
    attribute_coord_3d = glGetAttribLocation(shader_program, "coord3d");

    glViewport(0, 0, w, h);
    glClearColor(0.9f, 0.9f, 0.9f, 0.f);

}

void I_GLShutdownGraphics()
{
    // TODO: glDeleteBuffers( etc.

    glDeleteProgram(shader_program);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}

void I_GLUpdateNoBlit()
{
}

void I_GLFinishUpdate()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_program);

    SDL_GL_SwapWindow(window);
}

void I_GLStartFrame()
{
}

void I_GLStartTic()
{
    I_GLGetEvent();
}

void I_GLGetWindowPosition(int * x, int * y, int w, int h)
{
}
