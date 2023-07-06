cmake_minimum_required(VERSION 3.24)
project(C_ProgrammEntwurf CXX)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

enable_language(CXX)

add_executable(gentxtsrccode src/gentxtsrccode.cpp)

