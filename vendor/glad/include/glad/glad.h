#ifndef __GLAD_H_
#define __GLAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifndef GLAD_GL_CORE_PROFILE
#define GLAD_GL_CORE_PROFILE 1
#endif

#ifndef GLAD_NO_INLINE
#define GLAD_NO_INLINE
#endif

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

#ifndef GLAD_MALLOC
#define GLAD_MALLOC(x) malloc(x)
#endif

#ifndef GLAD_FREE
#define GLAD_FREE(x) free(x)
#endif

#ifdef _WIN32
#define GLAD_API_CALL
#else
#define GLAD_API_CALL
#endif

/* OpenGL enum definitions */
#define GL_VERSION 0x1F02
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C

#define GL_TRUE 1
#define GL_FALSE 0

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84

/* Extension loader callback type */
typedef void* (*GLADloadproc)(const char *name);

/* GLAD loader function - declared in glad.c */
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif

#endif  // __GLAD_H_
