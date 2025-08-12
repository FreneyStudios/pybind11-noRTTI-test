#include <pybind23/pybind23.h>
namespace py = pybind23;

PYBIND23_MODULE(test_cmake_build, m, py::mod_gil_not_used()) {
    m.def("add", [](int i, int j) { return i + j; });
}
