cmake_minimum_required(VERSION 4.00)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(lxml_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-freflection-latest -fexpansion-statements -stdlib=libc++)
  add_link_options(-fuse-ld=lld -lc++ -lc++abi)
else()
  add_compile_options(-fmodules-ts)
endif()

configure_file(.clangd.in ${CMAKE_CURRENT_SOURCE_DIR}/.clangd @ONLY)

# add_executable(lxml_c test.c lxml.h)

add_subdirectory(littlexml)

add_executable(lxml_cpp test.cpp)
target_link_libraries(lxml_cpp PRIVATE lxml)
