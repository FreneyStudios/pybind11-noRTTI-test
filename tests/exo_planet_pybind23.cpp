// Copyright (c) 2024 The pybind Community.

#if defined(PYBIND23_INTERNALS_VERSION)
#    undef PYBIND23_INTERNALS_VERSION
#endif
#define PYBIND23_INTERNALS_VERSION 900000001

#include "test_cpp_conduit_traveler_bindings.h"

namespace pybind23_tests {
namespace test_cpp_conduit {

PYBIND23_MODULE(exo_planet_pybind23, m) {
    wrap_traveler(m);
    m.def("wrap_very_lonely_traveler", [m]() { wrap_very_lonely_traveler(m); });
}

} // namespace test_cpp_conduit
} // namespace pybind23_tests
