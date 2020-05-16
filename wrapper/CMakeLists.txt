cmake_minimum_required (VERSION 3.8)
add_executable (wrapper "main.cpp")
target_include_directories (wrapper PRIVATE "include")
add_definitions(-DUNICODE)