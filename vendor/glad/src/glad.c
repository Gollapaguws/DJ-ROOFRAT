#include <glad/glad.h>
#include <stddef.h>

/* Define the global GLAD structure */
GLADgl g_GLAD = {0};

/* Define external pointers that reference the struct members */
PFNGLCLEARPROC glClear = NULL;
PFNGLCLEARCOLORPROC glClearColor = NULL;
PFNGLGETSTRINGPROC glGetString = NULL;
PFNGLGETINTEGERVPROC glGetIntegerv = NULL;
PFNGLVIEWPORTPROC glViewport = NULL;
PFNGLENABLEPROC glEnable = NULL;
PFNGLDISABLEPROC glDisable = NULL;

PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;

PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;

PFNGLDRAWARRAYSPROC glDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC glDrawElements = NULL;

int gladLoadGLLoader(GLADloadproc load) {
    if (load == NULL) {
        return 0;
    }

    /* Load all function pointers into the struct */
    g_GLAD.glClear = (PFNGLCLEARPROC)load("glClear");
    g_GLAD.glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    g_GLAD.glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
    g_GLAD.glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");
    g_GLAD.glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    g_GLAD.glEnable = (PFNGLENABLEPROC)load("glEnable");
    g_GLAD.glDisable = (PFNGLDISABLEPROC)load("glDisable");

    g_GLAD.glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    g_GLAD.glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    g_GLAD.glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    g_GLAD.glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    g_GLAD.glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
    g_GLAD.glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
    g_GLAD.glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    g_GLAD.glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
    g_GLAD.glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    g_GLAD.glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    g_GLAD.glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
    g_GLAD.glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
    g_GLAD.glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");

    g_GLAD.glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
    g_GLAD.glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
    g_GLAD.glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    g_GLAD.glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    g_GLAD.glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");

    g_GLAD.glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
    g_GLAD.glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
    g_GLAD.glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    g_GLAD.glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    g_GLAD.glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");

    g_GLAD.glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    g_GLAD.glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");

    /* Copy loaded pointers to external variables for API access */
    glClear = g_GLAD.glClear;
    glClearColor = g_GLAD.glClearColor;
    glGetString = g_GLAD.glGetString;
    glGetIntegerv = g_GLAD.glGetIntegerv;
    glViewport = g_GLAD.glViewport;
    glEnable = g_GLAD.glEnable;
    glDisable = g_GLAD.glDisable;

    glCreateShader = g_GLAD.glCreateShader;
    glDeleteShader = g_GLAD.glDeleteShader;
    glShaderSource = g_GLAD.glShaderSource;
    glCompileShader = g_GLAD.glCompileShader;
    glGetShaderiv = g_GLAD.glGetShaderiv;
    glGetShaderInfoLog = g_GLAD.glGetShaderInfoLog;
    glCreateProgram = g_GLAD.glCreateProgram;
    glDeleteProgram = g_GLAD.glDeleteProgram;
    glAttachShader = g_GLAD.glAttachShader;
    glLinkProgram = g_GLAD.glLinkProgram;
    glGetProgramiv = g_GLAD.glGetProgramiv;
    glGetProgramInfoLog = g_GLAD.glGetProgramInfoLog;
    glUseProgram = g_GLAD.glUseProgram;

    glGenVertexArrays = g_GLAD.glGenVertexArrays;
    glDeleteVertexArrays = g_GLAD.glDeleteVertexArrays;
    glBindVertexArray = g_GLAD.glBindVertexArray;
    glEnableVertexAttribArray = g_GLAD.glEnableVertexAttribArray;
    glVertexAttribPointer = g_GLAD.glVertexAttribPointer;

    glGenBuffers = g_GLAD.glGenBuffers;
    glDeleteBuffers = g_GLAD.glDeleteBuffers;
    glBindBuffer = g_GLAD.glBindBuffer;
    glBufferData = g_GLAD.glBufferData;
    glBufferSubData = g_GLAD.glBufferSubData;

    glDrawArrays = g_GLAD.glDrawArrays;
    glDrawElements = g_GLAD.glDrawElements;

    /* Verify critical functions loaded */
    if (!glCreateShader || !glGenVertexArrays || !glCreateProgram) {
        return 0;
    }

    return 430; /* OpenGL 4.3 */
}

