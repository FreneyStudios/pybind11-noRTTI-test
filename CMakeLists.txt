# CMakeLists.txt -- Build system for the pybind11 modules
#
# Copyright (c) 2015 Wenzel Jakob <wenzel@inf.ethz.ch>
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.

# Propagate this policy (FindPythonInterp removal) so it can be detected later
if(NOT CMAKE_VERSION VERSION_LESS "3.27")
  cmake_policy(GET CMP0148 _pybind23_cmp0148)
endif()

cmake_minimum_required(VERSION 3.15...4.0)

if(_pybind23_cmp0148)
  cmake_policy(SET CMP0148 ${_pybind23_cmp0148})
  unset(_pybind23_cmp0148)
endif()

# Avoid infinite recursion if tests include this as a subdirectory
include_guard(GLOBAL)

# Extract project version from source
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/pybind23/detail/common.h"
     pybind23_version_defines REGEX "#define PYBIND23_VERSION_(MAJOR|MINOR|PATCH) ")

foreach(ver ${pybind23_version_defines})
  if(ver MATCHES [[#define PYBIND23_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$]])
    set(PYBIND23_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
  endif()
endforeach()

if(PYBIND23_VERSION_PATCH MATCHES [[\.([a-zA-Z0-9]+)$]])
  set(pybind23_VERSION_TYPE "${CMAKE_MATCH_1}")
endif()
string(REGEX MATCH "^[0-9]+" PYBIND23_VERSION_PATCH "${PYBIND23_VERSION_PATCH}")

project(
  pybind23
  LANGUAGES CXX
  VERSION "${PYBIND23_VERSION_MAJOR}.${PYBIND23_VERSION_MINOR}.${PYBIND23_VERSION_PATCH}")

# Standard includes
set(CMAKE_CXX_STANDARD 23)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)

if(NOT pybind23_FIND_QUIETLY)
  message(STATUS "pybind23 v${PYBIND23_VERSION} ${PYBIND23_VERSION_TYPE}")
endif()

# Check if pybind23 is being used directly or via add_subdirectory
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  ### Warn if not an out-of-source builds
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    set(lines
        "You are building in-place. If that is not what you intended to "
        "do, you can clean the source directory with:\n"
        "rm -r CMakeCache.txt CMakeFiles/ cmake_uninstall.cmake pybind23Config.cmake "
        "pybind23ConfigVersion.cmake tests/CMakeFiles/\n")
    message(AUTHOR_WARNING ${lines})
  endif()

  set(PYBIND23_MASTER_PROJECT ON)

  message(STATUS "CMake ${CMAKE_VERSION}")

  if(DEFINED SKBUILD AND DEFINED ENV{PYBIND23_GLOBAL_SDIST})
    message(
      FATAL_ERROR
        "PYBIND23_GLOBAL_SDIST is not supported, use nox -s build_global or a pybind23-global SDist instead."
    )
  endif()

  if(CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
  endif()

  set(pybind23_system "")

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  if(CMAKE_VERSION VERSION_LESS "3.18")
    set(_pybind23_findpython_default OFF)
  else()
    set(_pybind23_findpython_default ON)
  endif()
else()
  set(PYBIND23_MASTER_PROJECT OFF)
  set(pybind23_system SYSTEM)
  set(_pybind23_findpython_default COMPAT)
endif()

# Options
option(PYBIND23_INSTALL "Install pybind23 header files?" ${PYBIND23_MASTER_PROJECT})
option(PYBIND23_TEST "Build pybind23 test suite?" ${PYBIND23_MASTER_PROJECT})
option(PYBIND23_NOPYTHON "Disable search for Python" OFF)
option(PYBIND23_DISABLE_HANDLE_TYPE_NAME_DEFAULT_IMPLEMENTATION
       "To enforce that a handle_type_name<> specialization exists" OFF)
option(PYBIND23_SIMPLE_GIL_MANAGEMENT
       "Use simpler GIL management logic that does not support disassociation" OFF)
set(PYBIND23_INTERNALS_VERSION
    ""
    CACHE STRING "Override the ABI version, may be used to enable the unstable ABI.")
option(PYBIND23_USE_CROSSCOMPILING "Respect CMAKE_CROSSCOMPILING" OFF)

if(PYBIND23_DISABLE_HANDLE_TYPE_NAME_DEFAULT_IMPLEMENTATION)
  add_compile_definitions(PYBIND23_DISABLE_HANDLE_TYPE_NAME_DEFAULT_IMPLEMENTATION)
endif()
if(PYBIND23_SIMPLE_GIL_MANAGEMENT)
  add_compile_definitions(PYBIND23_SIMPLE_GIL_MANAGEMENT)
endif()

cmake_dependent_option(
  USE_PYTHON_INCLUDE_DIR
  "Install pybind23 headers in Python include directory instead of default installation prefix"
  OFF "PYBIND23_INSTALL" OFF)

set(PYBIND23_FINDPYTHON
    ${_pybind23_findpython_default}
    CACHE STRING "Force new FindPython - NEW, OLD, COMPAT")

if(PYBIND23_MASTER_PROJECT)

  # Allow PYTHON_EXECUTABLE if in FINDPYTHON mode and building pybind23's tests
  # (makes transition easier while we support both modes).
  if(PYBIND23_FINDPYTHON
     AND DEFINED PYTHON_EXECUTABLE
     AND NOT DEFINED Python_EXECUTABLE)
    set(Python_EXECUTABLE "${PYTHON_EXECUTABLE}")
  endif()

  # This is a shortcut that is primarily for the venv cmake preset,
  # but can be used to quickly setup tests manually, too
  set(PYBIND23_CREATE_WITH_UV
      ""
      CACHE STRING "Create a virtualenv if it doesn't exist")

  if(NOT PYBIND23_CREATE_WITH_UV STREQUAL "")
    set(Python_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/.venv")
    if(EXISTS "${Python_ROOT_DIR}")
      if(EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
        message(STATUS "Using existing venv at ${Python_ROOT_DIR}, remove or --fresh to recreate")
      else()
        # --fresh used to remove the cache
        file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/.venv")
      endif()
    endif()
    if(NOT EXISTS "${Python_ROOT_DIR}")
      find_program(UV uv REQUIRED)
      # CMake 3.19+ would be able to use COMMAND_ERROR_IS_FATAL
      message(
        STATUS "Creating venv with ${UV} venv -p ${PYBIND23_CREATE_WITH_UV} '${Python_ROOT_DIR}'")
      execute_process(COMMAND ${UV} venv -p ${PYBIND23_CREATE_WITH_UV} "${Python_ROOT_DIR}"
                      RESULT_VARIABLE _venv_result)
      if(_venv_result AND NOT _venv_result EQUAL 0)
        message(FATAL_ERROR "uv venv failed with '${_venv_result}'")
      endif()
      message(
        STATUS
          "Installing deps with ${UV} pip install -p '${Python_ROOT_DIR}' -r tests/requirements.txt"
      )
      execute_process(
        COMMAND ${UV} pip install -p "${Python_ROOT_DIR}" -r
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/requirements.txt" RESULT_VARIABLE _pip_result)
      if(_pip_result AND NOT _pip_result EQUAL 0)
        message(FATAL_ERROR "uv pip install failed with '${_pip_result}'")
      endif()
    endif()
  else()
    if(NOT DEFINED Python3_EXECUTABLE
       AND NOT DEFINED Python_EXECUTABLE
       AND NOT DEFINED Python_ROOT_DIR
       AND NOT DEFINED ENV{VIRTUALENV}
       AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.venv")
      message(STATUS "Autodetecting Python in virtual environment")
      set(Python_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.venv")
    endif()
  endif()
endif()

set(PYBIND23_HEADERS
    include/pybind23/attr.h
    include/pybind23/buffer_info.h
    include/pybind23/cast.h
    include/pybind23/chrono.h
    include/pybind23/common.h
    include/pybind23/complex.h
    include/pybind23/conduit/pybind23_conduit_v1.h
    include/pybind23/conduit/pybind23_platform_abi_id.h
    include/pybind23/conduit/wrap_include_python_h.h
    include/pybind23/critical_section.h
    include/pybind23/detail/class.h
    include/pybind23/detail/common.h
    include/pybind23/detail/cpp_conduit.h
    include/pybind23/detail/descr.h
    include/pybind23/detail/dynamic_caster.h
    include/pybind23/detail/dynamic_raw_ptr_cast_if_possible.h
    include/pybind23/detail/exception_translation.h
    include/pybind23/detail/function_record_pyobject.h
    include/pybind23/detail/init.h
    include/pybind23/detail/internals.h
    include/pybind23/detail/native_enum_data.h
    include/pybind23/detail/pybind23_namespace_macros.h
    include/pybind23/detail/struct_smart_holder.h
    include/pybind23/detail/type_caster_base.h
    include/pybind23/detail/type_id_no_rtti.h
    include/pybind23/detail/typeid.h
    include/pybind23/detail/using_smart_holder.h
    include/pybind23/detail/value_and_holder.h
    include/pybind23/eigen.h
    include/pybind23/eigen/common.h
    include/pybind23/eigen/matrix.h
    include/pybind23/eigen/tensor.h
    include/pybind23/embed.h
    include/pybind23/eval.h
    include/pybind23/functional.h
    include/pybind23/gil.h
    include/pybind23/gil_safe_call_once.h
    include/pybind23/gil_simple.h
    include/pybind23/iostream.h
    include/pybind23/native_enum.h
    include/pybind23/numpy.h
    include/pybind23/operators.h
    include/pybind23/options.h
    include/pybind23/pybind23.h
    include/pybind23/pytypes.h
    include/pybind23/stl.h
    include/pybind23/stl/filesystem.h
    include/pybind23/stl_bind.h
    include/pybind23/subinterpreter.h
    include/pybind23/trampoline_self_life_support.h
    include/pybind23/type_caster_pyobject_ptr.h
    include/pybind23/typing.h
    include/pybind23/warnings.h)

# Compare with grep and warn if mismatched
if(PYBIND23_MASTER_PROJECT)
  file(
    GLOB_RECURSE _pybind23_header_check
    LIST_DIRECTORIES false
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    CONFIGURE_DEPENDS "include/pybind23/*.h")
  set(_pybind23_here_only ${PYBIND23_HEADERS})
  set(_pybind23_disk_only ${_pybind23_header_check})
  list(REMOVE_ITEM _pybind23_here_only ${_pybind23_header_check})
  list(REMOVE_ITEM _pybind23_disk_only ${PYBIND23_HEADERS})
  if(_pybind23_here_only)
    message(AUTHOR_WARNING "PYBIND23_HEADERS has extra files:" ${_pybind23_here_only})
  endif()
  if(_pybind23_disk_only)
    message(AUTHOR_WARNING "PYBIND23_HEADERS is missing files:" ${_pybind23_disk_only})
  endif()
endif()

list(TRANSFORM PYBIND23_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# Cache variable so this can be used in parent projects
set(pybind23_INCLUDE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/include"
    CACHE INTERNAL "Directory where pybind23 headers are located")

# Backward compatible variable for add_subdirectory mode
if(NOT PYBIND23_MASTER_PROJECT)
  set(PYBIND23_INCLUDE_DIR
      "${pybind23_INCLUDE_DIR}"
      CACHE INTERNAL "")
endif()

# Note: when creating targets, you cannot use if statements at configure time -
# you need generator expressions, because those will be placed in the target file.
# You can also place ifs *in* the Config.in, but not here.

# This section builds targets, but does *not* touch Python
# Non-IMPORT targets cannot be defined twice
if(NOT TARGET pybind23_headers)
  # Build the headers-only target (no Python included):
  # (long name used here to keep this from clashing in subdirectory mode)
  add_library(pybind23_headers INTERFACE)
  add_library(pybind23::pybind23_headers ALIAS pybind23_headers) # to match exported target
  add_library(pybind23::headers ALIAS pybind23_headers) # easier to use/remember

  target_include_directories(
    pybind23_headers ${pybind23_system} INTERFACE $<BUILD_INTERFACE:${pybind23_INCLUDE_DIR}>
                                                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  target_compile_features(pybind23_headers INTERFACE cxx_inheriting_constructors cxx_user_literals
                                                     cxx_right_angle_brackets)
  target_compile_options(pybind23_headers INTERFACE -fno-rtti)
  target_compile_definitions(pybind23_headers INTERFACE PYBIND23_NO_RTTI)
  if(NOT "${PYBIND23_INTERNALS_VERSION}" STREQUAL "")
    target_compile_definitions(
      pybind23_headers INTERFACE "PYBIND23_INTERNALS_VERSION=${PYBIND23_INTERNALS_VERSION}")
  endif()
else()
  # It is invalid to install a target twice, too.
  set(PYBIND23_INSTALL OFF)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/tools/pybind23Common.cmake")
# https://github.com/jtojnar/cmake-snips/#concatenating-paths-when-building-pkg-config-files
# TODO: cmake 3.20 adds the cmake_path() function, which obsoletes this snippet
include("${CMAKE_CURRENT_SOURCE_DIR}/tools/JoinPaths.cmake")

# Relative directory setting
if(USE_PYTHON_INCLUDE_DIR AND DEFINED Python_INCLUDE_DIRS)
  file(RELATIVE_PATH CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX} ${Python_INCLUDE_DIRS})
elseif(USE_PYTHON_INCLUDE_DIR AND DEFINED PYTHON_INCLUDE_DIR)
  file(RELATIVE_PATH CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX} ${PYTHON_INCLUDE_DIRS})
endif()

if(PYBIND23_INSTALL)
  if(DEFINED SKBUILD_PROJECT_NAME AND SKBUILD_PROJECT_NAME STREQUAL "pybind23_global")
    install(DIRECTORY ${pybind23_INCLUDE_DIR}/pybind23 DESTINATION "${SKBUILD_HEADERS_DIR}")
  endif()
  install(DIRECTORY ${pybind23_INCLUDE_DIR}/pybind23 DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  set(PYBIND23_CMAKECONFIG_INSTALL_DIR
      "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME}"
      CACHE STRING "install path for pybind23Config.cmake")

  if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(pybind23_INCLUDEDIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
  else()
    set(pybind23_INCLUDEDIR "\$\{PACKAGE_PREFIX_DIR\}/${CMAKE_INSTALL_INCLUDEDIR}")
  endif()

  configure_package_config_file(
    tools/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${PYBIND23_CMAKECONFIG_INSTALL_DIR})

  # CMake natively supports header-only libraries
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion ARCH_INDEPENDENT)

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
          tools/FindPythonLibsNew.cmake
          tools/pybind23Common.cmake
          tools/pybind23Tools.cmake
          tools/pybind23NewTools.cmake
          tools/pybind23GuessPythonExtSuffix.cmake
    DESTINATION ${PYBIND23_CMAKECONFIG_INSTALL_DIR})

  if(NOT PYBIND23_EXPORT_NAME)
    set(PYBIND23_EXPORT_NAME "${PROJECT_NAME}Targets")
  endif()

  install(TARGETS pybind23_headers EXPORT "${PYBIND23_EXPORT_NAME}")

  install(
    EXPORT "${PYBIND23_EXPORT_NAME}"
    NAMESPACE "pybind23::"
    DESTINATION ${PYBIND23_CMAKECONFIG_INSTALL_DIR})

  # pkg-config support
  if(NOT prefix_for_pc_file)
    if(IS_ABSOLUTE "${CMAKE_INSTALL_DATAROOTDIR}")
      set(prefix_for_pc_file "${CMAKE_INSTALL_PREFIX}")
    else()
      set(pc_datarootdir "${CMAKE_INSTALL_DATAROOTDIR}")
      if(CMAKE_VERSION VERSION_LESS 3.20)
        set(prefix_for_pc_file "\${pcfiledir}/..")
        while(pc_datarootdir)
          get_filename_component(pc_datarootdir "${pc_datarootdir}" DIRECTORY)
          string(APPEND prefix_for_pc_file "/..")
        endwhile()
      else()
        cmake_path(RELATIVE_PATH CMAKE_INSTALL_PREFIX BASE_DIRECTORY CMAKE_INSTALL_DATAROOTDIR
                   OUTPUT_VARIABLE prefix_for_pc_file)
      endif()
    endif()
  endif()
  join_paths(includedir_for_pc_file "\${prefix}" "${CMAKE_INSTALL_INCLUDEDIR}")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/pybind23.pc.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/pybind23.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pybind23.pc"
          DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig/")

  # When building a wheel, include __init__.py's for modules
  # (see https://github.com/pybind/pybind23/pull/5552)
  if(DEFINED SKBUILD_PROJECT_NAME AND SKBUILD_PROJECT_NAME STREQUAL "pybind23")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/empty")
    file(TOUCH "${CMAKE_CURRENT_BINARY_DIR}/empty/__init__.py")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/empty/__init__.py"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/empty/__init__.py"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig/")
  endif()

  # Uninstall target
  if(PYBIND23_MASTER_PROJECT)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P
                                        ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
  endif()
endif()

# BUILD_TESTING takes priority, but only if this is the master project
#if(PYBIND23_MASTER_PROJECT AND DEFINED BUILD_TESTING)
#  if(BUILD_TESTING)
#    if(_pybind23_nopython)
#      message(FATAL_ERROR "Cannot activate tests in NOPYTHON mode")
#    else()
#      add_subdirectory(tests)
#    endif()
#  endif()
#else()
#  if(PYBIND23_TEST)
#    if(_pybind23_nopython)
#      message(FATAL_ERROR "Cannot activate tests in NOPYTHON mode")
#    else()
#      add_subdirectory(tests)
#    endif()
#  endif()
#endif()

# Better symmetry with find_package(pybind23 CONFIG) mode.
if(NOT PYBIND23_MASTER_PROJECT)
  set(pybind23_FOUND
      TRUE
      CACHE INTERNAL "True if pybind23 and all required components found on the system")
endif()
