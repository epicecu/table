cmake_minimum_required(VERSION 3.20)
project(table VERSION 1.0.0 LANGUAGES C CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(TABLE_BUILD_TESTS "Build native validation tests" OFF)
option(TABLE_BUILD_EXAMPLES "Build native examples" OFF)
option(TABLE_BUILD_NANOPB "Build the optional Nanopb adapter" OFF)
option(TABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(TABLE_ENABLE_SANITIZERS "Enable address and undefined-behaviour sanitizers" OFF)
option(TABLE_ENABLE_COVERAGE "Enable GCC coverage instrumentation" OFF)
set(TABLE_NANOPB_ROOT "" CACHE PATH "Consumer-supplied Nanopb runtime root")

if(TABLE_ENABLE_COVERAGE)
  if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "TABLE_ENABLE_COVERAGE requires GCC and G++")
  endif()
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

if(TABLE_ENABLE_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

add_library(table src/table/table.c)
add_library(Table::Table ALIAS table)
set_target_properties(table PROPERTIES EXPORT_NAME Table)
target_compile_features(table PUBLIC c_std_11)
target_include_directories(table
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(table PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
  )
  if(TABLE_WARNINGS_AS_ERRORS)
    target_compile_options(table PRIVATE -Werror)
  endif()
endif()

if(TABLE_BUILD_NANOPB)
  if(NOT TARGET Corelib::Nanopb AND NOT TARGET nanopb::nanopb AND TABLE_NANOPB_ROOT)
    foreach(nanopb_source pb_common.c pb_decode.c pb_encode.c)
      if(NOT EXISTS "${TABLE_NANOPB_ROOT}/${nanopb_source}")
        message(FATAL_ERROR "TABLE_NANOPB_ROOT does not contain ${nanopb_source}")
      endif()
    endforeach()
    add_library(table_supplied_nanopb STATIC
      ${TABLE_NANOPB_ROOT}/pb_common.c
      ${TABLE_NANOPB_ROOT}/pb_decode.c
      ${TABLE_NANOPB_ROOT}/pb_encode.c
    )
    add_library(nanopb::nanopb ALIAS table_supplied_nanopb)
    target_include_directories(table_supplied_nanopb PUBLIC
      ${TABLE_NANOPB_ROOT}
    )
  endif()
  if(NOT TARGET Corelib::Nanopb AND NOT TARGET nanopb::nanopb)
    message(FATAL_ERROR "TABLE_BUILD_NANOPB requires Corelib::Nanopb or nanopb::nanopb")
  endif()
  add_library(table_nanopb
    extras/nanopb/table/nanopb.c
    extras/nanopb/table/v1/table.pb.c
  )
  add_library(Table::Nanopb ALIAS table_nanopb)
  set_target_properties(table_nanopb PROPERTIES EXPORT_NAME Nanopb)
  target_compile_features(table_nanopb PUBLIC c_std_11)
  target_include_directories(table_nanopb PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extras/nanopb>
  )
  target_link_libraries(table_nanopb PUBLIC table)
  if(TARGET Corelib::Nanopb)
    target_link_libraries(table_nanopb PUBLIC Corelib::Nanopb)
  else()
    target_link_libraries(table_nanopb PUBLIC nanopb::nanopb)
  endif()
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(table_nanopb PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
    if(TABLE_WARNINGS_AS_ERRORS)
      target_compile_options(table_nanopb PRIVATE -Werror)
    endif()
  endif()
endif()

if(TABLE_BUILD_TESTS)
  enable_testing()
  include(FetchContent)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
    GIT_SHALLOW FALSE
  )
  FetchContent_MakeAvailable(googletest)
  include(GoogleTest)

  function(table_add_test target source)
    add_executable(${target} ${source})
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_link_libraries(${target} PRIVATE table GTest::gtest_main)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      target_compile_options(${target} PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
      )
      if(TABLE_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE -Werror)
      endif()
    endif()
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST)
  endfunction()

  table_add_test(table_c_unit_tests tests/unit/table_c_test.cpp)
  table_add_test(table_cpp_unit_tests tests/unit/table_cpp_test.cpp)
  if(TABLE_BUILD_NANOPB)
    table_add_test(table_nanopb_unit_tests tests/unit/table_nanopb_test.cpp)
    target_link_libraries(table_nanopb_unit_tests PRIVATE table_nanopb)
  endif()

  add_executable(table_c_runtime tests/integration/c_runtime.c)
  target_link_libraries(table_c_runtime PRIVATE table)
  add_test(NAME table_c_runtime COMMAND table_c_runtime)

  add_executable(table_cpp_runtime tests/integration/cpp_runtime.cpp)
  target_compile_features(table_cpp_runtime PRIVATE cxx_std_11)
  target_link_libraries(table_cpp_runtime PRIVATE table)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(table_cpp_runtime PRIVATE -fno-exceptions -fno-rtti)
  endif()
  add_test(NAME table_cpp_runtime COMMAND table_cpp_runtime)
endif()

if(TABLE_BUILD_EXAMPLES)
  add_executable(table_c_example examples/native_c/main.c)
  target_link_libraries(table_c_example PRIVATE table)
  add_executable(table_cpp_example examples/native_cpp/main.cpp)
  target_compile_features(table_cpp_example PRIVATE cxx_std_11)
  target_link_libraries(table_cpp_example PRIVATE table)
endif()

install(TARGETS table EXPORT TableTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY src/table DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
install(FILES src/Table.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY proto/ DESTINATION ${CMAKE_INSTALL_DATADIR}/table/proto)
install(EXPORT TableTargets
  FILE TableTargets.cmake
  NAMESPACE Table::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Table
)
configure_package_config_file(cmake/TableConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/TableConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Table
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/TableConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/TableConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/TableConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Table
)
