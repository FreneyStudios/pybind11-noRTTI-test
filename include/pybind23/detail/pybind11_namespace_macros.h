// Copyright (c) 2016-2025 The Pybind Development Team.
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#pragma once

// PLEASE DO NOT ADD ANY INCLUDES HERE

// Define some generic pybind23 helper macros for warning management.
//
// Note that compiler-specific push/pop pairs are baked into the
// PYBIND23_NAMESPACE_BEGIN/PYBIND23_NAMESPACE_END pair of macros. Therefore manual
// PYBIND23_WARNING_PUSH/PYBIND23_WARNING_POP are usually only needed in `#include` sections.
//
// If you find you need to suppress a warning, please try to make the suppression as local as
// possible using these macros. Please also be sure to push/pop with the pybind23 macros. Please
// only use compiler specifics if you need to check specific versions, e.g. Apple Clang vs. vanilla
// Clang.
#if defined(_MSC_VER)
#    define PYBIND23_COMPILER_MSVC
#    define PYBIND23_PRAGMA(...) __pragma(__VA_ARGS__)
#    define PYBIND23_WARNING_PUSH PYBIND23_PRAGMA(warning(push))
#    define PYBIND23_WARNING_POP PYBIND23_PRAGMA(warning(pop))
#elif defined(__INTEL_COMPILER)
#    define PYBIND23_COMPILER_INTEL
#    define PYBIND23_PRAGMA(...) _Pragma(#__VA_ARGS__)
#    define PYBIND23_WARNING_PUSH PYBIND23_PRAGMA(warning push)
#    define PYBIND23_WARNING_POP PYBIND23_PRAGMA(warning pop)
#elif defined(__clang__)
#    define PYBIND23_COMPILER_CLANG
#    define PYBIND23_PRAGMA(...) _Pragma(#__VA_ARGS__)
#    define PYBIND23_WARNING_PUSH PYBIND23_PRAGMA(clang diagnostic push)
#    define PYBIND23_WARNING_POP PYBIND23_PRAGMA(clang diagnostic pop)
#elif defined(__GNUC__)
#    define PYBIND23_COMPILER_GCC
#    define PYBIND23_PRAGMA(...) _Pragma(#__VA_ARGS__)
#    define PYBIND23_WARNING_PUSH PYBIND23_PRAGMA(GCC diagnostic push)
#    define PYBIND23_WARNING_POP PYBIND23_PRAGMA(GCC diagnostic pop)
#endif

#ifdef PYBIND23_COMPILER_MSVC
#    define PYBIND23_WARNING_DISABLE_MSVC(name) PYBIND23_PRAGMA(warning(disable : name))
#else
#    define PYBIND23_WARNING_DISABLE_MSVC(name)
#endif

#ifdef PYBIND23_COMPILER_CLANG
#    define PYBIND23_WARNING_DISABLE_CLANG(name) PYBIND23_PRAGMA(clang diagnostic ignored name)
#else
#    define PYBIND23_WARNING_DISABLE_CLANG(name)
#endif

#ifdef PYBIND23_COMPILER_GCC
#    define PYBIND23_WARNING_DISABLE_GCC(name) PYBIND23_PRAGMA(GCC diagnostic ignored name)
#else
#    define PYBIND23_WARNING_DISABLE_GCC(name)
#endif

#ifdef PYBIND23_COMPILER_INTEL
#    define PYBIND23_WARNING_DISABLE_INTEL(name) PYBIND23_PRAGMA(warning disable name)
#else
#    define PYBIND23_WARNING_DISABLE_INTEL(name)
#endif

#define PYBIND23_NAMESPACE_BEGIN(name)                                                            \
    namespace name {                                                                              \
    PYBIND23_WARNING_PUSH

#define PYBIND23_NAMESPACE_END(name)                                                              \
    PYBIND23_WARNING_POP                                                                          \
    }

// Robust support for some features and loading modules compiled against different pybind versions
// requires forcing hidden visibility on pybind code, so we enforce this by setting the attribute
// on the main `pybind23` namespace.
#if !defined(PYBIND23_NAMESPACE)
#    if defined(__GNUG__) && !defined(_WIN32)
#        define PYBIND23_NAMESPACE pybind23 __attribute__((visibility("hidden")))
#    else
#        define PYBIND23_NAMESPACE pybind23
#    endif
#endif
