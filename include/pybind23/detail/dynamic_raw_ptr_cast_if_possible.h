// Copyright (c) 2021 The Pybind Development Team.
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#pragma once

#include "common.h"
#include "dynamic_caster.h"

#include <type_traits>

PYBIND23_NAMESPACE_BEGIN(PYBIND23_NAMESPACE)
PYBIND23_NAMESPACE_BEGIN(detail)

template <typename To, typename From, typename SFINAE = void>
struct dynamic_raw_ptr_cast_is_possible : std::false_type {};

template <typename To, typename From>
struct dynamic_raw_ptr_cast_is_possible<
    To,
    From,
    detail::enable_if_t<!std::is_same<To, void>::value && std::is_polymorphic<From>::value>>
    : std::true_type {};

template <typename To,
          typename From,
          detail::enable_if_t<!dynamic_raw_ptr_cast_is_possible<To, From>::value, int> = 0>
To *dynamic_raw_ptr_cast_if_possible(From * /*ptr*/) {
    return nullptr;
}

template <typename To,
          typename From,
          detail::enable_if_t<dynamic_raw_ptr_cast_is_possible<To, From>::value, int> = 0>
To *dynamic_raw_ptr_cast_if_possible(From *ptr) {
#if !defined(PYBIND23_NO_RTTI)
    return dynamic_cast<To *>(ptr);
#else
    auto src_type = get_type_info(type_index(typeid(From)));
    auto dst_type = get_type_info(type_index(typeid(To)));
    if (!src_type || !dst_type) {
        return nullptr;
    }
    return static_cast<To *>(internal_dynamic_cast(ptr, src_type, dst_type));
#endif
}

PYBIND23_NAMESPACE_END(detail)
PYBIND23_NAMESPACE_END(PYBIND23_NAMESPACE)
