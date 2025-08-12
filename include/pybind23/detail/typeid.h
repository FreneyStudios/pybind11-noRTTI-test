/*
    pybind23/detail/typeid.h: Compiler-independent access to type identifiers

    Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#include "common.h"
#include "type_id_no_rtti.h"

#if !defined(PYBIND23_NO_RTTI) && !defined(__GNUG__) && !defined(_MSC_VER)
#    define PYBIND23_NO_RTTI
#endif

#if !defined(PYBIND23_NO_RTTI)
#    include <cxxabi.h>
#endif

PYBIND23_NAMESPACE_BEGIN(PYBIND23_NAMESPACE)
PYBIND23_NAMESPACE_BEGIN(detail)

/// Erase all occurrences of a substring
inline void erase_all(std::string &string, const std::string &search) {
    for (size_t pos = 0;;) {
        pos = string.find(search, pos);
        if (pos == std::string::npos) {
            break;
        }
        string.erase(pos, search.length());
    }
}

#if !defined(PYBIND23_NO_RTTI)
PYBIND23_NOINLINE void clean_type_id(std::string &name) {
    int status = 0;
    std::unique_ptr<char, void (*)(void *)> res{
        abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status), std::free};
    if (status == 0) {
        name = res.get();
    }
    detail::erase_all(name, "pybind23::");
}
#else
PYBIND23_NOINLINE void clean_type_id(std::string &) {}
#endif

inline std::string clean_type_id(const char *typeid_name) {
    std::string name(typeid_name);
    detail::clean_type_id(name);
    return name;
}

PYBIND23_NAMESPACE_END(detail)

#if !defined(PYBIND23_NO_RTTI)
/// Return a string representation of a C++ type
template <typename T>
static std::string type_id() {
    return detail::clean_type_id(typeid(T).name());
}
using type_index = std::type_index;
#else
/// Return a string representation of a C++ type
template <typename T>
static std::string type_id() {
    return "unknown";
}
using type_index = no_rtti_type_index;
#endif

PYBIND23_NAMESPACE_END(PYBIND23_NAMESPACE)
