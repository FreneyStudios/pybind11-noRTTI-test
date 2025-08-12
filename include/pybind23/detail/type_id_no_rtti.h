/*
    pybind23/detail/type_id_no_rtti.h: Alternative to typeid for when RTTI is disabled

    Copyright (c) 2023 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#include "common.h"

PYBIND23_NAMESPACE_BEGIN(PYBIND23_NAMESPACE)
PYBIND23_NAMESPACE_BEGIN(detail)

// When RTTI is disabled, we need a way to get a unique identifier for each
// type. We can do this by using the address of a static variable in a
// template.
template <typename T>
void* get_type_id() {
    static int id;
    return &id;
}

// A wrapper for the type id that can be used as a key in a map.
class type_id_wrapper {
public:
    type_id_wrapper(void* id) : id_(id) {}

    bool operator==(const type_id_wrapper& other) const {
        return id_ == other.id_;
    }

    bool operator<(const type_id_wrapper& other) const {
        return id_ < other.id_;
    }

private:
    void* id_;
};

PYBIND23_NAMESPACE_END(detail)

// A std::type_index replacement for when RTTI is disabled
class no_rtti_type_index {
public:
    no_rtti_type_index(void* id) : id_(id) {}

    bool operator==(const no_rtti_type_index& other) const {
        return id_ == other.id_;
    }

    bool operator<(const no_rtti_type_index& other) const {
        return id_ < other.id_;
    }

    size_t hash() const {
        return std::hash<void*>()(id_);
    }

    const char* name() const {
        return "unknown";
    }

private:
    void* id_;
};

PYBIND23_NAMESPACE_END(PYBIND23_NAMESPACE)

namespace std {
    template <>
    struct hash<PYBIND23_NAMESPACE::no_rtti_type_index> {
        size_t operator()(const PYBIND23_NAMESPACE::no_rtti_type_index& k) const {
            return k.hash();
        }
    };
}
