#include <pybind23/pybind23.h>

namespace py = pybind23;

/* Simple test module/test class to check that the referenced internals data of external pybind23
 * modules are different across subinterpreters
 */

PYBIND23_MODULE(mod_shared_interpreter_gil, m, py::multiple_interpreters::shared_gil()) {
    m.def("internals_at",
          []() { return reinterpret_cast<uintptr_t>(&py::detail::get_internals()); });
#ifdef PYBIND23_HAS_SUBINTERPRETER_SUPPORT
    m.attr("defined_PYBIND23_HAS_SUBINTERPRETER_SUPPORT") = true;
#else
    m.attr("defined_PYBIND23_HAS_SUBINTERPRETER_SUPPORT") = false;
#endif
}
