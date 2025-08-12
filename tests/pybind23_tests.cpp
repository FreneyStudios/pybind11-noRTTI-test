/*
    tests/pybind23_tests.cpp -- pybind example plugin

    Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#include "pybind23_tests.h"

#include "constructor_stats.h"

#include <functional>
#include <list>

/*
For testing purposes, we define a static global variable here in a function that each individual
test .cpp calls with its initialization lambda.  It's convenient here because we can just not
compile some test files to disable/ignore some of the test code.

It is NOT recommended as a way to use pybind23 in practice, however: the initialization order will
be essentially random, which is okay for our test scripts (there are no dependencies between the
individual pybind23 test .cpp files), but most likely not what you want when using pybind23
productively.

Instead, see the "How can I reduce the build time?" question in the "Frequently asked questions"
section of the documentation for good practice on splitting binding code over multiple files.
*/
std::list<std::function<void(py::module_ &)>> &initializers() {
    static std::list<std::function<void(py::module_ &)>> inits;
    return inits;
}

test_initializer::test_initializer(Initializer init) { initializers().emplace_back(init); }

test_initializer::test_initializer(const char *submodule_name, Initializer init) {
    initializers().emplace_back([=](py::module_ &parent) {
        auto m = parent.def_submodule(submodule_name);
        init(m);
    });
}

void bind_ConstructorStats(py::module_ &m) {
    py::class_<ConstructorStats>(m, "ConstructorStats")
        .def("alive", &ConstructorStats::alive)
        .def("values", &ConstructorStats::values)
        .def_readwrite("default_constructions", &ConstructorStats::default_constructions)
        .def_readwrite("copy_assignments", &ConstructorStats::copy_assignments)
        .def_readwrite("move_assignments", &ConstructorStats::move_assignments)
        .def_readwrite("copy_constructions", &ConstructorStats::copy_constructions)
        .def_readwrite("move_constructions", &ConstructorStats::move_constructions)
        .def_static("get",
                    (ConstructorStats & (*) (py::object)) & ConstructorStats::get,
                    py::return_value_policy::reference_internal)

        // Not exactly ConstructorStats, but related: expose the internal pybind number of
        // registered instances to allow instance cleanup checks (invokes a GC first)
        .def_static("detail_reg_inst", []() {
            ConstructorStats::gc();
            return py::detail::num_registered_instances();
        });
}

const char *cpp_std() {
    return
#if defined(PYBIND23_CPP20)
        "C++20";
#elif defined(PYBIND23_CPP17)
        "C++17";
#elif defined(PYBIND23_CPP14)
        "C++14";
#else
        "C++11";
#endif
}

PYBIND23_MODULE(pybind23_tests, m, py::mod_gil_not_used()) {
    m.doc() = "pybind23 test module";

    // Intentionally kept minimal to not create a maintenance chore
    // ("just enough" to be conclusive).
#if defined(__VERSION__)
    m.attr("compiler_info") = __VERSION__;
#elif defined(_MSC_FULL_VER)
    m.attr("compiler_info") = "MSVC " PYBIND23_TOSTRING(_MSC_FULL_VER);
#else
    m.attr("compiler_info") = py::none();
#endif
    m.attr("cpp_std") = cpp_std();
    m.attr("PYBIND23_INTERNALS_ID") = PYBIND23_INTERNALS_ID;
    // Free threaded Python uses UINT32_MAX for immortal objects.
    m.attr("PYBIND23_SIMPLE_GIL_MANAGEMENT") =
#if defined(PYBIND23_SIMPLE_GIL_MANAGEMENT)
        true;
#else
        false;
#endif

    bind_ConstructorStats(m);

#if defined(PYBIND23_DETAILED_ERROR_MESSAGES)
    m.attr("detailed_error_messages_enabled") = true;
#else
    m.attr("detailed_error_messages_enabled") = false;
#endif

    py::class_<UserType>(m, "UserType", "A `py::class_` type for testing")
        .def(py::init<>())
        .def(py::init<int>())
        .def("get_value", &UserType::value, "Get value using a method")
        .def("set_value", &UserType::set, "Set value using a method")
        .def_property("value", &UserType::value, &UserType::set, "Get/set value using a property")
        .def("__repr__", [](const UserType &u) { return "UserType({})"_s.format(u.value()); });

    py::class_<IncType, UserType>(m, "IncType")
        .def(py::init<>())
        .def(py::init<int>())
        .def("__repr__", [](const IncType &u) { return "IncType({})"_s.format(u.value()); });

    for (const auto &initializer : initializers()) {
        initializer(m);
    }

    py::class_<TestContext>(m, "TestContext")
        .def(py::init<>(&TestContext::createNewContextForInit))
        .def("__enter__", &TestContext::contextEnter)
        .def("__exit__", &TestContext::contextExit);
}
