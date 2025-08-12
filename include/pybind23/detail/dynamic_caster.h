/*
    pybind23/detail/dynamic_caster.h: Caster for dynamic casting when RTTI is disabled

    Copyright (c) 2023 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#include "common.h"

PYBIND23_NAMESPACE_BEGIN(PYBIND23_NAMESPACE)
PYBIND23_NAMESPACE_BEGIN(detail)

// This is a simplified version of the dynamic_cast functionality.
// It is not as powerful as the real dynamic_cast, but it is enough
// for pybind23's needs.
//
// It works by checking if the target type is in the list of base
// types of the source type.
inline void* internal_dynamic_cast(void* src, const type_info* src_type, const type_info* dst_type) {
    if (src_type == dst_type) {
        return src;
    }

    for (const auto& base : src_type->bases) {
        if (base.first == dst_type) {
            return base.second(src);
        }
    }

    return nullptr;
}

PYBIND23_NAMESPACE_END(detail)
PYBIND23_NAMESPACE_END(PYBIND23_NAMESPACE)
