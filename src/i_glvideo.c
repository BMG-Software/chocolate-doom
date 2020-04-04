#include "i_glvideo.h"

#include "glew.h"

#include "SDL.h"
#include "SDL_opengl.h"

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

void I_GLInitGraphics()
{

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return;

    int x, y, w = SCREENWIDTH, h = SCREENHEIGHT;
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
}

void I_GLGetWindowPosition(int * x, int * y, int w, int h)
{
}
