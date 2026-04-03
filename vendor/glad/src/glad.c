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
PFNGLGETERRORPROC glGetError = NULL;
PFNGLISENABLEDPROC glIsEnabled = NULL;
PFNGLGETFLOATVPROC glGetFloatv = NULL;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = NULL;
PFNGLDEPTHFUNCPROC glDepthFunc = NULL;

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

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORM3FVPROC glUniform3fv = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;

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
PFNGLMAPBUFFERPROC glMapBuffer = NULL;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange = NULL;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;

PFNGLDRAWARRAYSPROC glDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC glDrawElements = NULL;

PFNGLGENTEXTURESPROC glGenTextures = NULL;
PFNGLDELETETEXTURESPROC glDeleteTextures = NULL;
PFNGLBINDTEXTUREPROC glBindTexture = NULL;
PFNGLTEXIMAGE2DPROC glTexImage2D = NULL;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D = NULL;
PFNGLTEXPARAMETERIPROC glTexParameteri = NULL;
PFNGLTEXPARAMETERFPROC glTexParameterf = NULL;
PFNGLTEXPARAMETERIVPROC glTexParameteriv = NULL;
PFNGLTEXPARAMETERFVPROC glTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv = NULL;
PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;

/* FBO function pointers */
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = NULL;
PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = NULL;

/* Pixel transfer function pointers */
PFNGLREADPIXELSPROC glReadPixels = NULL;

/* Compute shader function pointers */
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = NULL;
PFNGLMEMORYBARRIERPROC glMemoryBarrier = NULL;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = NULL;

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
    g_GLAD.glGetError = (PFNGLGETERRORPROC)load("glGetError");
    g_GLAD.glIsEnabled = (PFNGLISENABLEDPROC)load("glIsEnabled");
    g_GLAD.glGetFloatv = (PFNGLGETFLOATVPROC)load("glGetFloatv");
    g_GLAD.glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)load("glGetVertexAttribiv");
    g_GLAD.glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");

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

    g_GLAD.glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
    g_GLAD.glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
    g_GLAD.glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
    g_GLAD.glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
    g_GLAD.glUniform3fv = (PFNGLUNIFORM3FVPROC)load("glUniform3fv");
    g_GLAD.glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
    g_GLAD.glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");

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
    g_GLAD.glMapBuffer = (PFNGLMAPBUFFERPROC)load("glMapBuffer");
    g_GLAD.glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)load("glMapBufferRange");
    g_GLAD.glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)load("glUnmapBuffer");

    g_GLAD.glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    g_GLAD.glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");

    g_GLAD.glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
    g_GLAD.glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
    g_GLAD.glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
    g_GLAD.glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
    g_GLAD.glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)load("glTexSubImage2D");
    g_GLAD.glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
    g_GLAD.glTexParameterf = (PFNGLTEXPARAMETERFPROC)load("glTexParameterf");
    g_GLAD.glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)load("glTexParameteriv");
    g_GLAD.glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)load("glTexParameterfv");
    g_GLAD.glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)load("glGetTexParameteriv");
    g_GLAD.glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)load("glGetTexParameterfv");
    g_GLAD.glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
    g_GLAD.glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");

    /* Load FBO functions */
    g_GLAD.glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
    g_GLAD.glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
    g_GLAD.glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
    g_GLAD.glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
    g_GLAD.glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
    g_GLAD.glDrawBuffers = (PFNGLDRAWBUFFERSPROC)load("glDrawBuffers");
    g_GLAD.glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load("glBlitFramebuffer");
    g_GLAD.glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetFramebufferAttachmentParameteriv");

    /* Load pixel transfer functions */
    g_GLAD.glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");

    /* Load compute shader functions */
    g_GLAD.glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)load("glDispatchCompute");
    g_GLAD.glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)load("glMemoryBarrier");
    g_GLAD.glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");

    /* Copy loaded pointers to external variables for API access */
    glClear = g_GLAD.glClear;
    glClearColor = g_GLAD.glClearColor;
    glGetString = g_GLAD.glGetString;
    glGetIntegerv = g_GLAD.glGetIntegerv;
    glViewport = g_GLAD.glViewport;
    glEnable = g_GLAD.glEnable;
    glDisable = g_GLAD.glDisable;
    glGetError = g_GLAD.glGetError;
    glIsEnabled = g_GLAD.glIsEnabled;
    glGetFloatv = g_GLAD.glGetFloatv;
    glGetVertexAttribiv = g_GLAD.glGetVertexAttribiv;
    glDepthFunc = g_GLAD.glDepthFunc;

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

    glGetUniformLocation = g_GLAD.glGetUniformLocation;
    glUniform1i = g_GLAD.glUniform1i;
    glUniform1f = g_GLAD.glUniform1f;
    glUniform3f = g_GLAD.glUniform3f;
    glUniform3fv = g_GLAD.glUniform3fv;
    glUniform4f = g_GLAD.glUniform4f;
    glUniformMatrix4fv = g_GLAD.glUniformMatrix4fv;

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
    glMapBuffer = g_GLAD.glMapBuffer;
    glMapBufferRange = g_GLAD.glMapBufferRange;
    glUnmapBuffer = g_GLAD.glUnmapBuffer;

    glDrawArrays = g_GLAD.glDrawArrays;
    glDrawElements = g_GLAD.glDrawElements;

    glGenTextures = g_GLAD.glGenTextures;
    glDeleteTextures = g_GLAD.glDeleteTextures;
    glBindTexture = g_GLAD.glBindTexture;
    glTexImage2D = g_GLAD.glTexImage2D;
    glTexSubImage2D = g_GLAD.glTexSubImage2D;
    glTexParameteri = g_GLAD.glTexParameteri;
    glTexParameterf = g_GLAD.glTexParameterf;
    glTexParameteriv = g_GLAD.glTexParameteriv;
    glTexParameterfv = g_GLAD.glTexParameterfv;
    glGetTexParameteriv = g_GLAD.glGetTexParameteriv;
    glGetTexParameterfv = g_GLAD.glGetTexParameterfv;
    glActiveTexture = g_GLAD.glActiveTexture;
    glGenerateMipmap = g_GLAD.glGenerateMipmap;

    /* Assign FBO pointers */
    glGenFramebuffers = g_GLAD.glGenFramebuffers;
    glDeleteFramebuffers = g_GLAD.glDeleteFramebuffers;
    glBindFramebuffer = g_GLAD.glBindFramebuffer;
    glFramebufferTexture2D = g_GLAD.glFramebufferTexture2D;
    glCheckFramebufferStatus = g_GLAD.glCheckFramebufferStatus;
    glDrawBuffers = g_GLAD.glDrawBuffers;
    glBlitFramebuffer = g_GLAD.glBlitFramebuffer;
    glGetFramebufferAttachmentParameteriv = g_GLAD.glGetFramebufferAttachmentParameteriv;

    /* Assign pixel transfer pointers */
    glReadPixels = g_GLAD.glReadPixels;

    /* Assign compute shader pointers */
    glDispatchCompute = g_GLAD.glDispatchCompute;
    glMemoryBarrier = g_GLAD.glMemoryBarrier;
    glBindBufferBase = g_GLAD.glBindBufferBase;

    /* Verify critical functions loaded */
    if (!glCreateShader || !glGenVertexArrays || !glCreateProgram) {
        return 0;
    }

    return 430; /* OpenGL 4.3 */
}

