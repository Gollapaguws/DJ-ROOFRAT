/*
** Copyright (c) 2008-2018 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

/* Khronos platform-specific types and definitions.
 *
 * The master copy of khrplatform.h is maintained in the Khronos SVN
 * repository at https://cvs.khronos.org/svn/repos/registry/trunk/api/KHR/khrplatform.h
 * The canonical copy of this file is in the official Khronos Headers repository.
 * Adopters are encouraged to check the Khronos official repository to receive
 * the most up-to-date version of this file. Please see the Policies and Procedures
 * at https://www.khronos.org/registry/speccreator for information on how to
 * contribute fixes to this file. A Khronos member or selected contributor may,
 * at their discretion, submit a pull request adding reversions of the file that
 * most of the Khronos members agree implement the best practices for backwards
 * compatibility.
 */

#ifndef __khrplatform_h_
#define __khrplatform_h_

/*
** STDINT:
** Make sure we have all the #defines for all the types we use in Khronos code.
** Types in khrplatform.h should only reflect the ANSI C header definitions we
** are mimicing. Therefore, this is just a convenience for developers, and must
** be on by default so the Khronos spec doesn't have to include extra defines
** for all developers.
*/
#include <stdint.h>
#if defined(__cplusplus) && !defined(KHRONOS_FORCE_INLINE_KEYWORD_ONLY)
#   define KHRONOS_INLINE inline
#elif !defined(__cplusplus) && !defined(inline)
#   define inline
#endif

typedef int32_t khronos_int32_t;
typedef uint32_t khronos_uint32_t;
typedef int64_t khronos_int64_t;
typedef uint64_t khronos_uint64_t;
typedef signed char khronos_int8_t;
typedef unsigned char khronos_uint8_t;
typedef signed short int khronos_int16_t;
typedef unsigned short int khronos_uint16_t;

/*
** FLOAT:
** Scalar floating point types
*/
typedef float khronos_float_t;

#if defined(_WIN64)
typedef signed __int64 khronos_intptr_t;
typedef unsigned __int64 khronos_uintptr_t;
#elif defined(__LP64__)
typedef signed long khronos_intptr_t;
typedef unsigned long khronos_uintptr_t;
#else
typedef signed int khronos_intptr_t;
typedef unsigned int khronos_uintptr_t;
#endif

#define KHRONOS_MAX_ENUM 0x7FFFFFFF

#endif /* __khrplatform_h_ */
