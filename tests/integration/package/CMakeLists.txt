cmake_minimum_required(VERSION 3.20)
project(table_package_consumer LANGUAGES C CXX)

find_package(Table 1 REQUIRED)

add_executable(table_package_c main.c)
target_link_libraries(table_package_c PRIVATE Table::Table)

add_executable(table_package_cpp main.cpp)
target_compile_features(table_package_cpp PRIVATE cxx_std_11)
target_link_libraries(table_package_cpp PRIVATE Table::Table)
