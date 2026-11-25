# Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
# SPDX-License-Identifier: MIT OR GPL-2.0-only
#
# POSANT's simulator-agnostic core on its own: a static library and its tests,
# with no ns-3 tree. It builds on swarm-core's dual-licensed primitives
# (swarm-routing/swarm-core core/): the ports, RouteDecision, the pheromone
# table, the dedup history, roulette selection and the wire helpers. Today it
# is the skeleton only (the package version and one test): no protocol code
# lands before docs/fidelity.md is reviewed (AGENTS.md rule 11; #37, #38).
# The ns-3 module (ns3/, later) compiles the same sources against the
# swarm-core contrib module.
#
#   cmake -S core -B build/core && cmake --build build/core && ctest --test-dir build/core
#
# swarm-core comes from its vcpkg port (swarm-routing/.github#489): with the
# vcpkg toolchain, vcpkg installs the version ../vcpkg.json pins from
# swarm-routing/vcpkg-registry at ../vcpkg-configuration.json's baseline (manifest
# mode), and find_package(swarm-core CONFIG REQUIRED) finds it:
#
#   cmake -S core -B build/core -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
#
# Any installed swarm-core package works too (CMAKE_PREFIX_PATH).
# SWARM_CORE_DIR: a local swarm-core core/ directory instead (development
# against an unreleased swarm-core).
# POSANT_CORE_SANITIZE=ON adds ASan + UBSan, as the sanitized CI leg.
# POSANT_CORE_TESTS builds the tests (ON at the top level).
# POSANT_CORE_SWARM_CORE_FROM_PACKAGE=ON takes swarm-core from find_package even
# when SWARM_CORE_DIR is set; POSANT_CORE_INSTALL=ON (which needs the package)
# adds the install rules and the CMake package posant-core:
# find_package(posant-core CONFIG REQUIRED) gives the imported target posant::core
# (swarm-routing/.github#436, the vcpkg port).
# POSANT_CORE_VERSION is the package version (default: .cz.toml's, else 0.0.0).
cmake_minimum_required(VERSION 3.20)
# The manifest is at the repository root, not in core/: tell the vcpkg
# toolchain (read by project() below) where it is, unless the caller did.
if(NOT DEFINED VCPKG_MANIFEST_DIR AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../vcpkg.json"
   AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
endif()
# The package version: .cz.toml's (Commitizen bumps it at each release), or
# POSANT_CORE_VERSION when core/ is built without it (a release tarball, a port).
set(POSANT_CORE_VERSION "" CACHE STRING "posant-core's package version; empty: .cz.toml's")
if(POSANT_CORE_VERSION)
  set(_posant_core_version "${POSANT_CORE_VERSION}")
elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../.cz.toml")
  file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../.cz.toml" _posant_core_version REGEX "^version = " LIMIT_COUNT 1)
  # a release bump of .cz.toml re-runs the configure step (the package version)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../.cz.toml")
  string(REGEX REPLACE "^version = \"([0-9.]+)\".*" "\\1" _posant_core_version "${_posant_core_version}")
else()
  set(_posant_core_version 0.0.0)
endif()
project(posant_core VERSION ${_posant_core_version} LANGUAGES CXX)

# The C++ standard of the ns-3 release the org supports: ns-3's own
# CMAKE_CXX_STANDARD_MINIMUM (C++23 from ns-3.46), the standard swarm-core's
# core/ targets too (swarm-routing/ns3-swarm-core#141). Set here, not left to
# swarm::core: a swarm-core commit or release before ns3-swarm-core#142
# (v0.3.0 included) passes on C++20.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SWARM_CORE_DIR "" CACHE PATH "swarm-core's core/ directory; empty: find_package(swarm-core CONFIG), its vcpkg port")
option(POSANT_CORE_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(_posant_core_top ON)
else()
  set(_posant_core_top OFF)
endif()
option(POSANT_CORE_TESTS "Build the tests and register them with ctest" ${_posant_core_top})
option(POSANT_CORE_SWARM_CORE_FROM_PACKAGE "Take swarm-core from find_package(swarm-core CONFIG) even when SWARM_CORE_DIR is set" OFF)
option(POSANT_CORE_INSTALL "Add the install rules and the CMake package posant-core" OFF)
if(SWARM_CORE_DIR AND NOT POSANT_CORE_SWARM_CORE_FROM_PACKAGE)
  set(_posant_core_swarm_core_from_dir ON)
else()
  set(_posant_core_swarm_core_from_dir OFF)
endif()
if(POSANT_CORE_INSTALL AND _posant_core_swarm_core_from_dir)
  message(FATAL_ERROR "POSANT_CORE_INSTALL needs swarm-core from its package, not SWARM_CORE_DIR: the package links an installed swarm-core")
endif()

# Sanitizer flags first: they are directory options, so swarm-core's sources,
# when SWARM_CORE_DIR adds them below, are instrumented too (the vcpkg port is
# a prebuilt library, linked as published).
if(POSANT_CORE_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# swarm-core: its installed CMake package (the vcpkg port, or any install on
# CMAKE_PREFIX_PATH; swarm-routing/ns3-swarm-core#137), which defines the
# imported target swarm::core; or, for development, a checkout's core/ as a
# subproject, which defines the same target and nothing else: no test target,
# no ctest entry, no -Werror on our sources (swarm-routing/ns3-swarm-core#100;
# its README "Use core/ from another CMake project").
if(_posant_core_swarm_core_from_dir)
  if(NOT EXISTS "${SWARM_CORE_DIR}/swarm-ports.h")
    message(FATAL_ERROR "SWARM_CORE_DIR=${SWARM_CORE_DIR} is not swarm-core's core/ directory")
  endif()
  add_subdirectory(${SWARM_CORE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/swarm-core)
else()
  find_package(swarm-core CONFIG REQUIRED)
endif()

# The strict warnings of the org's ns-3 CI (ns3-containers' swarm-quality.cmake),
# on POSANT's own sources only.
# A swarm-core deprecation ([[deprecated]], its docs/api-policy.md
# "Deprecation") stays a warning: it shows in every build, never fails one.
set(POSANT_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast
                    -Wcast-align -Woverloaded-virtual -Wnull-dereference -Wformat=2 -Werror
                    -Wno-error=deprecated-declarations)

add_library(posant_core STATIC
  posant-header.cc
  posant-router.cc
  posant-version.cc
  posant-zone.cc
)
add_library(posant::core ALIAS posant_core)
# the package version, for posant::core::version() and the tests
target_compile_definitions(posant_core PRIVATE POSANT_CORE_VERSION_STRING="${PROJECT_VERSION}")
# The installed package's imported target is posant::core too (EXPORT_NAME and
# the export's NAMESPACE).
set_target_properties(posant_core PROPERTIES EXPORT_NAME core)
include(GNUInstallDirs)
target_include_directories(posant_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/posant-core>)
if(POSANT_CORE_SANITIZE)
  # The objects are instrumented, so whatever links them needs the sanitizer
  # runtime: add_link_options above reaches this directory's targets only, not
  # a consumer's (or an installed package's) executable.
  target_link_options(posant_core INTERFACE -fsanitize=address,undefined)
endif()
target_link_libraries(posant_core PUBLIC swarm::core)
target_compile_options(posant_core PRIVATE ${POSANT_WARNINGS})

if(POSANT_CORE_TESTS)
  add_executable(posant_core_tests
    test/run-core-tests.cc
    test/test-header.cc
    test/test-router.cc
    test/test-zone.cc
  )
  target_link_libraries(posant_core_tests PRIVATE posant_core)
  target_compile_definitions(posant_core_tests PRIVATE POSANT_CORE_VERSION_STRING="${PROJECT_VERSION}")
  target_compile_options(posant_core_tests PRIVATE ${POSANT_WARNINGS})

  enable_testing()
  add_test(NAME posant-core COMMAND posant_core_tests)
endif()

if(POSANT_CORE_INSTALL)
  include(CMakePackageConfigHelpers)
  set(_posant_core_cmake_dir ${CMAKE_INSTALL_LIBDIR}/cmake/posant-core)
  install(TARGETS posant_core EXPORT posant-coreTargets
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES
    posant-config.h
    posant-header.h
    posant-router.h
    posant-version.h
    posant-zone.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/posant-core)
  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/posant-core)
  install(EXPORT posant-coreTargets NAMESPACE posant:: DESTINATION ${_posant_core_cmake_dir})
  configure_package_config_file(posant-coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/posant-coreConfig.cmake
    INSTALL_DESTINATION ${_posant_core_cmake_dir})
  # As swarm-core (its docs/api-policy.md): before 1.0 any minor may break,
  # so a 0.y release satisfies a request for 0.y only; from 1.0, its major.
  if(PROJECT_VERSION_MAJOR EQUAL 0)
    set(_posant_core_compat SameMinorVersion)
  else()
    set(_posant_core_compat SameMajorVersion)
  endif()
  write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/posant-coreConfigVersion.cmake
    COMPATIBILITY ${_posant_core_compat})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/posant-coreConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/posant-coreConfigVersion.cmake
          DESTINATION ${_posant_core_cmake_dir})
endif()
