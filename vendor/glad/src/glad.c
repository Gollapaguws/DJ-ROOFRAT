#include <glad/glad.h>
#include <stddef.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Function pointers that will be loaded */
PFNGLGETSTRINGPROC glGetString = NULL;
PFNGLGETINTEGERVPROC glGetIntegerv = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;

/* Helper to load a single function pointer */
static void* _glad_load_function(GLADloadproc load, const char* name) {
    void* result = load(name);
    return result;
}

/* Main loader function - returns GL version (e.g., 430 for GL 4.3) */
int gladLoadGLLoader(GLADloadproc load) {
    if (load == NULL) {
        return 0;
    }

    /* Load critical functions */
    glGetString = (PFNGLGETSTRINGPROC)_glad_load_function(load, "glGetString");
    if (glGetString == NULL) {
        return 0;
    }

    glGetIntegerv = (PFNGLGETINTEGERVPROC)_glad_load_function(load, "glGetIntegerv");
    if (glGetIntegerv == NULL) {
        return 0;
    }

    /* Load shader compilation functions */
    glCreateShader = (PFNGLCREATESHADERPROC)_glad_load_function(load, "glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)_glad_load_function(load, "glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)_glad_load_function(load, "glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)_glad_load_function(load, "glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)_glad_load_function(load, "glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)_glad_load_function(load, "glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)_glad_load_function(load, "glUseProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)_glad_load_function(load, "glDeleteShader");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)_glad_load_function(load, "glGetProgramiv");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)_glad_load_function(load, "glGetShaderiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)_glad_load_function(load, "glGetProgramInfoLog");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)_glad_load_function(load, "glGetShaderInfoLog");

    /* Check OpenGL version (required: 4.3+) */
    int major = 0, minor = 0;
    glGetIntegerv(0x821B, &major);  /* GL_MAJOR_VERSION */
    glGetIntegerv(0x821C, &minor);  /* GL_MINOR_VERSION */

    if (major < 4 || (major == 4 && minor < 3)) {
        /* Return version as GLAD does: e.g., 430 for OpenGL 4.3 */
        return 0;
    }

    /* Return a non-zero value to indicate success (version encoded) */
    return major * 100 + minor * 10;
}
