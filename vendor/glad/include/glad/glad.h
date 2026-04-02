/*
 * GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
 * This is a generated file for OpenGL 4.3 Core Profile
 * 
 * Generated glad loader for OpenGL 4.3 Core
 * Platform: Windows/Linux/macOS
 */

#ifndef __GLAD_H_
#define __GLAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <KHR/khrplatform.h>

#ifndef APIENTRY
#ifdef _WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

/* OpenGL core types */
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef khronos_int8_t GLbyte;
typedef khronos_uint8_t GLubyte;
typedef khronos_int16_t GLshort;
typedef khronos_uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef khronos_int32_t GLclampx;
typedef int GLsizei;
typedef khronos_float_t GLfloat;
typedef khronos_float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef khronos_intptr_t GLintptr;
typedef khronos_uintptr_t GLuintptr;
typedef khronos_int64_t GLint64;
typedef khronos_uint64_t GLuint64;
typedef ptrdiff_t GLsizeiptr;
typedef char GLchar;

/* OpenGL 4.3 Core enums */
#define GL_VERSION 0x1F02
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C

#define GL_TRUE 1
#define GL_FALSE 0

/* Shader types */
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_TESS_CONTROL_SHADER 0x8E88
#define GL_TESS_EVALUATION_SHADER 0x8E87
#define GL_COMPUTE_SHADER 0x91B9

/* VAO and VBO */
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_SHADER_STORAGE_BUFFER 0x90D2

/* Shader compile status */
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84

/* Buffer usage */
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

/* Draw modes */
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006

/* Pixel format */
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_DOUBLE 0x140A

/* Texture properties */
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_3D 0x806F
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_MIRRORED_REPEAT 0x8370

/* Texture units */
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF

/* Capabilities */
#define GL_BLEND 0x0BE2
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71

/* Error codes */
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_OUT_OF_MEMORY 0x0505

/* ===== FUNCTION POINTER TYPEDEFS ===== */

/* Basic functions */
typedef void (APIENTRYP PFNGLCLEARPROC)(GLbitfield mask);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGPROC)(GLenum name);
typedef void (APIENTRYP PFNGLGETINTEGERVPROC)(GLenum pname, GLint *data);
typedef void (APIENTRYP PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLENABLEPROC)(GLenum cap);
typedef void (APIENTRYP PFNGLDISABLEPROC)(GLenum cap);

/* Shader functions */
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);

/* Uniform functions */
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* VAO functions */
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

/* VBO/Buffer functions */
typedef void (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

/* Draw functions */
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices);

/* Texture functions */
typedef void (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXPARAMETERFPROC)(GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXPARAMETERIVPROC)(GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERFVPROC)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIVPROC)(GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERFVPROC)(GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC)(GLenum target);

/* ===== FUNCTION POINTER DECLARATIONS ===== */

typedef struct {
    PFNGLCLEARPROC glClear;
    PFNGLCLEARCOLORPROC glClearColor;
    PFNGLGETSTRINGPROC glGetString;
    PFNGLGETINTEGERVPROC glGetIntegerv;
    PFNGLVIEWPORTPROC glViewport;
    PFNGLENABLEPROC glEnable;
    PFNGLDISABLEPROC glDisable;
    
    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLUSEPROGRAMPROC glUseProgram;
    
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    PFNGLUNIFORM1FPROC glUniform1f;
    PFNGLUNIFORM3FPROC glUniform3f;
    PFNGLUNIFORM4FPROC glUniform4f;
    PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
    
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLBUFFERSUBDATAPROC glBufferSubData;
    
    PFNGLDRAWARRAYSPROC glDrawArrays;
    PFNGLDRAWELEMENTSPROC glDrawElements;
    
    PFNGLGENTEXTURESPROC glGenTextures;
    PFNGLDELETETEXTURESPROC glDeleteTextures;
    PFNGLBINDTEXTUREPROC glBindTexture;
    PFNGLTEXIMAGE2DPROC glTexImage2D;
    PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
    PFNGLTEXPARAMETERIPROC glTexParameteri;
    PFNGLTEXPARAMETERFPROC glTexParameterf;
    PFNGLTEXPARAMETERIVPROC glTexParameteriv;
    PFNGLTEXPARAMETERFVPROC glTexParameterfv;
    PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv;
    PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv;
    PFNGLACTIVETEXTUREPROC glActiveTexture;
    PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
} GLADgl;

