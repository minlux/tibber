cmake_minimum_required(VERSION 3.10)
project(sml ASM C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -O0")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

#---------------------------------------------------------------------------------------
# Application
#---------------------------------------------------------------------------------------
add_executable(sml
   src/sml_parser.cpp
   src/sml.cpp
   src/crc.c
   src/main.cpp
)

#includes
target_include_directories(sml PRIVATE
   dep/
   src
)