extern GLADgl g_GLAD;

/* Convenience pointers for compatibility */
#ifdef glClear
#undef glClear
#endif
#ifdef glClearColor
#undef glClearColor
#endif
#ifdef glGetString
#undef glGetString
#endif
#ifdef glGetIntegerv
#undef glGetIntegerv
#endif
#ifdef glViewport
#undef glViewport
#endif
#ifdef glEnable 
#undef glEnable
#endif
#ifdef glDisable
#undef glDisable
#endif
#ifdef glCreateShader
#undef glCreateShader
#endif
#ifdef glDeleteShader
#undef glDeleteShader
#endif
#ifdef glShaderSource
#undef glShaderSource
#endif
#ifdef glCompileShader
#undef glCompileShader
#endif
#ifdef glGetShaderiv
#undef glGetShaderiv
#endif
#ifdef glGetShaderInfoLog
#undef glGetShaderInfoLog
#endif
#ifdef glCreateProgram
#undef glCreateProgram
#endif
#ifdef glDeleteProgram
#undef glDeleteProgram
#endif
#ifdef glAttachShader
#undef glAttachShader
#endif
#ifdef glLinkProgram
#undef glLinkProgram
#endif
#ifdef glGetProgramiv
#undef glGetProgramiv
#endif
#ifdef glGetProgramInfoLog
#undef glGetProgramInfoLog
#endif
#ifdef glUseProgram
#undef glUseProgram
#endif
#ifdef glGenVertexArrays
#undef glGenVertexArrays
#endif
#ifdef glDeleteVertexArrays
#undef glDeleteVertexArrays
#endif
#ifdef glBindVertexArray
#undef glBindVertexArray
#endif
#ifdef glEnableVertexAttribArray
#undef glEnableVertexAttribArray
#endif
#ifdef glVertexAttribPointer
#undef glVertexAttribPointer
#endif
#ifdef glGenBuffers
#undef glGenBuffers
#endif
#ifdef glDeleteBuffers
#undef glDeleteBuffers
#endif
#ifdef glBindBuffer
#undef glBindBuffer
#endif
#ifdef glBufferData
#undef glBufferData
#endif
#ifdef glBufferSubData
#undef glBufferSubData
#endif
#ifdef glDrawArrays
#undef glDrawArrays
#endif
#ifdef glDrawElements
#undef glDrawElements
#endif
#ifdef glGenTextures
#undef glGenTextures
#endif
#ifdef glDeleteTextures
#undef glDeleteTextures
#endif
#ifdef glBindTexture
#undef glBindTexture
#endif
#ifdef glTexImage2D
#undef glTexImage2D
#endif
#ifdef glTexSubImage2D
#undef glTexSubImage2D
#endif
#ifdef glTexParameteri
#undef glTexParameteri
#endif
#ifdef glTexParameterf
#undef glTexParameterf
#endif
#ifdef glTexParameteriv
#undef glTexParameteriv
#endif
#ifdef glTexParameterfv
#undef glTexParameterfv
#endif
#ifdef glGetTexParameteriv
#undef glGetTexParameteriv
#endif
#ifdef glGetTexParameterfv
#undef glGetTexParameterfv
#endif
#ifdef glActiveTexture
#undef glActiveTexture
#endif
#ifdef glGenerateMipmap
#undef glGenerateMipmap
#endif

extern PFNGLCLEARPROC glClear;
extern PFNGLCLEARCOLORPROC glClearColor;
extern PFNGLGETSTRINGPROC glGetString;
extern PFNGLGETINTEGERVPROC glGetIntegerv;
extern PFNGLVIEWPORTPROC glViewport;
extern PFNGLENABLEPROC glEnable;
extern PFNGLDISABLEPROC glDisable;

extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC glUseProgram;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;

extern PFNGLDRAWARRAYSPROC glDrawArrays;
extern PFNGLDRAWELEMENTSPROC glDrawElements;

/* ===== GLAD LOADER FUNCTION ===== */

typedef void* (*GLADloadproc)(const char *name);
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif

#endif  // __GLAD_H_
